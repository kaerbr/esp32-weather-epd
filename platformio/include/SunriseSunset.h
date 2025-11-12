#ifndef SUNRISE_SUNSET_H
#define SUNRISE_SUNSET_H

#include <math.h>
#include <time.h>

// A C-style implementation for sunrise/sunset calculation.
// Based on the algorithm from the "Almanac for Computers, 1990".

// Function to calculate sunrise and sunset for a given date and location.
// Returns sunrise and sunset times as time_t (UTC).
void calculateSunriseSunset(int year, int month, int day, double latitude, double longitude, time_t* sunrise, time_t* sunset) {
    const double RAD = M_PI / 180.0;
    const double DEG = 180.0 / M_PI;
    const double OFFICIAL_ZENITH = 90.833333;

    // 1. Day of the year
    int N = 0;
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        days_in_month[2] = 29;
    }
    for (int i = 1; i < month; ++i) {
        N += days_in_month[i];
    }
    N += day;

    // 2. Approximate time
    double lngHour = longitude / 15.0;
    double t_rise = N + ((6.0 - lngHour) / 24.0);
    double t_set = N + ((18.0 - lngHour) / 24.0);

    // 3. Sun's mean anomaly
    double M_rise = (0.9856 * t_rise) - 3.289;
    double M_set = (0.9856 * t_set) - 3.289;

    // 4. Sun's true longitude
    double L_rise = fmod(M_rise + (1.916 * sin(M_rise * RAD)) + (0.020 * sin(2 * M_rise * RAD)) + 282.634, 360.0);
    if (L_rise < 0) L_rise += 360.0;
    double L_set = fmod(M_set + (1.916 * sin(M_set * RAD)) + (0.020 * sin(2 * M_set * RAD)) + 282.634, 360.0);
    if (L_set < 0) L_set += 360.0;

    // 5. Sun's right ascension
    double RA_rise = atan(0.91764 * tan(L_rise * RAD)) * DEG;
    RA_rise = fmod(RA_rise, 360.0);
    if (RA_rise < 0) RA_rise += 360.0;
    double RA_set = atan(0.91764 * tan(L_set * RAD)) * DEG;
    RA_set = fmod(RA_set, 360.0);
    if (RA_set < 0) RA_set += 360.0;

    double Lquadrant_rise = floor(L_rise / 90.0) * 90.0;
    double RAquadrant_rise = floor(RA_rise / 90.0) * 90.0;
    RA_rise = RA_rise + (Lquadrant_rise - RAquadrant_rise);
    RA_rise /= 15.0;

    double Lquadrant_set = floor(L_set / 90.0) * 90.0;
    double RAquadrant_set = floor(RA_set / 90.0) * 90.0;
    RA_set = RA_set + (Lquadrant_set - RAquadrant_set);
    RA_set /= 15.0;

    // 6. Sun's declination
    double sinDec_rise = 0.39782 * sin(L_rise * RAD);
    double cosDec_rise = cos(asin(sinDec_rise));
    double sinDec_set = 0.39782 * sin(L_set * RAD);
    double cosDec_set = cos(asin(sinDec_set));

    // 7. Sun's local hour angle
    double cosH_rise = (cos(OFFICIAL_ZENITH * RAD) - (sinDec_rise * sin(latitude * RAD))) / (cosDec_rise * cos(latitude * RAD));
    double cosH_set = (cos(OFFICIAL_ZENITH * RAD) - (sinDec_set * sin(latitude * RAD))) / (cosDec_set * cos(latitude * RAD));

    if (cosH_rise > 1.0 || cosH_set > 1.0) { // Sun never rises
        *sunrise = 0;
        *sunset = 0;
        return;
    }
    if (cosH_rise < -1.0 || cosH_set < -1.0) { // Sun never sets
        *sunrise = 0;
        *sunset = 0;
        return;
    }

    // 8. Finish calculating H and convert to hours
    double H_rise = acos(cosH_rise) * DEG / 15.0;
    double H_set = (360.0 - acos(cosH_set) * DEG) / 15.0;

    // 9. Calculate local mean time of rising/setting
    double T_rise = H_rise + RA_rise - (0.06571 * t_rise) - 6.622;
    double T_set = H_set + RA_set - (0.06571 * t_set) - 6.622;

    // 10. Adjust back to UTC
    double UT_rise = fmod(T_rise - lngHour, 24.0);
    if (UT_rise < 0) UT_rise += 24.0;
    double UT_set = fmod(T_set - lngHour, 24.0);
    if (UT_set < 0) UT_set += 24.0;

    // 11. Convert to time_t
    struct tm t_info = {0};
    t_info.tm_year = year - 1900;
    t_info.tm_mon = month - 1;
    t_info.tm_mday = day;

    t_info.tm_hour = (int)UT_rise;
    t_info.tm_min = (int)((UT_rise - t_info.tm_hour) * 60);
    t_info.tm_sec = 0;
    *sunrise = mktime(&t_info);

    t_info.tm_hour = (int)UT_set;
    t_info.tm_min = (int)((UT_set - t_info.tm_hour) * 60);
    *sunset = mktime(&t_info);
}

#endif // SUNRISE_SUNSET_H
