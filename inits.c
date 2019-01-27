/************************************************

  inits.c -

  $Author: matz $
  $Date: 1995/01/10 10:42:38 $
  created at: Tue Dec 28 16:01:58 JST 1993

  Copyright (C) 1993-1995 Yukihiro Matsumoto

************************************************/

#include "ruby.h"

void
rb_call_inits()
{
    Init_sym();
    Init_var_tables();
    Init_Object();
    Init_GC();
    Init_eval();
    Init_Comparable();
    Init_Enumerable();
    Init_Numeric();
    Init_Bignum();
    Init_Array();
    Init_Hash();
    Init_Struct();
    Init_String();
    Init_Regexp();
    Init_pack();
    Init_Range();
    Init_IO();
    Init_Dir();
    Init_Time();
    Init_Random();
    Init_signal();
    Init_process();
    Init_load();
    Init_Proc();
    Init_Math();
    Init_ext();
    Init_version();
}
