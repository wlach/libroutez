#ifndef __TRIPSTOP_H
#define __TRIPSTOP_H
#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>

#define MAX_ID_LEN 16

// a triphop represents a hop to a specific node on the graph at a 
// particular time (with a particular duration)
struct TripHop
{
    TripHop() { }

    TripHop(int32_t _start_time, int32_t _end_time, std::string _dest_id, 
            int32_t _trip_id)
    {
        start_time = _start_time;
        end_time = _end_time;
        assert(_dest_id.length() < MAX_ID_LEN);
        strcpy(dest_id, _dest_id.c_str());
        trip_id = _trip_id;
    }

    int32_t start_time;
    int32_t end_time;
    char dest_id[MAX_ID_LEN];
    int32_t trip_id;
};


struct TripStop
{
    TripStop(FILE *fp);
    TripStop(std::string _id, std::string _type, float _lat, float _lng);
    TripStop() {}

    void write(FILE *fp);

    void add_triphop(int32_t start_time, int32_t end_time, std::string dest_id, 
                     int32_t route_id, int32_t trip_id, std::string service_id);
    void add_walkhop(std::string dest_id, float walktime);
    boost::unordered_set<int> get_routes(std::string service_id);
    boost::shared_ptr<TripHop> find_triphop(int time, int route_id, 
                                            std::string service_period);

    char id[MAX_ID_LEN];
    char type[MAX_ID_LEN];
    float lat, lng;
    typedef std::vector<boost::shared_ptr<TripHop> > TripHopList;
    typedef boost::unordered_map<int, TripHopList> TripHopDict;
    typedef boost::unordered_map<std::string, TripHopDict> ServiceDict;
    typedef boost::unordered_map<std::string, float> WalkHopDict;
    ServiceDict tdict;
    WalkHopDict wdict;
};

#endif // __TRIPSTOP_H
