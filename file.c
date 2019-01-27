/************************************************

  file.c -

  $Author: matz $
  $Date: 1995/01/10 10:42:36 $
  created at: Mon Nov 15 12:24:34 JST 1993

  Copyright (C) 1993-1995 Yukihiro Matsumoto

************************************************/

#include "ruby.h"
#include "io.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#else
# define MAXPATHLEN 1024
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#else
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};
#endif

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#else
char *strrchr();
#endif

char *strdup();
char *getenv();

extern VALUE cIO;
VALUE cFile;
VALUE mFileTest;
static VALUE sStat;

VALUE time_new();

VALUE
file_open(fname, mode)
    char *fname, *mode;
{
    VALUE port;
    OpenFile *fptr;

    port = obj_alloc(cFile);

    MakeOpenFile(port, fptr);
    fptr->mode = io_mode_flags(mode);

    fptr->f = fopen(fname, mode);
    if (fptr->f == NULL) {
	if (errno == EMFILE) {
	    gc();
	    fptr->f = fopen(fname, mode);
	}
	if (fptr->f == NULL) {
	    rb_sys_fail(fname);
	}
    }

    fptr->path = strdup(fname);

    return port;
}

static int
apply2files(func, args, arg)
    int (*func)();
    struct RArray *args;
    void *arg;
{
    int i;
    VALUE path;

    for (i=0; i<args->len; i++) {
	path = args->ptr[i];
	Check_Type(path, T_STRING);
	if ((*func)(RSTRING(path)->ptr, arg) < 0)
	    rb_sys_fail(RSTRING(path)->ptr);
    }

    return args->len;
}

static VALUE
file_tell(obj)
    VALUE obj;
{
    OpenFile *fptr;
    long pos;

    GetOpenFile(obj, fptr);

    pos = ftell(fptr->f);
    if (ferror(fptr->f) != 0) rb_sys_fail(Qnil);

    return int2inum(pos);
}

static VALUE
file_seek(obj, offset, ptrname)
    VALUE obj, offset, ptrname;
{
    OpenFile *fptr;
    long pos;

    GetOpenFile(obj, fptr);

    pos = fseek(fptr->f, NUM2INT(offset), NUM2INT(ptrname));
    if (pos != 0) rb_sys_fail(Qnil);
    clearerr(fptr->f);

    return obj;
}

static VALUE
file_set_pos(obj, offset)
    VALUE obj, offset;
{
    OpenFile *fptr;
    long pos;

    GetOpenFile(obj, fptr);
    pos = fseek(fptr->f, NUM2INT(offset), 0);
    if (pos != 0) rb_sys_fail(Qnil);
    clearerr(fptr->f);

    return obj;
}

static VALUE
file_rewind(obj)
    VALUE obj;
{
    OpenFile *fptr;

    GetOpenFile(obj, fptr);
    if (fseek(fptr->f, 0L, 0) != 0) rb_sys_fail(Qnil);
    clearerr(fptr->f);

    return obj;
}

static VALUE
file_eof(obj)
    VALUE obj;
{
    OpenFile *fptr;

    GetOpenFile(obj, fptr);
    if (feof(fptr->f) == 0) return FALSE;
    return TRUE;
}

static VALUE
file_path(obj)
    VALUE obj;
{
    OpenFile *fptr;

    GetOpenFile(obj, fptr);
    return str_new2(fptr->path);
}

static VALUE
file_isatty(obj)
    VALUE obj;
{
    return FALSE;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

static VALUE
stat_new(st)
    struct stat *st;
{
    if (st == Qnil) Bug("stat_new() called with nil");
    return struct_new(sStat,
		      INT2FIX((int)st->st_dev),
		      INT2FIX((int)st->st_ino),
		      INT2FIX((int)st->st_mode),
		      INT2FIX((int)st->st_nlink),
		      INT2FIX((int)st->st_uid),
		      INT2FIX((int)st->st_gid),
#ifdef HAVE_ST_RDEV
		      INT2FIX((int)st->st_rdev),
#else
		      INT2FIX(0),
#endif
		      INT2FIX((int)st->st_size),
#ifdef HAVE_ST_BLKSIZE
		      INT2FIX((int)st->st_blksize),
#else
		      INT2FIX(0),
#endif
#ifdef HAVE_ST_BLOCKS
		      INT2FIX((int)st->st_blocks),
#else
		      INT2FIX(0),
#endif
		      time_new(st->st_atime, 0),
		      time_new(st->st_mtime, 0),
		      time_new(st->st_ctime, 0),
		      Qnil);
}

static struct stat laststat;

int
cache_stat(path, st)
    char *path;
    struct stat *st;
{
    if (strcmp("&", path) == 0) {
	*st = laststat;
	return 0;
    }
    if (stat(path, st) == -1)
	return -1;

    laststat = *st;
    return 0;
}

static VALUE
file_s_stat(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) == -1) {
	rb_sys_fail(fname->ptr);
    }
    return stat_new(&st);
}

