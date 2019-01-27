/************************************************

  struct.c -

  $Author: matz $
  $Date: 1995/01/10 10:43:02 $
  created at: Tue Mar 22 18:44:30 JST 1995

************************************************/

#include "ruby.h"

ID rb_frame_last_func();
VALUE cStruct;
extern VALUE mEnumerable;

static VALUE
struct_ref(obj)
    struct RStruct *obj;
{
    VALUE nstr, member, slot;
    int i;

    nstr = CLASS_OF(obj);
    member = rb_ivar_get(nstr, rb_intern("__member__"));
    if (member == Qnil) {
	Fail("non-initialized struct");
    }
    slot = INT2FIX(rb_frame_last_func());
    for (i=0; i<RARRAY(member)->len; i++) {
	if (RARRAY(member)->ptr[i] == slot) {
	    return obj->ptr[i];
	}
    }
    Fail("not struct member");
    return Qnil;		/* not reached */
}

static VALUE struct_ref0(obj) struct RStruct *obj; {return obj->ptr[0];}
static VALUE struct_ref1(obj) struct RStruct *obj; {return obj->ptr[1];}
static VALUE struct_ref2(obj) struct RStruct *obj; {return obj->ptr[2];}
static VALUE struct_ref3(obj) struct RStruct *obj; {return obj->ptr[3];}
static VALUE struct_ref4(obj) struct RStruct *obj; {return obj->ptr[4];}
static VALUE struct_ref5(obj) struct RStruct *obj; {return obj->ptr[5];}
static VALUE struct_ref6(obj) struct RStruct *obj; {return obj->ptr[6];}
static VALUE struct_ref7(obj) struct RStruct *obj; {return obj->ptr[7];}
static VALUE struct_ref8(obj) struct RStruct *obj; {return obj->ptr[8];}
static VALUE struct_ref9(obj) struct RStruct *obj; {return obj->ptr[9];}

VALUE (*ref_func[10])() = {
    struct_ref0,
    struct_ref1,
    struct_ref2,
    struct_ref3,
    struct_ref4,
    struct_ref5,
    struct_ref6,
    struct_ref7,
    struct_ref8,
    struct_ref9,
};

static VALUE
struct_set(obj, val)
    struct RStruct *obj;
    VALUE val;
{
    VALUE nstr, member, slot;
    int i;

    nstr = CLASS_OF(obj);
    member = rb_ivar_get(nstr, rb_intern("__member__"));
    if (member == Qnil) {
	Fail("non-initialized struct");
    }
    for (i=0; i<RARRAY(member)->len; i++) {
	slot = RARRAY(member)->ptr[i];
	if (id_attrset(FIX2INT(slot)) == rb_frame_last_func()) {
	    return obj->ptr[i] = val;
	}
    }
    Fail("not struct member");
    return Qnil;		/* not reached */
}

static VALUE struct_s_new();

static VALUE
make_struct(name, member)
    struct RString *name;
    struct RArray *member;
{
    VALUE nstr;
    int i;

    nstr = rb_define_class_under(cStruct, name->ptr, cStruct);
    rb_ivar_set(nstr, rb_intern("__size__"), INT2FIX(member->len));
    rb_ivar_set(nstr, rb_intern("__member__"), member);

    rb_define_singleton_method(nstr, "new", struct_s_new, -1);
    for (i=0; i< member->len; i++) {
	ID id = FIX2INT(member->ptr[i]);
	if (i<10) {
	    rb_define_method_id(nstr, id, ref_func[i], 0);
	}
	else {
	    rb_define_method_id(nstr, id, struct_ref, 0);
	}
	rb_define_method_id(nstr, id_attrset(id), struct_set, 1);
    }

    return nstr;
}

#include <varargs.h>

VALUE
struct_define(name, va_alist)
    char *name;
    va_dcl
{
    va_list ar;
    VALUE nm, ary;
    char *mem;

    nm = str_new2(name);
    ary = ary_new();

    va_start(ar);
    while (mem = va_arg(ar, char*)) {
	ID slot = rb_intern(mem);
	ary_push(ary, INT2FIX(slot));
    }
    va_end(ar);

    return make_struct(nm, ary);
}

static VALUE
struct_s_def(argc, argv)
    int argc;
    VALUE *argv;
{
    struct RString *name;
    struct RArray *rest;
    VALUE nstr;
    int i;

    rb_scan_args(argc, argv, "1*", &name, &rest);
    Check_Type(name, T_STRING);
    for (i=0; i<rest->len; i++) {
	Check_Type(rest->ptr[i], T_FIXNUM);
    }
    return make_struct(name, rest);
}

VALUE
struct_alloc(class, values)
    VALUE class;
    struct RArray *values;
{
    VALUE size;
    int n;

    size = rb_ivar_get(class, rb_intern("__size__"));
    n = FIX2INT(size);
    if (n < values->len) {
	Fail("struct size differs");
    }
    else {
	NEWOBJ(st, struct RStruct);
	OBJSETUP(st, class, T_STRUCT);
	st->len = n;
	st->ptr = ALLOC_N(VALUE, n);
	MEMCPY(st->ptr, values->ptr, VALUE, values->len);
	MEMZERO(st->ptr+values->len, VALUE, n - values->len);

	return (VALUE)st;
    }
    return Qnil;		/* not reached */
}

