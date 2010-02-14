#ifndef __SERVICEPERIOD_H
#define __SERVICEPERIOD_H
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <vector>


struct ServicePeriodException
{
    ServicePeriodException(int32_t _tm_mday, int32_t _tm_mon, int32_t _tm_year);
    ServicePeriodException();

    int32_t tm_mday;
    int32_t tm_mon;
    int32_t tm_year;
};

class ServicePeriod
{
  public:
    ServicePeriod(int32_t id,
                  int32_t start_mday, int32_t start_mon, int32_t start_year, 
                  int32_t end_mday, int32_t end_mon, int32_t end_year, 
                  int32_t duration, bool weekday, bool saturday, bool sunday);
    ServicePeriod(const ServicePeriod &s);
    ServicePeriod();
    ServicePeriod(FILE *fp);
    
    void add_exception_on(int32_t tm_mday, int32_t tm_mon, int32_t tm_year);
    void add_exception_off(int32_t tm_mday, int32_t tm_mon, int32_t tm_year);

    bool is_turned_on(int32_t tm_mday, int32_t tm_mon, int32_t tm_year);
    bool is_turned_off(int32_t tm_mday, int32_t tm_mon, int32_t tm_year);

    void write(FILE *fp);

    int32_t id;

    // start/end time: the range of dates for which the service period is 
    // valid (e.g. Jan 2008 - Sep 2009)
    time_t start_time;
    time_t end_time;

    // duration and days of the week that the service period is active
    int32_t duration;
    bool weekday; 
    bool saturday;
    bool sunday;

    // days that the service period is off (regardless of what the normal 
    // schedule) says. E.g. a weekday sched on Xmas
    std::vector<ServicePeriodException> exceptions_off;

    // days that the service period is on (regardless of what the normal 
    // schedule) says. E.g. a sunday sched on Xmas
    std::vector<ServicePeriodException> exceptions_on;
};

#endif // __SERVICEPERIOD_H
