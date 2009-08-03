#include <string.h>
#include <assert.h>
#include "serviceperiod.h"
#include "defuns.h"


static time_t get_time_t(int tm_mday, int tm_mon, int tm_year)
{
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = tm_mday;
    t.tm_mon = tm_mon;
    t.tm_year = tm_year;
    t.tm_wday = -1;
    t.tm_yday = -1;
    t.tm_isdst = -1;

    return mktime(&t);
}

ServicePeriod::ServicePeriod(std::string _id, int32_t _start_mday, 
                             int32_t _start_mon, int32_t _start_year, 
                             int32_t _end_mday, int32_t _end_mon, 
                             int32_t _end_year, int32_t _duration, 
                             bool _weekday, bool _saturday, 
                             bool _sunday)
{
    id = _id;

    start_time = get_time_t(_start_mday, _start_mon, _start_year);
    end_time = get_time_t(_end_mday, _end_mon, _end_year);

    duration = _duration;
    weekday = _weekday; 
    saturday = _saturday;
    sunday = _sunday;
}


ServicePeriod::ServicePeriod(const ServicePeriod &s) 
{
    id = s.id;

    start_time = s.start_time;
    end_time = s.end_time; 

    duration = s.duration;
    weekday = s.weekday; 
    saturday = s.saturday;
    sunday = s.sunday;
}


ServicePeriod::ServicePeriod()
{
    // blank service period object
    start_time = 0;
    end_time = 0;

    duration = 0;
    weekday = false; 
    saturday = false;
    sunday = false;
}


ServicePeriod::ServicePeriod(FILE *fp)
{
    char _id[MAX_ID_LEN];
    assert(fread(_id, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
    id = _id;

    assert(fread(&start_time, sizeof(time_t), 1, fp) == 1);
    assert(fread(&end_time, sizeof(time_t), 1, fp) == 1);
    assert(fread(&duration, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&weekday, sizeof(bool), 1, fp) == 1);
    assert(fread(&saturday, sizeof(bool), 1, fp) == 1);
    assert(fread(&sunday, sizeof(bool), 1, fp) == 1);
}


void ServicePeriod::write(FILE *fp)
{
    char spstr[MAX_ID_LEN];
    strncpy(spstr, id.c_str(), MAX_ID_LEN);
    assert(fwrite(spstr, sizeof(char), MAX_ID_LEN, fp) == MAX_ID_LEN);

    assert(fwrite(&start_time, sizeof(time_t), 1, fp) == 1);
    assert(fwrite(&end_time, sizeof(time_t), 1, fp) == 1);

    assert(fwrite(&duration, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&weekday, sizeof(bool), 1, fp) == 1);
    assert(fwrite(&saturday, sizeof(bool), 1, fp) == 1);
    assert(fwrite(&sunday, sizeof(bool), 1, fp) == 1);
}
