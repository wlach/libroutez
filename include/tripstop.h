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

const int MAX_ID_LEN = 20;

// a triphop represents a hop to a specific node on the graph at a 
// particular time (with a particular duration)
struct TripHop
{
    TripHop() { }

    TripHop(int32_t _start_time, int32_t _end_time, int32_t _dest_id, 
            int32_t _trip_id)
    {
        start_time = _start_time;
        end_time = _end_time;
        dest_id = _dest_id;
        trip_id = _trip_id;
    }

    int32_t start_time;
    int32_t end_time;
    int32_t dest_id;
    int32_t trip_id;
};


struct TripStop
{
    TripStop(FILE *fp);
    TripStop(int32_t _id, std::string _type, float _lat, float _lng);
    TripStop() {}

    void write(FILE *fp);

    void add_triphop(int32_t start_time, int32_t end_time, int32_t dest_id, 
                     int32_t route_id, int32_t trip_id, std::string service_id);
    void add_walkhop(int32_t dest_id, float walktime);
    boost::unordered_set<int> get_routes(std::string service_id);
    boost::shared_ptr<TripHop> find_triphop(int time, int route_id, 
                                            std::string service_period);

    int32_t id;
    char type[MAX_ID_LEN];
    float lat, lng;
    typedef std::vector<boost::shared_ptr<TripHop> > TripHopList;
    typedef boost::unordered_map<int, TripHopList> TripHopDict;
    typedef boost::unordered_map<std::string, TripHopDict> ServiceDict;
    typedef boost::unordered_map<int32_t, float> WalkHopDict;
    ServiceDict tdict;
    WalkHopDict wdict;
};

#endif // __TRIPSTOP_H
