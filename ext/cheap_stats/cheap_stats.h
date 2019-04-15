/*
 * Small statics library
 *
 *  Copyright (C) 2019 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

#ifndef __SMALL_STATS_H__
#define __SMALL_STATS_H__

#include <stdlib.h>

typedef struct {
  double* a0;
  double* a1; // sorted
  size_t n;

  double total;
  double mean;
  double min;
  double max;
  double q1;
  double q3;
  double median;
  double variance;
  double std;
} cheap_stats_t;

int cheap_stats_new(double* samples, size_t size, cheap_stats_t** obj);
int cheap_stats_destroy(cheap_stats_t* obj);
int cheap_stats_cdf(cheap_stats_t* obj, double v, double* dst);
int cheap_stats_moment(cheap_stats_t* obj, double k, double* dst);
int cheap_stats_central_moment(cheap_stats_t* obj, double k, double* dst);
int cheap_stats_std_moment(cheap_stats_t* obj, double k, double* dst);
int cheap_stats_skewness(cheap_stats_t* obj, double* dst);
int cheap_stats_pearson_skewness(cheap_stats_t* obj, double* dst);

#endif /* !defined(__SMALL_STATS_H__) */