VALUE
struct_new(class, va_alist)
    VALUE class;
    va_dcl
{
    VALUE val, mem;
    va_list args;

    mem = ary_new();
    va_start(args);
    while (val = va_arg(args, VALUE)) {
	ary_push(mem, val);
    }
    va_end(args);

    return struct_alloc(class, mem);
}

static VALUE
struct_s_new(argc, argv, obj)
    int argc;
    VALUE *argv;
{
    VALUE member, slot;

    member = ary_new4(argc, argv);
    return struct_alloc(obj, member);
}

static VALUE
struct_each(s)
    struct RStruct *s;
{
    int i;

    for (i=0; i<s->len; i++) {
	rb_yield(s->ptr[i]);
    }
    return Qnil;
}

char *rb_class2name();
#define HDR "struct "

static VALUE
struct_to_s(s)
    struct RStruct *s;
{
    char *name, *buf;

    name = rb_class2name(CLASS_OF(s));
    buf = ALLOCA_N(char, strlen(name)+sizeof(HDR)+1);
    sprintf(buf, "%s%s", HDR, name);
    return str_new2(buf);
}

static VALUE
struct_inspect(s)
    struct RStruct *s;
{
    char *name = rb_class2name(CLASS_OF(s));
    ID inspect = rb_intern("inspect");
    VALUE str, member;
    char buf[256];
    int i;

    member = rb_ivar_get(CLASS_OF(s), rb_intern("__member__"));
    if (member == Qnil) {
	Fail("non-initialized struct");
    }

    sprintf(buf, "#<%s%s: ", HDR, name);
    str = str_new2(buf);
    for (i=0; i<s->len; i++) {
	VALUE str2, slot;
	char *p;

	if (i > 0) {
	    str_cat(str, ", ", 2);
	}
	slot = RARRAY(member)->ptr[i];
	p = rb_id2name(FIX2INT(slot));
	str_cat(str, p, strlen(p));
	str_cat(str, "=", 1);
	str2 = rb_funcall(s->ptr[i], inspect, 0, 0);
	str2 = obj_as_string(str2);
	str_cat(str, RSTRING(str2)->ptr, RSTRING(str2)->len);
    }
    str_cat(str, ">", 1);

    return str;
}

static VALUE
struct_to_a(s)
    struct RStruct *s;
{
    return ary_new4(s->len, s->ptr);
}

static VALUE
struct_clone(s)
    struct RStruct *s;
{
    NEWOBJ(st, struct RStruct);
    CLONESETUP(st, s);
    st->len = s->len;
    st->ptr = ALLOC_N(VALUE, s->len);
    MEMCPY(st->ptr, s->ptr, VALUE, st->len);

    return (VALUE)st;
}

static VALUE
struct_aref(s, idx)
    struct RStruct *s;
    VALUE idx;
{
    int i;

    i = NUM2INT(idx);
    if (i < 0) i = s->len - i;
    if (i < 0)
        Fail("offset %d too small for struct(size:%d)", i, s->len);
    if (s->len <= i)
        Fail("offset %d too large for struct(size:%d)", i, s->len);
    return s->ptr[i];
}

static VALUE
struct_aset(s, idx, val)
    struct RStruct *s;
    VALUE idx, val;
{
    int i;

    i = NUM2INT(idx);
    if (i < 0) i = s->len - i;
    if (i < 0)
        Fail("offset %d too small for struct(size:%d)", i, s->len);
    if (s->len <= i)
        Fail("offset %d too large for struct(size:%d)", i, s->len);
    return s->ptr[i] = val;
}

static VALUE
struct_equal(s, s2)
    struct RStruct *s, *s2;
{
    int i;

    if (TYPE(s2) != T_STRUCT) return FALSE;
    if (CLASS_OF(s) != CLASS_OF(s2)) return FALSE;
    if (s->len != s2->len) {
	Fail("incomsistent struct");
    }

    for (i=0; i<s->len; i++) {
	if (!rb_equal(s->ptr[i], s2->ptr[i])) return FALSE;
    }
    return TRUE;
}

static VALUE
struct_hash(s)
    struct RStruct *s;
{
    int i, h;
    ID hash = rb_intern("hash");

    h = CLASS_OF(s);
    for (i=0; i<s->len; i++) {
	h ^= rb_funcall(s->ptr[i], hash, 0);
    }
    return INT2FIX(h);
}

void
Init_Struct()
{
    cStruct = rb_define_class("Struct", cObject);
    rb_include_module(cStruct, mEnumerable);

    rb_define_singleton_method(cStruct, "new", struct_s_def, -1);

    rb_define_method(cStruct, "clone", struct_clone, 0);

    rb_define_method(cStruct, "==", struct_equal, 1);
    rb_define_method(cStruct, "hash", struct_hash, 0);

    rb_define_method(cStruct, "to_s", struct_to_s, 0);
    rb_define_method(cStruct, "inspect", struct_inspect, 0);
    rb_define_method(cStruct, "to_a", struct_to_a, 0);
    rb_define_method(cStruct, "values", struct_to_a, 0);

    rb_define_method(cStruct, "each", struct_each, 0);
    rb_define_method(cStruct, "[]", struct_aref, 1);
    rb_define_method(cStruct, "[]=", struct_aset, 2);
}
