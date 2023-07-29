// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
//
// distributions
// ---------------------------------------------------------------------------------------------------------------------

#ifndef SRC_QU_DISTS_C
#define SRC_QU_DISTS_C "qu/dists.c"

#include <math.h>
#include "../../include/qu/qu.h"



// ---------------------------------------------------------------------------------------------------------------------
// qu_cn_Hart
// from BETTER APPROXIMATIONS TO CUMULATIVE NORMAL FUNCTIONS By GRAEME WEST
// ---------------------------------------------------------------------------------------------------------------------

pub double qu_cn_Hart(double x) {
    double xabs, ans;
    if (isnan(x)) return NAN;
    if (!isfinite(x)) return (x < 0 ? 0.0 : 1.0);
    xabs = fabs(x);
    if (xabs > 37.0) return 0.0;
    if (xabs < 7.07106781186547) {
        ans = exp(-xabs * xabs * 0.5)
             * (((((((3.52624965998911E-02 * xabs + 0.700383064443688)) * xabs + 6.37396220353165) * xabs + 33.912866078383) * xabs + 112.079291497871) * xabs + 221.213596169931) * xabs + 220.206867912376)
             / (((((((8.83883476483184E-02 * xabs + 1.75566716318264) * xabs + 16.064177579207) * xabs + 86.7807322029461) * xabs + 296.564248779674) * xabs + 637.333633378831) * xabs + 793.826512519948) * xabs + 440.413735824752);
    } else {
        ans = exp(-xabs * xabs * 0.5) / (xabs + 1.0 / (xabs + 2.0 / (xabs + 3.0 / (xabs + 4.0 / (xabs + 0.65))))) / 2.506628274631;
    }
    return (x < 0.0) ? ans : 1.0 - ans;
}


// ---------------------------------------------------------------------------------------------------------------------
// qu_invcn_Acklam
// http://home.online.no/~pjacklam/notes/invnorm/index.html
// http://home.online.no/~pjacklam/notes/invnorm/impl/herrero/inversecdf.txt
// ---------------------------------------------------------------------------------------------------------------------

pub double qu_invcn_Acklam(double p) {
    double q, t, u;

    const double a[6] = {
        -3.969683028665376e+01,  2.209460984245205e+02,
        -2.759285104469687e+02,  1.383577518672690e+02,
        -3.066479806614716e+01,  2.506628277459239e+00
    };
    const double b[5] = {
        -5.447609879822406e+01,  1.615858368580409e+02,
        -1.556989798598866e+02,  6.680131188771972e+01,
        -1.328068155288572e+01
    };
    const double c[6] = {
        -7.784894002430293e-03, -3.223964580411365e-01,
        -2.400758277161838e+00, -2.549732539343734e+00,
        4.374664141464968e+00,  2.938163982698783e+00
    };
    const double d[4] = {
        7.784695709041462e-03,  3.224671290700398e-01,
        2.445134137142996e+00,  3.754408661907416e+00
    };

    if (isnan(p) || p < 0.0 || 1.0 < p) return NAN;
    if (p == 0.0) return -INFINITY;
    if (p == 1.0) return  INFINITY;
    q = p < (t=1-p) ? p : t;
    if (q > 0.02425) {
        // Rational approximation for central region.
        u = q - 0.5;
        t = u * u;
        u = u*(((((a[0]*t+a[1])*t+a[2])*t+a[3])*t+a[4])*t+a[5]) / (((((b[0]*t+b[1])*t+b[2])*t+b[3])*t+b[4])*t+1);
    } else {
        // Rational approximation for tail region.
        t = sqrt(-2 * log(q));
        u = (((((c[0]*t+c[1])*t+c[2])*t+c[3])*t+c[4])*t+c[5]) / ((((d[0]*t+d[1])*t+d[2])*t+d[3])*t+1);
    }
    t = qu_cn_Hart(u) - q;
    t = t * QU_SQRT_TWO_PI * exp(u * u * 0.5);      // Newton step, i.e. f(u)/df(u)
    u = u - t / (1 + u * t * 0.5);                  // polish with Halley's method

    return (p > 0.5 ? -u : u);
}


// ---------------------------------------------------------------------------------------------------------------------
// qu_cn_h
// ---------------------------------------------------------------------------------------------------------------------

const double d_A[6] = {
    0.0,                  3.16112374387056560,  113.864154151050156,
    377.485237685302021,  3209.37758913846947,  0.185777706184603153
};

const double d_B[5] = {
    0.0,                  23.6012909523441209,  244.024637934444173,  1282.61652607737228,  2844.23683343917062
};

const double d_C[10] = {
    0.0,                  0.564188496988670089,  8.88314979438837594,  66.1191906371416295,  298.635138197400131,
    881.952221241769090,  1712.04761263407058,   2051.07837782607147,  1230.33935479799725,  2.15311535474403846e-8
};

const double d_D[9] = {
    0.0,                  15.7449261107098347,  117.693950891312499,  537.181101862009858,  1621.38957456669019,
    3290.79923573345963,  4362.61909014324716,  3439.36767414372164,  1230.33935480374942
};

