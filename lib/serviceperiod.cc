#include <string.h>
#include <assert.h>
#include "serviceperiod.h"
#include "defuns.h"


ServicePeriod::ServicePeriod(std::string _id, int32_t _start_mday, 
                             int32_t _start_mon, int32_t _start_year, 
                             int32_t _end_mday, int32_t _end_mon, 
                             int32_t _end_year, int32_t _duration, 
                             bool _weekday, bool _saturday, 
                             bool _sunday)
{
    id = _id;
    start_mday = _start_mday;
    start_mon = _start_mon;
    start_year = _start_year;
    end_mday = _end_mday;
    end_mon = _end_mon;
    end_year = _end_year; 
    duration = _duration;
    weekday = _weekday; 
    saturday = _saturday;
    sunday = _sunday;
}


ServicePeriod::ServicePeriod(const ServicePeriod &s) 
{
    id = s.id;
    start_mday = s.start_mday;
    start_mon = s.start_mon;
    start_year = s.start_year;
    end_mday = s.end_mday;
    end_mon = s.end_mon;
    end_year = s.end_year; 
    duration = s.duration;
    weekday = s.weekday; 
    saturday = s.saturday;
    sunday = s.sunday;
}


ServicePeriod::ServicePeriod()
{
    // blank service period object
    start_mday = 0;
    start_mon = 0;
    start_year = 0;
    end_mday = 0;
    end_mon = 0;
    end_year = 0;
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

    assert(fread(&start_mday, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&start_mon, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&start_year, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&end_mday, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&end_mon, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&end_year, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&duration, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&weekday, sizeof(bool), 1, fp) == 1);
    assert(fread(&saturday, sizeof(bool), 1, fp) == 1);
    assert(fread(&sunday, sizeof(bool), 1, fp) == 1);
}


void ServicePeriod::write(FILE *fp)
{
    char spstr[MAX_ID_LEN];
    strcpy(spstr, id.c_str());
    assert(fwrite(&spstr, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);

    assert(fwrite(&start_mday, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&start_mon, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&start_year, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&end_mday, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&end_mon, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&end_year, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&duration, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&weekday, sizeof(bool), 1, fp) == 1);
    assert(fwrite(&saturday, sizeof(bool), 1, fp) == 1);
    assert(fwrite(&sunday, sizeof(bool), 1, fp) == 1);
}