static VALUE
file_stat(obj)
    VALUE obj;
{
    OpenFile *fptr;

    GetOpenFile(obj, fptr);
    if (fstat(fileno(fptr->f), &laststat) == -1) {
	rb_sys_fail(fptr->path);
    }
    return stat_new(&laststat);
}

static VALUE
file_s_lstat(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (lstat(fname->ptr, &st) == -1) {
	rb_sys_fail(fname->ptr);
    }
    return stat_new(&st);
}

static VALUE
file_lstat(obj)
    VALUE obj;
{
    OpenFile *fptr;
    struct stat st;

    GetOpenFile(obj, fptr);
    if (lstat(fptr->path, &st) == -1) {
	rb_sys_fail(fptr->path);
    }
    return stat_new(&st);
}

static int
group_member(gid)
    GETGROUPS_T gid;
{
#ifndef NT
    if (getgid() ==  gid || getegid() == gid)
	return TRUE;

# ifdef HAVE_GETGROUPS
#  ifndef NGROUPS
#    define NGROUPS 32
#  endif
    {
	GETGROUPS_T gary[NGROUPS];
	int anum;

	anum = getgroups(NGROUPS, gary);
	while (--anum >= 0)
	    if (gary[anum] == gid)
		return TRUE;
    }
# endif
#endif
    return FALSE;
}

#ifndef S_IXUGO
#  define S_IXUGO		(S_IXUSR | S_IXGRP | S_IXOTH)
#endif

int
eaccess(path, mode)
     char *path;
     int mode;
{
  struct stat st;
  static int euid = -1;

  if (cache_stat(path, &st) < 0) return (-1);

  if (euid == -1)
    euid = geteuid ();

  if (euid == 0)
    {
      /* Root can read or write any file. */
      if (mode != X_OK)
	return 0;

      /* Root can execute any file that has any one of the execute
	 bits set. */
      if (st.st_mode & S_IXUGO)
	return 0;
    }

  if (st.st_uid == euid)        /* owner */
    mode <<= 6;
  else if (group_member (st.st_gid))
    mode <<= 3;

  if (st.st_mode & mode) return 0;

  return -1;
}

static VALUE
test_d(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifndef S_ISDIR
#   define S_ISDIR(m) ((m & S_IFMT) == S_IFDIR)
#endif

    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISDIR(st.st_mode)) return TRUE;
    return FALSE;
}

static VALUE
test_p(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifdef S_IFIFO
#  ifndef S_ISFIFO
#    define S_ISFIFO(m) ((m & S_IFMT) == S_IFIFO)
#  endif

    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISFIFO(st.st_mode)) return TRUE;

#endif
    return FALSE;
}

static VALUE
test_l(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifndef S_ISLNK
#  ifdef _S_ISLNK
#    define S_ISLNK(m) _S_ISLNK(m)
#  else
#    ifdef _S_IFLNK
#      define S_ISLNK(m) ((m & S_IFMT) == _S_IFLNK)
#    else
#      ifdef S_IFLNK
#	 define S_ISLNK(m) ((m & S_IFMT) == S_IFLNK)
#      endif
#    endif
#  endif
#endif

#ifdef S_ISLNK
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISLNK(st.st_mode)) return TRUE;

#endif
    return FALSE;
}

