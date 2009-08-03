#ifndef __SERVICEPERIOD_H
#define __SERVICEPERIOD_H
#include <stdint.h>
#include <string>


struct ServicePeriod
{
    ServicePeriod(std::string id, 
                  int32_t start_mday, int32_t start_mon, int32_t start_year, 
                  int32_t end_mday, int32_t end_mon, int32_t end_year, 
                  int32_t duration, bool weekday, bool saturday, bool sunday);
    ServicePeriod(const ServicePeriod &s);
    ServicePeriod();
    ServicePeriod(FILE *fp);

    void write(FILE *fp);

    std::string id;

    // start/end time: the range of dates for which the service period is 
    // valid (e.g. Jan 2008 - Sep 2009)
    time_t start_time;
    time_t end_time;

    // duration and days of the week that the service period is active
    int32_t duration;
    bool weekday; 
    bool saturday;
    bool sunday;
};

#endif // __SERVICEPERIOD_H
