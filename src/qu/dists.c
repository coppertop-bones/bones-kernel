// ---------------------------------------------------------------------------------------------------------------------
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
    double xabs, result;
    if (isnan(x)) return NAN;
    if (!isfinite(x)) return (x < 0 ? 0.0 : 1.0);
    xabs = fabs(x);
    if (xabs > 37.0) return 0.0;
    if (xabs < 7.07106781186547) {
        result = exp(-xabs * xabs * 0.5)
             *  (((((((3.52624965998911E-02 * xabs + 0.700383064443688)) * xabs + 6.37396220353165) * xabs + 33.912866078383) * xabs + 112.079291497871) * xabs + 221.213596169931) * xabs + 220.206867912376)
             / (((((((8.83883476483184E-02 * xabs + 1.75566716318264) * xabs + 16.064177579207) * xabs + 86.7807322029461) * xabs + 296.564248779674) * xabs + 637.333633378831) * xabs + 793.826512519948) * xabs + 440.413735824752);
    } else {
        result = exp(-xabs * xabs * 0.5) / (xabs + 1.0 / (xabs + 2.0 / (xabs + 3.0 / (xabs + 4.0 / (xabs + 0.65))))) / 2.506628274631;
    }
    if (x < 0.0) {
        return result;
    } else {
        return 1.0 - result;
    }
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
        u = u*(((((a[0]*t+a[1])*t+a[2])*t+a[3])*t+a[4])*t+a[5])
            /(((((b[0]*t+b[1])*t+b[2])*t+b[3])*t+b[4])*t+1);
    } else {
        // Rational approximation for tail region.
        t = sqrt(-2 * log(q));
        u = (((((c[0]*t+c[1])*t+c[2])*t+c[3])*t+c[4])*t+c[5])
            /((((d[0]*t+d[1])*t+d[2])*t+d[3])*t+1);
    }
    // The relative error of the approximation has absolute value less than 1.15e-9.  One iteration of Halley's
    // rational method (third order) gives full machine precision...
    t = qu_cn_Hart(u) - q;
    t = t * QU_SQRT2PI * exp(u * u * 0.5);      // f(u)/df(u)
    u = u - t / (1 + u * t * 0.5);              // Halley's method

    return (p > 0.5 ? -u : u);
}


#endif  // SRC_QU_DISTS_C