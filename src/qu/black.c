// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// black76
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_QU_BLACK_C
#define SRC_QU_BLACK_C "qu/black.c"

#include <math.h>
#include "../../include/qu/black.h"



// ---------------------------------------------------------------------------------------------------------------------
// black 76
// https://www.glynholton.com/notes/black_1976/
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// qu_b76_call
// ---------------------------------------------------------------------------------------------------------------------
pub double qu_b76_call(double t, double k, double f, double sigma, double df, double (* CN)(double)) {
    double sigmaRootT, sigmaRootTOver2, x, d1, d2;
    sigmaRootT = sigma * sqrt(t);
    sigmaRootTOver2 = sigmaRootT * 0.5;
    x = log(f/k) / sigmaRootT;
    d1 = x + sigmaRootTOver2;
    d2 = x - sigmaRootTOver2;
    return df * (f * CN(d1) - k * CN(d2));
}

// ---------------------------------------------------------------------------------------------------------------------
// qu_b76_call_plusgreeks
// ---------------------------------------------------------------------------------------------------------------------
pub black_greeks qu_b76_call_greeks(double t, double k, double f, double sigma, double r, double df, double (* CN)(double)) {
    double rootT, sigmaRootT, oneOverSigmaRootT, sigmaRootTOver2, x, d1, d2, dfCNd1, fdfCNd1, dfCNd2, c, delta, gamma, vega, theta, rho;
    rootT = sqrt(t);
    sigmaRootT = sigma * rootT;
    sigmaRootTOver2 = sigmaRootT * 0.5;
    oneOverSigmaRootT = 1 / sigmaRootT;
    x = log(f/k) * oneOverSigmaRootT;
    d1 = x + sigmaRootTOver2;
    d2 = x - sigmaRootTOver2;
    dfCNd1 = df * CN(d1);
    fdfCNd1 = f * dfCNd1;
    dfCNd2 = df * CN(d2);
    c = fdfCNd1 - k * dfCNd2;
    delta = dfCNd1;
    gamma = dfCNd1 * oneOverSigmaRootT / f;
    vega = fdfCNd1 * rootT;
    theta = -(fdfCNd1 * sigma) / (2 * rootT) + r * c;
    rho = -t * c;
    return (black_greeks) {c, delta, gamma, vega, theta, rho};
}

// ---------------------------------------------------------------------------------------------------------------------
// qu_b76_put
// ---------------------------------------------------------------------------------------------------------------------
pub double qu_b76_put(double t, double k, double f, double sigma, double df, double (* CN)(double)) {
    double sigmaRootT, sigmaRootTOver2, x, d1, d2;
    sigmaRootT = sigma * sqrt(t);
    sigmaRootTOver2 = sigmaRootT * 0.5;
    x = log(f/k) / sigmaRootT;
    d1 = x + sigmaRootTOver2;
    d2 = x - sigmaRootTOver2;
    return df * (k * CN(-d2) - f * CN(-d1));
}

// ---------------------------------------------------------------------------------------------------------------------
// qu_b76_put_greeks
// ---------------------------------------------------------------------------------------------------------------------
pub black_greeks qu_b76_put_greeks(double t, double k, double f, double sigma, double r, double df, double (* CN)(double)) {
    double rootT, sigmaRootT, oneOverSigmaRootT, sigmaRootTOver2, x, d1, d2, dfCNd1, fdfCNd1, dfCN_d1, dfCN_d2, p, delta, gamma, vega, theta, rho;
    rootT = sqrt(t);
    sigmaRootT = sigma * rootT;
    sigmaRootTOver2 = sigmaRootT * 0.5;
    oneOverSigmaRootT = 1 / sigmaRootT;
    x = log(f/k) * oneOverSigmaRootT;
    d1 = x + sigmaRootTOver2;
    d2 = x - sigmaRootTOver2;
    dfCNd1 = df * CN(d1);
    fdfCNd1 = f * dfCNd1;
    dfCN_d1 = df * CN(-d1);
    dfCN_d2 = df * CN(-d2);
    p = k * dfCN_d2 - f * dfCN_d1;
    delta = dfCNd1 - df;
    gamma = dfCNd1 * oneOverSigmaRootT / f;
    vega = fdfCNd1 * rootT;
    theta = -(fdfCNd1 * sigma) / (2 * rootT) + r * p;
    rho = -t * p;
    return (black_greeks) {p, delta, gamma, vega, theta, rho};
}


// ---------------------------------------------------------------------------------------------------------------------
// black-scholes
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// qu_bs_call
// ---------------------------------------------------------------------------------------------------------------------
pub double qu_bs_call(double t, double k, double s, double sigma, double r, double (* CN)(double)) {
    double sigmaRootT, rt, d1, d2;
    sigmaRootT = sigma * sqrt(t);
    rt = r * t;
    d1 = (log(s / k) + rt + sigma * sigma * t * 0.5) * sigmaRootT;
    d2 = d1 - sigmaRootT;
    return CN(d1) * s - CN(d2) * k * exp(-rt);
}


// ---------------------------------------------------------------------------------------------------------------------
// bachelier
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// qu_bachelier_call
// ---------------------------------------------------------------------------------------------------------------------
pub double qu_bachelier_call(double t, double k, double f, double sigma, double df, double (* CN)(double), double (* NPDF)(double)) {
    double sigmaRootT, D;
    sigmaRootT = sigma * sqrt(t);
    D = (f - k) / sigmaRootT;
    return df * ((f - k) * CN(D) + sigmaRootT * NPDF(D));
}

// ---------------------------------------------------------------------------------------------------------------------
// qu_bachelier_put
// ---------------------------------------------------------------------------------------------------------------------
pub double qu_bachelier_put(double t, double k, double f, double sigma, double df, double (* CN)(double), double (* NPDF)(double)) {
    double sigmaRootT, D;
    sigmaRootT = sigma * sqrt(t);
    D = (f - k) / sigmaRootT;
    return df * ((k - f) * CN(-D) + sigmaRootT * NPDF(D));
}

#endif  // SRC_QU_BLACK_C