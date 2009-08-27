#ifndef __TRIPSTOP_H
#define __TRIPSTOP_H
#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <list>


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


struct WalkHop
{
    WalkHop() { }
    WalkHop(int32_t _dest_id, float _walktime)
    {
        dest_id = _dest_id;
        walktime = _walktime;
    }

    int32_t dest_id;
    float walktime;
};


struct TripStop
{
    int32_t id;
    enum Type { OSM, GTFS };
    Type type;
    float lat, lng;

    TripStop(FILE *fp);
    TripStop(int32_t _id, Type _type, float _lat, float _lng);
    TripStop();
    ~TripStop();

    void write(FILE *fp);

    void add_triphop(int32_t start_time, int32_t end_time, int32_t dest_id, 
                     int32_t route_id, int32_t trip_id, int32_t service_id);
    void add_walkhop(int32_t dest_id, float walktime);
    std::list<int> get_routes(int32_t service_id);
    const TripHop * find_triphop(int time, int route_id, int32_t service_id);
    std::vector<TripHop> find_triphops(
        int time, int route_id, int32_t service_id, int num);

    typedef std::vector<TripHop> TripHopList;
    typedef boost::unordered_map<int, TripHopList> TripHopDict;
    typedef boost::unordered_map<int32_t, TripHopDict> ServiceDict;
    typedef boost::unordered_map<int32_t, float> WalkHopDict;

    // we keep a pointer to a tdict, tas most nodes won't have one and we
    // don't want the memory overhead of one if not strictly needed
    ServiceDict * tdict;

    typedef std::list<WalkHop> WalkHopList;
    WalkHopList wlist;
};

#endif // __TRIPSTOP_H
