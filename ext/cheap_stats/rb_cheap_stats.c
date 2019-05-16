/*
 * cheap statistics library for ruby
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ruby.h"

#include "cheap_stats.h"

#define N(x)                      (sizeof(x)/sizeof(*x))

#define RUNTIME_ERROR(msg, ...)   rb_raise(rb_eRuntimeError, (msg), __VA_ARGS__)
#define ARGUMENT_ERROR(msg, ...)  rb_raise(rb_eArgError, (msg), __VA_ARGS__)
#define TYPE_ERROR(msg, ...)      rb_raise(rb_eTypeError, (msg), __VA_ARGS__)
#define NOMEMORY_ERROR(msg, ...)  rb_raise(rb_eNoMemError, (msg), __VA_ARGS__)

#define API_SIMPLIFIED            1
#define API_CLASSIC               2

#define EQ_STR(val,str)           (rb_to_id(val) == rb_intern(str))
#define EQ_INT(val,n)             (FIX2INT(val) == n)
#define IS_NUMERIC(t) \
      ((t) == T_FLOAT || (t) ==  T_FIXNUM || (t) == T_BIGNUM)

typedef struct {
  cheap_stats_t* stats;
} rb_cheap_stats_t;

VALUE klass;

static size_t
rb_cheap_stats_size(const void* _ptr)
{
  rb_cheap_stats_t* ptr;

  ptr = (rb_cheap_stats_t*)_ptr;

  return sizeof(*ptr) + ((sizeof(double) * ptr->stats->n) * 2);
}

static void
rb_cheap_stats_free(void* _ptr)
{
  rb_cheap_stats_t* ptr;

  ptr = (rb_cheap_stats_t*)_ptr;

  if (ptr->stats != NULL) {
    cheap_stats_destroy(ptr->stats);
    ptr->stats = NULL;
  }

  xfree(ptr);
}

static const struct rb_data_type_struct rb_cheap_stats_data_type = {
  "A Cheap satatics library",
  {
    NULL,
    rb_cheap_stats_free,
    rb_cheap_stats_size,
    {NULL, NULL}
  },
  NULL,
  NULL,
};

static VALUE
rb_cheap_stats_alloc(VALUE self)
{
  rb_cheap_stats_t* ptr;

  ptr = ALLOC(rb_cheap_stats_t);
  memset(ptr, 0, sizeof(*ptr));

  return TypedData_Wrap_Struct(klass, &rb_cheap_stats_data_type, ptr);
}

/**
 * initialize object
 *
 * @params [Array<Numeric>] samples   sample vaules.
 */
static VALUE
rb_cheap_stats_initialize(VALUE self, VALUE samples)
{
  rb_cheap_stats_t* ptr;
  VALUE exp;
  const char* msg;
  int err;
  double *a;
  int i;
  int n;
  VALUE v;
  int t;
  char str[64];

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  Check_Type(samples, T_ARRAY);

  do {
    exp = Qnil;
    a   = NULL;

    /*
     * allocate work buffer
     */
    n   = RARRAY_LEN(samples);
    a   = malloc(sizeof(double) * n);

    if (a == NULL) {
      exp = rb_eNoMemError;
      msg = "Memory allocation failed";
      break;
    }

    /*
     * copy source value
     */
    for (i = 0; i < n; i++) {
      v = RARRAY_AREF(samples, i);
      t = TYPE(v);

      if (!IS_NUMERIC(t)) break;

      a[i] = NUM2DBL(v);
    }

    if (i != n) {
      exp = rb_eTypeError;
      msg = "the value that not numeric was included";
    }

    /*
     * create statistic context
     */
    err = cheap_stats_new(a, n, &ptr->stats);
    if (err) {
      sprintf(str, "cheap_stats_new() failed [err=%d]", err); 

      exp = rb_eRuntimeError;
      msg = str;
      break;
    }
  } while (0);

  /*
   * post porcess 
   */
  if (a != NULL) free(a);

  if (exp != Qnil) {
    if (ptr->stats != NULL) {
      cheap_stats_destroy(ptr->stats);
      ptr->stats = NULL;
    }

    rb_raise(exp, msg);
  }

  return self;
}

/**
 * get total value of samples
 *
 * @return [Float] total value
 */
static VALUE
rb_cheap_stats_total(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->total);
}

/**
 * get mean of samples
 *
 * @return [Float] mean
 */
static VALUE
rb_cheap_stats_mean(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->mean);
}

/**
 * get min value of samples
 *
 * @return [Float] min value
 */
static VALUE
rb_cheap_stats_min(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->min);
}

/**
 * get max value of samples
 *
 * @return [Float] max value
 */
static VALUE
rb_cheap_stats_max(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->max);
}

/**
 * get 1/4 quartile value of samples
 *
 * @return [Float] 1/4 quartile value
 */
