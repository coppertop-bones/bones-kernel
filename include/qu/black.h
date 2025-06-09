// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// black - black style options pricing
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_QU_BLACK_H
#define INC_QU_BLACK_H "qu/black.h"

#include "qu.h"


typedef struct {
    double price;
    double delta;
    double gamma;
    double vega;
    double theta;
    double rho;
} black_greeks;


pub double qu_b76_call(double t, double k, double f, double sigma, double df, double (* CN)(double));
pub black_greeks qu_b76_call_greeks(double t, double k, double f, double sigma, double r, double df, double (* CN)(double));
pub double qu_b76_put(double t, double k, double f, double sigma, double df, double (* CN)(double));
pub black_greeks qu_b76_put_greeks(double t, double k, double f, double sigma, double r, double df, double (* CN)(double));

pub double qu_bs_call(double t, double k, double s, double sigma, double r, double (* CN)(double));
pub double qu_bs_put(double t, double k, double s, double sigma, double r, double (* CN)(double));

pub double qu_bachelier_call(double t, double k, double s, double sigma, double df, double (* CN)(double), double (* NPDF)(double));
pub double qu_bachelier_put(double t, double k, double s, double sigma, double df, double (* CN)(double), double (* NPDF)(double));


#endif  // INC_QU_BLACK_H
