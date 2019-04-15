/*
 * Small statics library
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cheap_stats.h"

#define DEFAULT_ERROR         __LINE__
#define MIN_SAMPLES           10

#define ALLOC(t)              ((t*)malloc(sizeof(t)))
#define NALLOC(t,n)           ((t*)malloc(sizeof(t) * (n)))
#define FREE(var)             do {free(var);var = NULL;} while (0)
#define SHRINK(n)             ((n * 10) / 13)
#define SWAP(a,b)             do {double t; t = b; b = a; a = t;} while(0)

static void
combsort11(double* a, size_t n)
{
  int h;
  int f;
  int i;

  /*
   * sort by ascending order
   */

  h = n;

  while (h > 1 || f) {
    f = 0;
    h = SHRINK(h);

    if (h == 9 || h == 10) h = 11;

    for (i = 0; i < (n - h); i++) {
      if (a[i] > a[i + h]) {
        SWAP(a[i], a[i + h]);
        f = !0;
      }
    }
  }
}

static int
binsearch(double* a, size_t n, double v)
{
  int l;
  int r;
  int ret;

  l = 0;
  r = n - 1;

  while (1) {
    ret = (l + r) / 2;

    if (r <= l) break;

    if (a[ret] < v) {
      l = ret + 1;
      continue;
    } 
    
    if (a[ret] > v) {
      r = ret - 1;
      continue;
    }

    break;
  }

  return ret;
}

static double
calc_sum(double* a, size_t n)
{
  int i;
  double s;

  s = 0;

  for (i = 0; i < n; i++) {
    s += a[i];
  }

  return s;
}

static double
calc_variance(double* a, size_t n, double mean)
{
  int i;

  double d;
  double s;

  s = 0;

  for (i = 0; i < n; i++) {
    d  = a[i] - mean; 
    s += (d * d);
  }

  return s / n;
}

static double
calc_cdf(double* a, size_t n, double v)
{
  int idx;

  if (v > a[n - 1]) {
    idx = n;
  } else {
    idx = binsearch(a, n, v);
  }

  return (double)idx / n;
}

static double
calc_moment(double* a, size_t n, double k)
{
  double s;
  int i;

  for (i = 0; i < n; i++) {
    s += pow(a[i], k);
  }

  return s / n;
}

static double
calc_central_moment(double* a, size_t n, double k, double mean)
{
  double s;
  int i;

  for (i = 0; i < n; i++) {
    s += pow(a[i] - mean, k);
  }

  return s / n;
}

static double
calc_std_moment(double* a, size_t n, double k, double mean, double std)
{
  return calc_central_moment(a, n, k, mean) / pow(std, k);
}

static double
calc_normal_pdf(double* a, size_t n,
                double mean, double std, double total, double v)
{
  double t;

  t = (v - mean) / std;

  // 2.50662827463 == sqrt(2.0 * M_PI)
  return (exp(-0.5 * (t * t)) / (std * 2.50662827463)) / total;
}

static double
kernel_gaussian(double x)
{
  // 2.50662827463 == sqrt(2.0 * M_PI)
  return exp(-(x * x) / 2.0) / 2.50662827463;
}

static double
calc_kde(double* a, size_t n, double sig, double v)
{
  double h;
  double s;
  int i;

  h = (0.9 * sig) / pow(n, 1.0 / 5.0);
  s = 0.0;

  for (i = 0; i < n; i++) {
    s += kernel_gaussian((v - a[i]) / h);
  }

  return (s / (n * h));
}

int
cheap_stats_new(double* src, size_t n, cheap_stats_t** dst)
{
	int ret;
  double* a0;
  double* a1;
  cheap_stats_t* ptr;

  /*
   * initialize
   */
  ret = 0;
  ptr = NULL;
  a0  = NULL;
  a1  = NULL;

  /*
   * argument check
   */
  do {
    if (src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (n < MIN_SAMPLES) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * alloc memory
   */
  if (!ret) do {
    a0 = NALLOC(double, n);
    if (a0 == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    a1 = NALLOC(double, n);
    if (a1 == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    ptr = ALLOC(cheap_stats_t);
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * put return parameter
   */
  if (!ret) {
    memcpy(a0, src, sizeof(double) * n);
    memcpy(a1, src, sizeof(double) * n);
    combsort11(a1, n);

    ptr->a0       = a0;
    ptr->a1       = a1;
    ptr->n        = n;
    ptr->total    = calc_sum(a0, n);
    ptr->mean     = ptr->total / n;
    ptr->min      = ptr->a1[0];
    ptr->max      = ptr->a1[n - 1];
    ptr->q1       = ptr->a1[n / 4];
    ptr->q3       = ptr->a1[(3 * n) / 4];
    ptr->median   = ptr->a1[n / 2];
    ptr->variance = calc_variance(a0, n, ptr->mean);
    ptr->std      = sqrt(ptr->variance);

    *dst = ptr;
  }

  /*
   * post process
   */
  if (ret) {
    if (ptr) free(ptr);
    if (a0) free(a0);
    if (a1) free(a1);
  }

  return ret;
}

int
cheap_stats_destroy(cheap_stats_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) {
    ret = DEFAULT_ERROR;
  }

  /*
   * release memory
   */
  if (!ret) {
    if (ptr->a0) free(ptr->a0);
    if (ptr->a1) free(ptr->a1);
    free(ptr);
  }

  return ret;
}

int
cheap_stats_cdf(cheap_stats_t* ptr, double v, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc CDF
   */
  if (!ret) {
    *dst = calc_cdf(ptr->a1, ptr->n, v); 
  }

  return ret;
}

int
cheap_stats_normal_pdf(cheap_stats_t* ptr, double v, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc CDF
   */
  if (!ret) {
    *dst = calc_normal_pdf(ptr->a1, ptr->n, ptr->mean, ptr->std, ptr->total, v);
  }

  return ret;
}

int
cheap_stats_estimated_pdf(cheap_stats_t* ptr, double v, double* dst)
{
  int ret;
  double sig;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc CDF
   */
  if (!ret) {
    sig  = ptr->q3 - ptr->q1;
    if (sig > ptr->std) sig = ptr->std;

    *dst = calc_kde(ptr->a1, ptr->n, sig, v); 
  }

  return ret;
}


int
cheap_stats_moment(cheap_stats_t* ptr, double k, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc raw moment
   */
  if (!ret) {
    *dst = calc_moment(ptr->a1, ptr->n, k); 
  }

  return ret;
}

int
cheap_stats_central_moment(cheap_stats_t* ptr, double k, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc raw moment
   */
  if (!ret) {
    *dst = calc_central_moment(ptr->a1, ptr->n, k, ptr->mean); 
  }

  return ret;
}

int
cheap_stats_std_moment(cheap_stats_t* ptr, double k, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc raw moment
   */
  if (!ret) {
    *dst = calc_std_moment(ptr->a1, ptr->n, k, ptr->mean, ptr->std); 
  }

  return ret;
}

int
cheap_stats_skewness(cheap_stats_t* ptr, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc raw moment
   */
  if (!ret) {
    *dst = calc_std_moment(ptr->a1, ptr->n, 3.0, ptr->mean, ptr->std); 
  }

  return ret;
}

int
cheap_stats_pearson_skewness(cheap_stats_t* ptr, double* dst)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  do {
    if (ptr == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * calc raw moment
   */
  if (!ret) {
    *dst = (3.0 * (ptr->mean - ptr->median)) / (ptr->std + 1e-15);
  }

  return ret;
}

