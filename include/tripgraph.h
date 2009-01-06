#ifndef __TRIPGRAPH_H
#define __TRIPGRAPH_H
#include <queue>
#include <stdint.h>
#include <string>
#include <tr1/memory>
#include <vector>

#include <tr1/unordered_map>
#include "trippath.h"
#include "tripstop.h"


class TripGraph
{
  public:
    TripGraph();
    void load(std::string fname);
    void save(std::string fname);

    void add_triphop(int32_t start_time, int32_t end_time, std::string src_id, 
                     std::string dest_id, int32_t route_id,
                     std::string service_id);
    void add_tripstop(std::string id, std::string type, float lat, float lng);
    void add_walkhop(std::string src_id, std::string dest_id);

    void link_osm_gtfs();

    TripStop get_tripstop(std::string id);

    TripPath find_path(int secs, std::string service_period, bool walkonly,
                       double src_lat, double src_lng, 
                       double dest_lat, double dest_lng);

    // various internal types
    struct PathCompare
    {
        inline bool operator() (const std::tr1::shared_ptr<TripPath> &x, 
                                const std::tr1::shared_ptr<TripPath> &y)
        {
            return x->heuristic_weight > y->heuristic_weight;
        }
    };

    typedef std::vector<std::tr1::shared_ptr<TripPath> > TripPathList;
    typedef std::tr1::unordered_map<const char*, std::tr1::unordered_map<int, std::tr1::shared_ptr<TripPath> > > VisitedRouteMap;
    typedef std::tr1::unordered_map<const char*, std::tr1::unordered_map<const char*, std::tr1::shared_ptr<TripPath> > > VisitedWalkMap;
    typedef std::priority_queue<std::tr1::shared_ptr<TripPath>, std::vector<std::tr1::shared_ptr<TripPath> >, PathCompare> PathQueue;
    typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<TripStop> > TripStopDict;
    
  private:
    // internal copy of get_tripstop: returns a pointer, not a copy, so
    // much faster (when called many times)
    std::tr1::shared_ptr<TripStop> _get_tripstop(std::string id);
    std::tr1::shared_ptr<TripStop> get_nearest_stop(double lat, double lng);

    void extend_path(std::tr1::shared_ptr<TripPath> &path, 
                     std::string &service_period, bool walkonly, 
                     const char *end_id, int &num_paths_considered,
                     VisitedRouteMap &visited_routes, 
                     VisitedWalkMap &visited_walks, 
                     PathQueue &uncompleted_paths, PathQueue &completed_paths);
    
    TripStopDict tripstops;
};

#endif // __TRIPGRAPH