VALUE
test_S(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifndef S_ISSOCK
#  ifdef _S_ISSOCK
#    define S_ISSOCK(m) _S_ISSOCK(m)
#  else
#    ifdef _S_IFSOCK
#      define S_ISSOCK(m) ((m & S_IFMT) == _S_IFSOCK)
#    else
#      ifdef S_IFSOCK
#	 define S_ISSOCK(m) ((m & S_IFMT) == S_IFSOCK)
#      endif
#    endif
#  endif
#endif

#ifdef S_ISSOCK
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISSOCK(st.st_mode)) return TRUE;

#endif
    return FALSE;
}

static VALUE
test_b(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifndef S_ISBLK
#   ifdef S_IFBLK
#	define S_ISBLK(m) ((m & S_IFMT) == S_IFBLK)
#   endif
#endif

#ifdef S_ISBLK
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISBLK(st.st_mode)) return TRUE;

#endif
    return FALSE;
}

static VALUE
test_c(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifndef S_ISCHR
#   define S_ISCHR(m) ((m & S_IFMT) == S_IFCHR)
#endif

    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISBLK(st.st_mode)) return TRUE;

    return FALSE;
}

static VALUE
test_e(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_r(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
    if (eaccess(fname->ptr, R_OK) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_R(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
    if (access(fname->ptr, R_OK) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_w(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
    if (eaccess(fname->ptr, W_OK) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_W(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
    if (access(fname->ptr, W_OK) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_x(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
    if (eaccess(fname->ptr, X_OK) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_X(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
    if (access(fname->ptr, X_OK) < 0) return FALSE;
    return TRUE;
}

static VALUE
test_f(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (S_ISREG(st.st_mode)) return TRUE;
    return FALSE;
}

static VALUE
test_z(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (st.st_size == 0) return TRUE;
    return FALSE;
}

static VALUE
test_s(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (st.st_size == 0) return FALSE;
    return int2inum(st.st_size);
}

static VALUE
test_owned(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (st.st_uid == geteuid()) return TRUE;
    return FALSE;
}

static VALUE
test_rowned(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (st.st_uid == getuid()) return TRUE;
    return FALSE;
}

static VALUE
test_grpowned(obj, fname)
    VALUE obj;
    struct RString *fname;
{
#ifndef NT
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) return FALSE;
    if (st.st_gid == getegid()) return TRUE;
#else
    Check_Type(fname, T_STRING);
#endif
    return FALSE;
}

#if defined(S_ISUID) || defined(S_ISGID) || defined(S_ISVTX)
static VALUE
check3rdbyte(file, mode)
    char *file;
    int mode;
{
    struct stat st;

    if (cache_stat(file, &st) < 0) return FALSE;
    if (st.st_mode & mode) return TRUE;
    return FALSE;
}
#endif

static VALUE
test_suid(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
#ifdef S_ISUID
    return check3rdbyte(fname->ptr, S_ISUID);
#else
    return FALSE;
#endif
}

static VALUE
test_sgid(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
#ifndef NT
    return check3rdbyte(fname->ptr, S_ISGID);
#else
    return FALSE;
#endif
}

static VALUE
test_sticky(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    Check_Type(fname, T_STRING);
#ifdef S_ISVTX
    return check3rdbyte(fname->ptr, S_ISVTX);
#else
    return FALSE;
#endif
}

static VALUE
file_s_type(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;
    char *t;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) rb_sys_fail(fname->ptr);

    if (S_ISREG(st.st_mode)) {
	t = "file";
    } else if (S_ISDIR(st.st_mode)) {
	t = "directory";
    } else if (S_ISCHR(st.st_mode)) {
	t = "characterSpecial";
    }
#ifdef S_ISBLK
    else if (S_ISBLK(st.st_mode)) {
	t = "blockSpecial";
    }
#endif
#ifndef S_ISFIFO
    else if (S_ISFIFO(st.st_mode)) {
	t = "fifo";
    }
#endif
#ifdef S_ISLNK
    else if (S_ISLNK(st.st_mode)) {
	t = "link";
    }
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(st.st_mode)) {
	t = "socket";
    }
#endif
    else {
	t = "unknown";
    }

    return str_new2(t);
}

static VALUE
file_s_atime(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) rb_sys_fail(fname->ptr);
    return time_new(st.st_atime, 0);
}

static VALUE
file_atime(obj)
    VALUE obj;
{
    OpenFile *fptr;
    struct stat st;

    GetOpenFile(obj, fptr);
    if (fstat(fileno(fptr->f), &st) == -1) {
	rb_sys_fail(fptr->path);
    }
    return time_new(st.st_atime, 0);
}

static VALUE
file_s_mtime(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) rb_sys_fail(fname->ptr);
    return time_new(st.st_mtime, 0);
}

static VALUE
file_mtime(obj)
    VALUE obj;
{
    OpenFile *fptr;
    struct stat st;

    GetOpenFile(obj, fptr);
    if (fstat(fileno(fptr->f), &st) == -1) {
	rb_sys_fail(fptr->path);
    }
    return time_new(st.st_mtime, 0);
}

static VALUE
file_s_ctime(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    struct stat st;

    Check_Type(fname, T_STRING);
    if (cache_stat(fname->ptr, &st) < 0) rb_sys_fail(fname->ptr);
    return time_new(st.st_ctime, 0);
}

static VALUE
file_ctime(obj)
    VALUE obj;
{
    OpenFile *fptr;
    struct stat st;

    GetOpenFile(obj, fptr);
    if (fstat(fileno(fptr->f), &st) == -1) {
	rb_sys_fail(fptr->path);
    }
    return time_new(st.st_ctime, 0);
}

static void
chmod_internal(path, mode)
    char *path;
    int mode;
{
    if (chmod(path, mode) == -1)
	rb_sys_fail(path);
}

static VALUE
file_s_chmod(argc, argv)
    int argc;
    VALUE *argv;
{
    VALUE vmode;
    VALUE rest;
    int mode, n;

    rb_scan_args(argc, argv, "1*", &vmode, &rest);
    mode = NUM2INT(vmode);

    n = apply2files(chmod_internal, rest, mode);
    return INT2FIX(n);
}

static VALUE
file_chmod(obj, vmode)
    VALUE obj, vmode;
{
    OpenFile *fptr;
    int mode;

    mode = NUM2INT(vmode);

    GetOpenFile(obj, fptr);
    if (fchmod(fileno(fptr->f), mode) == -1)
	rb_sys_fail(fptr->path);

    return INT2FIX(0);
}

struct chown_args {
    int owner, group;
};

static void
chown_internal(path, args)
    char *path;
    struct chown_args *args;
{
    if (chown(path, args->owner, args->group) < 0)
	rb_sys_fail(path);
}

static VALUE
file_s_chown(argc, argv)
    int argc;
    VALUE *argv;
{
    VALUE o, g, rest;
    struct chown_args arg;
    int n;

    rb_scan_args(argc, argv, "2*", &o, &g, &rest);
    if (o == Qnil) {
	arg.owner = -1;
    }
    else {
	arg.owner = NUM2INT(o);
    }
    if (g == Qnil) {
	arg.group = -1;
    }
    else {
	arg.group = NUM2INT(g);
    }

    n = apply2files(chown_internal, rest, &arg);
    return INT2FIX(n);
}

VALUE
file_chown(obj, owner, group)
    VALUE obj, owner, group;
{
    OpenFile *fptr;

    GetOpenFile(obj, fptr);
    if (fchown(fileno(fptr->f), NUM2INT(owner), NUM2INT(group)) == -1)
	rb_sys_fail(fptr->path);

    return INT2FIX(0);
}

struct timeval *time_timeval();

#ifdef HAVE_UTIMES

static void
utime_internal(path, tvp)
    char *path;
    struct timeval tvp[];
{
    if (utimes(path, tvp) < 0)
	rb_sys_fail(path);
}

static VALUE
file_s_utime(argc, argv)
    int argc;
    VALUE *argv;
{
    VALUE atime, mtime, rest;
    struct timeval tvp[2];
    int n;

    rb_scan_args(argc, argv, "2*", &atime, &mtime, &rest);

    tvp[0] = *time_timeval(atime);
    tvp[1] = *time_timeval(mtime);

    n = apply2files(utime_internal, rest, tvp);
    return INT2FIX(n);
}

#else

#ifndef HAVE_UTIME_H
# ifdef NT
#  include <sys/utime.h>
# else
struct utimbuf {
    long actime;
    long modtime;
};
# endif
#endif

static void
utime_internal(path, utp)
    char *path;
    struct utimbuf *utp;
{
    if (utime(path, utp) < 0)
	rb_sys_fail(path);
}

static VALUE
file_s_utime(argc, argv)
    int argc;
    VALUE *argv;
{
    VALUE atime, mtime, rest;
    int n;
    struct timeval *tv;
    struct utimbuf utbuf;

    rb_scan_args(argc, argv, "2*", &atime, &mtime, &rest);

    tv = time_timeval(atime);
    utbuf.actime = tv->tv_sec;
    tv = time_timeval(mtime);
    utbuf.modtime = tv->tv_sec;

    n = apply2files(utime_internal, rest, &utbuf);
    return INT2FIX(n);
}

#endif

static VALUE
file_s_link(obj, from, to)
    VALUE obj;
    struct RString *from, *to;
{
    Check_Type(from, T_STRING);
    Check_Type(to, T_STRING);

    if (link(from->ptr, to->ptr) < 0)
	rb_sys_fail(from->ptr);
    return INT2FIX(0);
}

static VALUE
file_s_symlink(obj, from, to)
    VALUE obj;
    struct RString *from, *to;
{
    Check_Type(from, T_STRING);
    Check_Type(to, T_STRING);

    if (symlink(from->ptr, to->ptr) < 0)
	rb_sys_fail(from->ptr);
    return TRUE;
}

static VALUE
file_s_readlink(obj, path)
    VALUE obj;
    struct RString *path;
{
    char buf[MAXPATHLEN];
    int cc;

    Check_Type(path, T_STRING);

    if ((cc = readlink(path->ptr, buf, MAXPATHLEN)) < 0)
	rb_sys_fail(path->ptr);

    return str_new(buf, cc);
}

static void
unlink_internal(path)
    char *path;
{
    if (unlink(path) < 0)
	rb_sys_fail(path);
}

static VALUE
file_s_unlink(obj, args)
    VALUE obj;
    struct RArray *args;
{
    int n;

    n = apply2files(unlink_internal, args, Qnil);
    return INT2FIX(n);
}

static VALUE
file_s_rename(obj, from, to)
    VALUE obj;
    struct RString *from, *to;
{
    Check_Type(from, T_STRING);
    Check_Type(to, T_STRING);

    if (rename(from->ptr, to->ptr) == -1)
	rb_sys_fail(from->ptr);

    return INT2FIX(0);
}

static VALUE
file_s_umask(argc, argv)
    int argc;
    VALUE *argv;
{
    int omask = 0;

    if (argc == 0) {
	int omask = umask(0);
	umask(omask);
    }
    else if (argc == 1) {
	omask = umask(NUM2INT(argv[1]));
    }
    else {
	Fail("wrong # of argument");
    }
    return INT2FIX(omask);
}

#if defined(HAVE_TRUNCATE) || defined(HAVE_CHSIZE)
static VALUE
file_s_truncate(obj, path, len)
    VALUE obj, len;
    struct RString *path;
{
    Check_Type(path, T_STRING);

#ifdef HAVE_TRUNCATE
    if (truncate(path->ptr, NUM2INT(len)) < 0)
	rb_sys_fail(path->ptr);
#else
# ifdef HAVE_CHSIZE
    {
	int tmpfd;

#if defined(NT)
	if ((tmpfd = open(path->ptr, O_RDWR)) < 0) {
	    rb_sys_fail(path->ptr);
	}
#else
	if ((tmpfd = open(path->ptr, 0)) < 0) {
	    rb_sys_fail(path->ptr);
	}
#endif
	if (chsize(tmpfd, NUM2INT(len)) < 0) {
	    close(tmpfd);
	    rb_sys_fail(path->ptr);
	}
	close(tmpfd);
    }
# endif
#endif
    return TRUE;
}

static VALUE
file_truncate(obj, len)
    VALUE obj, len;
{
    OpenFile *fptr;

    GetOpenFile(obj, fptr);

    if (!(fptr->mode & FMODE_WRITABLE)) {
	Fail("not opened for writing");
    }
#ifdef HAVE_TRUNCATE
    if (ftruncate(fileno(fptr->f), NUM2INT(len)) < 0)
	rb_sys_fail(fptr->path);
#else
# ifdef HAVE_CHSIZE
    if (chsize(fileno(fptr->f), NUM2INT(len)) < 0)
	rb_sys_fail(fptr->path);
# endif
#endif
    return TRUE;
}
#endif

#ifdef HAVE_FCNTL
static VALUE
file_fcntl(obj, req, arg)
    VALUE obj, req;
    struct RString *arg;
{
    io_ctl(obj, req, arg, 0);
    return obj;
}
#endif

static VALUE
file_s_expand_path(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    char *s, *p;
    char buf[MAXPATHLEN];

    Check_Type(fname, T_STRING);
    s = fname->ptr;

    p = buf;
    if (s[0] == '~') {
	if (s[1] == '/' || s[1] == '\0') {
	    char *dir = getenv("HOME");

	    if (!dir) {
		Fail("couldn't find HOME environment -- expanding `%s'", s);
	    }
	    strcpy(buf, dir);
	    p = &buf[strlen(buf)];
	    s++;
	}
	else {
#ifdef HAVE_PWD_H
	    struct passwd *pwPtr;
	    s++;
#endif

	    while (*s && *s != '/') {
		*p++ = *s++;
	    }
	    *p = '\0';
#ifdef HAVE_PWD_H
	    pwPtr = getpwnam(buf);
	    if (!pwPtr) {
		endpwent();
		Fail("user %s doesn't exist", buf);
	    }
	    strcpy(buf, pwPtr->pw_dir);
	    p = &buf[strlen(buf)];
	    endpwent();
#endif
	}
    }
    else if (s[0] != '/') {
#ifdef HAVE_GETCWD
	getcwd(buf, MAXPATHLEN);
#else
	getwd(buf)l
#endif
	p = &buf[strlen(buf)];
    }
    *p = '/';

    for ( ; *s; s++) {
	switch (*s) {
	  case '.':
	    if (*(s+1)) {
		switch (*++s) {
		  case '.':
		    if (*(s+1) == '\0' || *(s+1) == '/') { 
			/* We must go back to the parent */
			if (*p == '/' && p > buf) p--;
			while (p > buf && *p != '/') p--;
		    }
		    else {
			*++p = '.';
			*++p = '.';
		    }
		    break;
		  case '/':
		    if (*p != '/') *++p = '/'; 
		    break;
		  default:
		    *++p = '.'; *++p = *s; break;
		}
	    }
	    break;
	  case '/':
	    if (*p != '/') *++p = '/'; break;
	  default:
	    *++p = *s;
	}
    }
  
    /* Place a \0 at end. If path ends with a "/", delete it */
    if (p == buf || *p != '/') p++;
    *p = '\0';

    return str_new2(buf);
}

static int
rmext(p, e)
    char *p, *e;
{
    int l1, l2;

    l1 = strlen(p);
    if (!e) return 0;

    l2 = strlen(e);
    if (l1 < l2) return l1;

    if (strcmp(p+l1-l2, e) == 0) {
	return l1-l2;
    }
    return 0;
}

static VALUE
file_s_basename(argc, argv)
    int argc;
    VALUE *argv;
{
    struct RString *fname;
    struct RString *ext;
    char *p;
    int f;

    rb_scan_args(argc, argv, "11", &fname, &ext);
    Check_Type(fname, T_STRING);
    if (ext) Check_Type(ext, T_STRING);
    p = strrchr(fname->ptr, '/');
    if (p == Qnil) {
	if (ext) {
	    f = rmext(fname->ptr, ext->ptr);
	    if (f) return str_new(fname->ptr, f);
	}
	return (VALUE)fname;
    }
    p++;			/* skip last `/' */
    if (ext) {
	f = rmext(p, ext->ptr);
	if (f) return str_new(p, f);
    }
    return str_new2(p);
}

static VALUE
file_s_dirname(obj, fname)
    VALUE obj;
    struct RString *fname;
{
    char *p;
    Check_Type(fname, T_STRING);
    p = strrchr(fname->ptr, '/');
    if (p == Qnil) return (VALUE)fname;
    return str_new(fname->ptr, p - fname->ptr);
}

static void
test_check(n, argc, argv)
    int n, argc;
    VALUE *argv;
{
    int i;

    n+=1;
    if (n < argc) Fail("Wrong # of arguments(%d for %d)", argc, n);
    for (i=1; i<n; i++) {
	Check_Type(argv[i], T_STRING);
    }
}

#define CHECK(n) test_check((n), argc, argv)

static VALUE
f_test(argc, argv)
    int argc;
    VALUE *argv;
{
    int cmd;

    if (argc == 0) Fail("Wrong # of arguments");
    Need_Fixnum(argv[0]);
    cmd = FIX2INT(argv[0]);
    if (strchr("bcdefgGkloOprRsSuwWxXz", cmd)) {
	CHECK(1);
	switch (cmd) {
	  case 'b':
	    return test_b(0, argv[1]);

	  case 'c':
	    return test_c(0, argv[1]);

	  case 'd':
	    return test_d(0, argv[1]);

	  case 'a':
	  case 'e':
	    return test_e(0, argv[1]);

	  case 'f':
	    return test_f(0, argv[1]);

	  case 'g':
	    return test_sgid(0, argv[1]);

	  case 'G':
	    return test_grpowned(0, argv[1]);

	  case 'k':
	    return test_sticky(0, argv[1]);

	  case 'l':
	    return test_l(0, argv[1]);

	  case 'o':
	    return test_owned(0, argv[1]);

	  case 'O':
	    return test_rowned(0, argv[1]);

	  case 'p':
	    return test_p(0, argv[1]);

	  case 'r':
	    return test_r(0, argv[1]);

	  case 'R':
	    return test_R(0, argv[1]);

	  case 's':
	    return test_s(0, argv[1]);

	  case 'S':
	    return test_S(0, argv[1]);

	  case 'u':
	    return test_suid(0, argv[1]);

	  case 'w':
	    return test_w(0, argv[1]);

	  case 'W':
	    return test_W(0, argv[1]);

	  case 'x':
	    return test_x(0, argv[1]);

	  case 'X':
	    return test_X(0, argv[1]);

	  case 'z':
	    return test_z(0, argv[1]);
	}
    }

    if (strchr("MAC", cmd)) {
	struct stat st;

	CHECK(1);
	if (cache_stat(RSTRING(argv[1])->ptr, &st) == -1) {
	    rb_sys_fail(RSTRING(argv[1])->ptr);
	}

	switch (cmd) {
	  case 'A':
	    return time_new(st.st_atime, 0);
	  case 'M':
	    return time_new(st.st_mtime, 0);
	  case 'C':
	    return time_new(st.st_ctime, 0);
	}
    }

    if (strchr("-=<>", cmd)) {
	struct stat st1, st2;

	CHECK(2);
	if (stat(RSTRING(argv[1])->ptr, &st1) < 0) return FALSE;
	if (stat(RSTRING(argv[2])->ptr, &st2) < 0) return FALSE;

	switch (cmd) {
	  case '-':
	    if (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino)
		return TRUE;
	    break;

	  case '=':
	    if (st1.st_mtime == st2.st_mtime) return TRUE;
	    break;

	  case '>':
	    if (st1.st_mtime > st2.st_mtime) return TRUE;
	    break;

	  case '<':
	    if (st1.st_mtime < st2.st_mtime) return TRUE;
	    break;
	}
    }
    return FALSE;
}

extern VALUE cKernel;

void
Init_File()
{
    mFileTest = rb_define_module("FileTest");

    rb_define_module_function(mFileTest, "directory?",  test_d, 1);
    rb_define_module_function(mFileTest, "exists?",  test_e, 1);
    rb_define_module_function(mFileTest, "readable?",  test_r, 1);
    rb_define_module_function(mFileTest, "readable_real?",  test_R, 1);
    rb_define_module_function(mFileTest, "writable?",  test_w, 1);
    rb_define_module_function(mFileTest, "writable_real?",  test_W, 1);
    rb_define_module_function(mFileTest, "executable?",  test_x, 1);
    rb_define_module_function(mFileTest, "executable_real?",  test_X, 1);
    rb_define_module_function(mFileTest, "file?",  test_f, 1);
    rb_define_module_function(mFileTest, "zero?",  test_z, 1);
    rb_define_module_function(mFileTest, "size",  test_s, 1);
    rb_define_module_function(mFileTest, "owned?",  test_owned, 1);
    rb_define_module_function(mFileTest, "grpowned?",  test_grpowned, 1);

    rb_define_module_function(mFileTest, "pipe?",  test_p, 1);
    rb_define_module_function(mFileTest, "symlink?",  test_l, 1);
    rb_define_module_function(mFileTest, "socket?",  test_S, 1);

    rb_define_module_function(mFileTest, "blockdev?",  test_b, 1);
    rb_define_module_function(mFileTest, "chardev?",  test_c, 1);

    rb_define_module_function(mFileTest, "setuid?",  test_suid, 1);
    rb_define_module_function(mFileTest, "setgid?",  test_sgid, 1);
    rb_define_module_function(mFileTest, "sticky?",  test_sticky, 1);

    cFile = rb_define_class("File", cIO);
    rb_extend_object(cFile, CLASS_OF(mFileTest));

    rb_define_singleton_method(cFile, "stat",  file_s_stat, 1);
    rb_define_singleton_method(cFile, "lstat", file_s_lstat, 1);
    rb_define_singleton_method(cFile, "type",  file_s_type, 1);

    rb_define_singleton_method(cFile, "atime", file_s_atime, 1);
    rb_define_singleton_method(cFile, "mtime", file_s_mtime, 1);
    rb_define_singleton_method(cFile, "ctime", file_s_ctime, 1);

    rb_define_singleton_method(cFile, "utime", file_s_utime, -1);
    rb_define_singleton_method(cFile, "chmod", file_s_chmod, -1);
    rb_define_singleton_method(cFile, "chown", file_s_chown, -1);

    rb_define_singleton_method(cFile, "link", file_s_link, 2);
    rb_define_singleton_method(cFile, "symlink", file_s_symlink, 2);
    rb_define_singleton_method(cFile, "readlink", file_s_readlink, 1);

    rb_define_singleton_method(cFile, "unlink", file_s_unlink, -2);
    rb_define_singleton_method(cFile, "delete", file_s_unlink, -2);
    rb_define_singleton_method(cFile, "rename", file_s_rename, 2);
    rb_define_singleton_method(cFile, "umask", file_s_umask, -1);
#if defined(HAVE_TRUNCATE) || defined(HAVE_CHSIZE)
    rb_define_singleton_method(cFile, "truncate", file_s_truncate, 2);
#endif
    rb_define_singleton_method(cFile, "expand_path", file_s_expand_path, 1);
    rb_define_singleton_method(cFile, "basename", file_s_basename, -1);
    rb_define_singleton_method(cFile, "dirname", file_s_dirname, 1);

    rb_define_method(cFile, "stat",  file_stat, 0);
    rb_define_method(cFile, "lstat",  file_lstat, 0);

    rb_define_method(cFile, "atime", file_atime, 0);
    rb_define_method(cFile, "mtime", file_mtime, 0);
    rb_define_method(cFile, "ctime", file_ctime, 0);

    rb_define_method(cFile, "chmod", file_chmod, 1);
    rb_define_method(cFile, "chown", file_chown, 2);
#if defined(HAVE_TRUNCATE) || defined(HAVE_CHSIZE)
    rb_define_method(cFile, "truncate", file_truncate, 1);
#endif

    rb_define_method(cFile, "tell",  file_tell, 0);
    rb_define_method(cFile, "seek",  file_seek, 2);

    rb_define_method(cFile, "pos",  file_tell, 0);
    rb_define_method(cFile, "pos=", file_set_pos, 1);

    rb_define_method(cFile, "rewind", file_rewind, 0);
    rb_define_method(cFile, "isatty", file_isatty, 0);
    rb_define_method(cFile, "tty?",  file_isatty, 0);
    rb_define_method(cFile, "eof",  file_eof, 0);
    rb_define_method(cFile, "eof?", file_eof, 0);

#ifdef HAVE_FCNTL
    rb_define_method(cIO, "fcntl", file_fcntl, 2);
#endif

    rb_define_method(cFile, "path",  file_path, 0);

    rb_define_method(cKernel, "test", f_test, -1);

    sStat = struct_define("Stat", "dev", "ino", "mode",
			  "nlink", "uid", "gid", "rdev",
			  "size", "blksize", "blocks", 
			  "atime", "mtime", "ctime", Qnil);
}
