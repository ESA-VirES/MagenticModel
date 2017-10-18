/**
 * @file sph_harm.h
 * @author Martin Paces <martin.paces@eox.at>
 * @brief Sun Position
 *
 * Calculation of the Solar position.
 *
 * This code is derived from the code kindly provided by Stephan Buchert.
 *
 * Also See:
 * R. Grena (2012), Five new algorithms for the computation of sun
 * position from 2010 to 2110, Solar Energy, vol. 86(5), 1323-1337,
 * doi:10.1016/j.solener.2012.01.024
 * Ph.Blanc (2012), The SG2 algorithm for a fast and accurate computation
 * of the position of the Sun for multi-decadal time period, Solar Energy,
 * vol. 86(5), 3072-3083, doi:10.1016/j.solener.2012.07.018
 *
 * Copyright (C) 2016 EOX IT Services GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies of this Software or works derived from this Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <math.h>

#ifndef SEC2DAYS
#define SEC2DAYS (1.0/86400.0)
#endif

#ifndef PI
#define PI 3.14159265358979
#endif

#define PI2 6.28318530717959 // 2*PI
#define PIM 1.57079632679490 // PI/2

/**
 * @brief Evaluate solar position in equatorial coordinate system.
 *
 * Algorithm #5 - part 1
 *
 * R. Grena (2012), Five new algorithms for the computation of sun
 * position from 2010 to 2110, Solar Energy, vol. 86(5), 1323-1337,
 * doi:10.1016/j.solener.2012.01.024
 * Ph.Blanc (2012), The SG2 algorithm for a fast and accurate computation
 * of the position of the Sun for multi-decadal time period, Solar Energy,
 * vol. 86(5), 3072-3083, doi:10.1016/j.solener.2012.07.018
 *
 *  outputs:
 *      decl - declination [rad]
 *      rasc - right ascension [rad] (eastward)
 *      hang - hour angle [rad] (westward!)
 *
 *  inputs:
 *      mjd2k  - Modified Julian Data 2000 [days]
 *      dtt - UTC offset to TT [seconds]
 *      lon - longitude [rad] (eastward)
 */

void sunpos5equat(
    double *decl, double *rasc, double *hang,
    double mjd2k, double dtt ,double lon
)
{
    // t is decimal number of days since 1939-12-31
    const double t = mjd2k + 42.0 - 21958.0;
    const double te = SEC2DAYS*dtt + t; // Terestrial Time correction
    const double wte = 0.0172019715*te;
    const double s1 = sin(wte);
    const double c1 = cos(wte);
    const double s2 = 2.0*s1*c1;
    const double c2 = (c1+s1)*(c1-s1);
    const double s3 = s2*c1 + c2*s1;
    const double c3 = c2*c1 - s2*s1;

    const double l = (
        1.7527901 + 1.7202792159e-2*te + 3.33024e-2*s1 - 2.0582e-3*c1 +
        3.512e-4*s2 - 4.07e-5*c2 + 5.2e-6*s3 - 9e-7*c3 -
        8.23e-5*s1*sin(2.92e-5*te) + 1.27e-5*sin(1.49e-3*te - 2.337) +
        1.21e-5*sin(4.31e-3*te + 3.065) + 2.33e-5*sin(1.076e-2*te - 1.533) +
        3.49e-5*sin(1.575e-2*te - 2.358) + 2.67e-5*sin(2.152e-2*te + 0.074) +
        1.28e-5*sin(3.152e-2*te + 1.547) + 3.14e-5*sin(2.1277e-1*te - 0.488)
    );

    const double nu = 9.282e-4*te - 0.8;
    const double dlam = 8.34e-5*sin(nu);
    const double lambda = l + PI + dlam;
    const double epsi = 4.089567e-1 - 6.19e-9*te + 4.46e-5*cos(nu);

    const double sl = sin(lambda);
    const double cl = cos(lambda);
    const double se = sin(epsi);
    const double ce = sqrt(1-se*se);

    // equatorial coordinates
    const double _rasc0 = atan2(sl*ce, cl);
    const double _rasc = _rasc0 + (_rasc0 < 0.0 ? PI2 : 0.0);
    const double _decl = asin(sl*se);
    const double _hang0 = (
        // 1.7528311 + 6.300388099*t Greenwich Sideral Time approximation [rad]
        1.7528311 + 6.300388099*t + lon - _rasc + 0.92*dlam
    );
    const double _hang = fmod(_hang0 + PI, PI2) - PI;

    *rasc = _rasc;
    *decl = _decl;
    *hang = _hang;
}

/**
 * @brief Convert solar equatorial to horizontal coordinates.
 *
 * Algorithm #5 part 2
 *
 * R. Grena (2012), Five new algorithms for the computation of sun
 * position from 2010 to 2110, Solar Energy, vol. 86(5), 1323-1337,
 * doi:10.1016/j.solener.2012.01.024
 * Ph.Blanc (2012), The SG2 algorithm for a fast and accurate computation
 * of the position of the Sun for multi-decadal time period, Solar Energy,
 * vol. 86(5), 3072-3083, doi:10.1016/j.solener.2012.07.018
 *
 *  outputs:
 *      zenith - zenith angle [rad]
 *      azimuth - azimuth angle [rad]
 *
 *  inputs:
 *      decl - declination [rad]
 *      hang - hour angle [rad] (westward!)
 *      lat - latitude [rad]
 *      rad - radius (distance from Earth centre) [km]
 *            (set to zero to disable paralax correction)
 */

void sunpos5eq2hor(
    double *zenith, double *azimuth,
    double decl, double hang, double lat, double rad
)
{
    // horizontal coordinates
    // move conversion equatorial to horizontal coords to a separate function
    const double sp = sin(lat);
    const double cp = sqrt((1-sp*sp));
    const double sd = sin(decl);
    const double cd = sqrt(1-sd*sd);
    const double sh = sin(hang);
    const double ch = cos(hang);
    const double se0 = sp*sd + cp*cd*ch;

    // elevation including parallax correction with radius in km
    const double ep = asin(se0) - sqrt(1.0-se0*se0)*rad/149597871.0;

    // NOTE: refraction correction is skipped
    //const double de = ep > 0.0 ? (0.08422*pres) / ((273.0+temp)*tan(ep + 0.003138/(ep + 0.08919))) : 0.0;

    *azimuth = atan2(sh, ch*sp - sd*cp/cd);
    *zenith = PIM - ep; // - de;
}