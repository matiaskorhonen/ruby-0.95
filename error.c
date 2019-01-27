/************************************************

  error.c -

  $Author: matz $
  $Date: 1995/01/10 10:42:31 $
  created at: Mon Aug  9 16:11:34 JST 1993

  Copyright (C) 1993-1995 Yukihiro Matsumoto

************************************************/

#include "ruby.h"
#include "env.h"
#include <stdio.h>
#include <varargs.h>

extern char *sourcefile;
extern int   sourceline;

int nerrs;

static void
err_sprintf(buf, fmt, args)
    char *buf, *fmt;
    va_list args;
{
    if (!sourcefile) {
	vsprintf(buf, fmt, args);
    }
    else {
	sprintf(buf, "%s:%d: ", sourcefile, sourceline);
	vsprintf((char*)buf+strlen(buf), fmt, args);
    }
    if (buf[strlen(buf)-1] != '\n')
	strcat(buf, "\n");
}

static void
err_print(fmt, args)
    char *fmt;
    va_list args;
{
    extern errstr;
    char buf[BUFSIZ];

    err_sprintf(buf, fmt, args);
    if (rb_in_eval) {
	if (errstr == Qnil) {
	    errstr = str_new2(buf);
	}
	else {
	    str_cat(errstr, buf, strlen(buf));
	}
    }
    else {
	fputs(buf, stderr);
    }
}

void
Error(fmt, va_alist)
    char *fmt;
    va_dcl
{
    va_list args;

    va_start(args);
    err_print(fmt, args);
    va_end(args);
    nerrs++;
}

int
yyerror(msg)
    char *msg;
{
    static char *f;
    static int line;

    if (line == sourceline && strcmp(f, sourcefile) == 0)
	return;
    f = sourcefile; line = sourceline;
    Error("%s", msg);
    return 0;
}

void
Warning(fmt, va_alist)
    char *fmt;
    va_dcl
{
    char buf[BUFSIZ];
    va_list args;

    if (!verbose) return;

    sprintf(buf, "warning: %s", fmt);

    va_start(args);
    err_print(buf, args);
    va_end(args);
}

void
Fatal(fmt, va_alist)
    char *fmt;
    va_dcl
{
    va_list args;

    va_start(args);
    err_print(fmt, args);
    va_end(args);
    rb_exit(1);
}

void
Bug(fmt, va_alist)
    char *fmt;
    va_dcl
{
    char buf[BUFSIZ];
    va_list args;

    sprintf(buf, "[BUG] %s", fmt);

    va_start(args);
    err_print(buf, args);
    va_end(args);
    abort();
}

void
Fail(fmt, va_alist)
    char *fmt;
    va_dcl
{
    va_list args;
    char buf[BUFSIZ];

    va_start(args);
    vsprintf(buf, fmt, args);
    va_end(args);

    rb_fail(str_new2(buf));
}

void
rb_sys_fail(mesg)
    char *mesg;
{
    char *strerror();
    char buf[BUFSIZ];
    extern int errno;

    if (mesg == Qnil)
	sprintf(buf, "%s", strerror(errno));
    else
	sprintf(buf, "%s - %s", strerror(errno), mesg);

    errno = 0;
    rb_fail(str_new2(buf));
}

static char *builtin_types[] = {
    "Nil",
    "Object",
    "Class",
    "iClass",
    "Module",
    "Float",
    "String",
    "Regexp",
    "Array",
    "Fixnum",
    "Hash",
    "Struct",
    "Bignum",
    "Data",
    "Match",
};

void
WrongType(x, t)
    VALUE x;
    int t;
{
    Fail("wrong argument type %s (expected %s)",
	 rb_class2name(CLASS_OF(x)), builtin_types[t]);
}
