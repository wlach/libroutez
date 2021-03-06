#ifndef __TRIPSTOP_H
#define __TRIPSTOP_H
#include <assert.h>
#include <tr1/memory>
#include <tr1/unordered_map>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <deque>
#include <list>


// a triphop represents a hop to a specific node on the graph at a 
// particular time (with a particular duration)
struct TripHop
{
    TripHop() { }

    TripHop(int32_t _start_time, int32_t _end_time, int32_t _dest_id, 
            int32_t _trip_id, int32_t _headsign_id)
    {
        start_time = _start_time;
        end_time = _end_time;
        dest_id = _dest_id;
        trip_id = _trip_id;
        headsign_id = _headsign_id;
    }

    int32_t start_time;
    int32_t end_time;
    int32_t dest_id;
    int32_t trip_id;
    int32_t headsign_id;
};


struct WalkHop
{
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

    void write(FILE *fp);

    void add_triphop(int32_t start_time, int32_t end_time, int32_t dest_id, 
                     int32_t route_id, int32_t trip_id, int32_t service_id,
                     int32_t headsign_id);
    void add_walkhop(int32_t dest_id, float walktime);
    std::deque<int> get_routes(int32_t service_id);
    const TripHop * find_triphop(int time, int route_id, int32_t service_id);
    std::vector<TripHop> find_triphops(
        int time, int route_id, int32_t service_id, int num);

    typedef std::vector<TripHop> TripHopList;
    typedef std::tr1::unordered_map<int, TripHopList> TripHopDict;
    typedef std::tr1::unordered_map<int32_t, TripHopDict> ServiceDict;

    // we keep a shared pointer to a tdict, as most nodes won't have one and
    // we don't want the memory overhead of one if not strictly needed
    // (note: we use a shared pointer instead of a standard pointer because
    // the same tripstop may have multiple instances, but we only want one
    // instance of its internal servicedict because it can be really huge...)
    std::tr1::shared_ptr<ServiceDict> tdict;

    typedef std::list<WalkHop> WalkHopList;
    WalkHopList wlist;
};

#endif // __TRIPSTOP_H