static VALUE
rb_cheap_stats_q1(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->q1);
}

/**
 * get 3/4 quartile value of samples
 *
 * @return [Float] 3/4 quartile value
 */
static VALUE
rb_cheap_stats_q3(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->q3);
}

/**
 * get median of samples
 *
 * @return [Float] median
 */
static VALUE
rb_cheap_stats_median(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->median);
}

/**
 * get standard division of samples
 *
 * @return [Float] standard division
 */
static VALUE
rb_cheap_stats_std(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->std);
}

/**
 * get variance of samples
 *
 * @return [Float] variance
 */
static VALUE
rb_cheap_stats_variance(VALUE self)
{
  rb_cheap_stats_t* ptr;

  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  return DBL2NUM(ptr->stats->variance);
}

/**
 * calc CDF
 *
 * @param [Numeric] x   taregt value
 *
 * @return [Float] CDF value that coresspoinding to target.
 */
static VALUE
rb_cheap_stats_cdf(VALUE self, VALUE x)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_cdf(ptr->stats, rb_num2dbl(x), &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_cdf() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}

/**
 * calc moment
 *
 * @param [Numeric] k   order number
 *
 * @return [Float] moment value
 */
static VALUE
rb_cheap_stats_moment(VALUE self, VALUE k)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_moment(ptr->stats, rb_num2dbl(k), &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_moment() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}

/**
 * calc central moment
 *
 * @param [Numeric] k   order number
 *
 * @return [Float] moment value
 */
static VALUE
rb_cheap_stats_central_moment(VALUE self, VALUE k)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_central_moment(ptr->stats, rb_num2dbl(k), &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_central_moment() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}

/**
 * calc std (standard division) moment
 *
 * @param [Numeric] k   order number
 *
 * @return [Float] moment value
 */
static VALUE
rb_cheap_stats_std_moment(VALUE self, VALUE k)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_std_moment(ptr->stats, rb_num2dbl(k), &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_std_moment() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}

/**
 * calc skewness value
 *
 * @return [Float] skewness value
 */
static VALUE
rb_cheap_stats_skewness(VALUE self)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_skewness(ptr->stats,  &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_skewness() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}

/**
 * calc pearson median skewness value
 *
 * @return [Float] skewness value
 */
static VALUE
rb_cheap_stats_pearson_skewness(VALUE self)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_pearson_skewness(ptr->stats,  &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_pearson_skewness() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}

/**
 * calc Z-score
 *
 * @param [Numeric] v   target value
 *
 * @return [Float] skewness value
 */
static VALUE
rb_cheap_stats_z_score(VALUE self, VALUE v)
{
  rb_cheap_stats_t* ptr;
  int err;
  double ret;

  /*
   * strip context data
   */
  TypedData_Get_Struct(self, rb_cheap_stats_t, &rb_cheap_stats_data_type, ptr);

  /*
   * check argument
   */
  err = cheap_stats_z_score(ptr->stats, rb_num2dbl(v), &ret);
  if (err) {
    RUNTIME_ERROR("cheap_stats_z_score() failed [err=%d]", err);
  }

  return DBL2NUM(ret);
}


void
Init_cheap_stats()
{
  klass = rb_define_class("CheapStats", rb_cObject);

  rb_define_alloc_func(klass, rb_cheap_stats_alloc);

  rb_define_method(klass, "initialize", rb_cheap_stats_initialize, 1);
  rb_define_method(klass, "total", rb_cheap_stats_total, 0);
  rb_define_method(klass, "min", rb_cheap_stats_min, 0);
  rb_define_method(klass, "max", rb_cheap_stats_max, 0);
  rb_define_method(klass, "q1", rb_cheap_stats_q1, 0);
  rb_define_method(klass, "q3", rb_cheap_stats_q3, 0);
  rb_define_method(klass, "mean", rb_cheap_stats_mean, 0);
  rb_define_method(klass, "median", rb_cheap_stats_median, 0);
  rb_define_method(klass, "variance", rb_cheap_stats_variance, 0);
  rb_define_method(klass, "std", rb_cheap_stats_std, 0);
  rb_define_method(klass, "cdf", rb_cheap_stats_cdf, 1);
  rb_define_method(klass, "moment", rb_cheap_stats_moment, 1);
  rb_define_method(klass, "central_moment", rb_cheap_stats_central_moment, 1);
  rb_define_method(klass, "std_moment", rb_cheap_stats_std_moment, 1);
  rb_define_method(klass, "skewness", rb_cheap_stats_skewness, 0);
  rb_define_method(klass, "pearson_skewness",rb_cheap_stats_pearson_skewness,0);
  rb_define_method(klass, "z_score",rb_cheap_stats_z_score, 1);

  rb_alias(klass, rb_intern("average"), rb_intern("mean"));
  rb_alias(klass, rb_intern("sigma"), rb_intern("std"));
}
