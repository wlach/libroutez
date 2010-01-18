#ifndef __TRIPGRAPH_H
#define __TRIPGRAPH_H
#include <queue>
#include <stdint.h>
#include <string>
#include <tr1/memory>
#include <tr1/unordered_map>
#include <vector>

#include "serviceperiod.h"
#include "trippath.h"
#include "tripstop.h"


class TripGraph
{
  public:
    TripGraph();
    void load(std::string fname);
    void save(std::string fname);    

    void set_timezone(std::string timezone);
    void add_service_period(ServicePeriod &service_period);
    void add_triphop(int32_t start_time, int32_t end_time, int32_t src_id, 
                     int32_t dest_id, int32_t route_id, int32_t trip_id,
                     int32_t service_id);
    void add_tripstop(int32_t id, TripStop::Type type, float lat, float lng);
    void add_walkhop(int32_t src_id, int32_t dest_id);

    void link_osm_gtfs();

    TripStop get_tripstop(int32_t id);

    std::vector<std::pair<int, int> > get_service_period_ids_for_time(int secs);

#ifdef SWIG
    %newobject find_path;
#endif
    TripPath * find_path(double start, bool walkonly,
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
    typedef std::tr1::unordered_map<int32_t, std::tr1::unordered_map<int, std::tr1::shared_ptr<TripPath> > > VisitedRouteMap;
    typedef std::tr1::unordered_map<int32_t, std::tr1::unordered_map<int32_t, std::tr1::shared_ptr<TripPath> > > VisitedWalkMap;
    typedef std::priority_queue<std::tr1::shared_ptr<TripPath>, std::vector<std::tr1::shared_ptr<TripPath> >, PathCompare> PathQueue;
    
    typedef std::vector<ServicePeriod> ServicePeriodList;
    typedef std::vector<std::tr1::shared_ptr<TripStop> > TripStopList;

    std::vector<TripStop> find_tripstops_in_range(double lat, double lng, 
                                                  TripStop::Type type,
                                                  double range);

  private:
    // internal copy of get_tripstop: returns a pointer, not a copy, so
    // much faster (when called many times)
    std::tr1::shared_ptr<TripStop> _get_tripstop(int32_t id);
    std::tr1::shared_ptr<TripStop> get_nearest_stop(double lat, double lng);

    void extend_path(std::tr1::shared_ptr<TripPath> &path, 
                     bool walkonly, int32_t end_id, int &num_paths_considered,
                     VisitedRouteMap &visited_routes, 
                     VisitedWalkMap &visited_walks, 
                     PathQueue &uncompleted_paths, PathQueue &completed_paths);
    
    std::string timezone;
    TripStopList tripstops;
    ServicePeriodList splist;
};

#endif // __TRIPGRAPH_H