const double d_E[7] = {
    0.0,                  0.305326634961232344, 0.360344899949804439, 0.125781726111229246, 0.0160837851487422766,
    0.000658749161529837803, 0.0163153871373020978
};

const double d_F[6] = {
    0.0,                  2.56852019228982242,  1.87295284992346047,  0.527905102951428412,  0.0605183413124413191,
    0.00233520497626869185
};

pub double qu_cn_h(double x) {
    double d, y, y2, ans, Xnum, Xden, one_over_y2;  int i;

    if (isnan(x)) return NAN;
    if (!isfinite(x)) return (x < 0 ? 0.0 : 1.0);

    d = x * QU_ONE_OVER_SQRT_TWO;    // Change of variables to evaluate erf()
    y = fabs(d);
    y2 = y * y;

    if (y <= 0.46875) {
        // Evaluate erf for |d| <= 0.46875
        Xnum = d_A[5] * y2;
        Xden = y2;
        for (i = 1; i < 4; i++) {
            Xnum = (Xnum + d_A[i]) * y2;
            Xden = (Xden + d_B[i]) * y2;
        }
        ans = d * (Xnum + d_A[4]) / (Xden + d_B[4]);
        return 0.5 * (1.0 + ans);
    }
    else if (y <= 4.0) {
        // Evaluate erfc for 0.46875 <= |d| <= 4.0
        Xnum = d_C[9] * y;
        Xden = y;
        for (i = 1; i < 8; i++) {
           Xnum = (Xnum + d_C[i]) * y;
           Xden = (Xden + d_D[i]) * y;
        }
        ans = (Xnum + d_C[8]) / (Xden + d_D[8]);
        ans *= exp(-y2);

        // Fix up for negative argument, erf, etc.
        return (d > 0) ? 1.0 - 0.5 * ans : 0.5 * ans;
    }
    else {
        // Evaluate erfc for |X| > 4.0
        one_over_y2 = 1 / y2;
        Xnum = d_E[6] * one_over_y2;
        Xden = one_over_y2;
        for (i = 1; i < 5; i++) {
           Xnum = (Xnum + d_E[i]) * one_over_y2;
           Xden = (Xden + d_F[i]) * one_over_y2;
        }
        ans = one_over_y2 * (Xnum + d_E[5]) / (Xden + d_F[5]);
        ans = (QU_ONE_OVER_SQRT_PI - ans) / y;
        ans *= exp(-y2);

        // Fix up for negative argument, erf, etc.
        return (d > 0) ? 1.0 - 0.5 * ans : 0.5 * ans;
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// qu_invcn_h
// ---------------------------------------------------------------------------------------------------------------------

const double d_InvA[4] = {2.50662823884,   -18.61500062529,  41.39119773534,   -25.44106049637};

const double d_InvB[4] = {-8.47351093090,  23.08336743743,   -21.06224101826,  3.13082909833};

const double d_InvC[9] = {
    0.3374754822726147,  0.9761690190917186,  0.1607979714918209,  0.0276438810333863,  0.0038405729373609,
    0.0003951896511919,  0.0000321767881768,  0.0000002888167364,  0.0000003960315187,
};

pub double qu_invcn_h(double p) {
    double x, r, ret, error, ns;

    if (isnan(p) || p < 0.0 || 1.0 < p) return NAN;
    if (p == 0.0) return -INFINITY;
    if (p == 1.0) return  INFINITY;

    x = p - 0.5;

    if (fabs(x) < 0.42) {
        r = x * x;
        ret = x * (((d_InvA[3] * r + d_InvA[2]) * r + d_InvA[1]) * r + d_InvA[0]) / ((((d_InvB[3] * r + d_InvB[2]) * r + d_InvB[1]) * r + d_InvB[0]) * r + 1.0);
    }
    else {
        r = (x > 0.0) ? 1.0 - p : p;
        r = log(-log(r));
        r = d_InvC[0] + r * (d_InvC[1] + r * (d_InvC[2] + r * (d_InvC[3] + r * (d_InvC[4] + r * (d_InvC[5] + r * (d_InvC[6] + r * (d_InvC[7] + r * d_InvC[8])))))));
        ret = (x > 0.0) ? r : -r;
    }

    error = qu_cn_h(ret) - p;
    ns = error * QU_SQRT_TWO_PI * exp(0.5 * ret * ret);     // Newton step, i.e. f(u)/df(u)
    return ret - ns / (1 + ret * ns * 0.5);                 // polish with Halley's method
    // return ret - ns;                                     // polish with one iteration of Newton's method
}


// ---------------------------------------------------------------------------------------------------------------------
// qu_norm_pdf
// ---------------------------------------------------------------------------------------------------------------------

pub double qu_norm_pdf(double x) {
    if (isnan(x)) return NAN;
    if (!isfinite(x)) return 0;
    return QU_ONE_OVER_SQRT_TWO_PI * exp(-0.5 * x * x);
}


#endif  // SRC_QU_DISTS_C