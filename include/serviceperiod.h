#ifndef __SERVICEPERIOD_H
#define __SERVICEPERIOD_H
#include <stdint.h>
#include <string>


struct ServicePeriod
{
    ServicePeriod(std::string id, 
                  int32_t start_mday, int32_t start_mon, int32_t start_year, 
                  int32_t end_mday, int32_t end_mon, int32_t end_year, 
                  bool weekday, bool saturday, bool sunday);
    ServicePeriod(const ServicePeriod &s);
    ServicePeriod();
    ServicePeriod(FILE *fp);

    void write(FILE *fp);

    std::string id;
    int32_t start_mday;
    int32_t start_mon;
    int32_t start_year;
    int32_t end_mday;
    int32_t end_mon;
    int32_t end_year; 
    bool weekday; 
    bool saturday;
    bool sunday;
};

#endif // __SERVICEPERIOD_H
