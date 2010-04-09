#include "tripgraph.h"
#include <assert.h>
#include <errno.h>
#include <map>
#include <math.h>
#include <stdlib.h>

using namespace std;
using namespace tr1;

// set to 1 to see what find_path is doing (VERY verbose)
#if 0
# define DEBUGPATH(fmt, args...) fprintf(stderr, fmt, ## args)
#else
# define DEBUGPATH
#endif

// Estimated walking speed in m/s
static const float EST_WALK_SPEED = 1.1f;
static int SECS_IN_DAY = (60*60*24);


static inline double radians(double degrees)
{
    return degrees/180.0f*M_PI;
}

static inline double degrees(double radians)
{
    return radians*180.0f/M_PI;
}

static double distance(double src_lat, double src_lng, double dest_lat, 
                       double dest_lng)
{
    // returns distance in meters
    static const double EPSILON = 0.00005;
    
    if (fabs(src_lat - dest_lat) < EPSILON && fabs(src_lng - dest_lng) < EPSILON) {
        return 0.0f;
    }

    double theta = src_lng - dest_lng;
    double src_lat_radians = radians(src_lat);
    double dest_lat_radians = radians(dest_lat);
    double dist = sin(src_lat_radians) * sin(dest_lat_radians) + 
                 cos(src_lat_radians) * cos(dest_lat_radians) * 
                 cos(radians(theta));
    dist = acos(dist);
    dist = degrees(dist);
    dist *= (60.0f * 1.1515 * 1.609344 * 1000.0f);
    return dist;
}


TripGraph::TripGraph()
{
    set_timezone("UTC");
}


void TripGraph::load(string fname)
{
    FILE *fp = fopen(fname.c_str(), "r");
    if (!fp)
    {
        printf("Error: Couldn't open graph file %s: %s (%d).\n", 
            fname.c_str(), strerror(errno), errno);
        return;
    }

    uint32_t timezone_len;
    assert(fread(&timezone_len, sizeof(uint32_t), 1, fp) == 1);
    char tz[timezone_len+1];
    assert(fread(tz, sizeof(char), timezone_len, fp) == timezone_len);
    tz[timezone_len] = '\0';
    set_timezone(tz);
    
    uint32_t num_service_periods;
    if (fread(&num_service_periods, sizeof(uint32_t), 1, fp) != 1)
    {
        printf("Error: Couldn't read the number of service periods.\n");
        return;
    }
    for (int i=0; i < num_service_periods; i++)
    {
        ServicePeriod s(fp);
        add_service_period(s);
    }
        
    uint32_t num_tripstops;
    if (fread(&num_tripstops, sizeof(uint32_t), 1, fp) != 1)
    {
        printf("Error: Couldn't read the number of tripstops.\n");
        return;
    }
        
    tripstops.reserve(num_tripstops);
    for (uint32_t i=0; i < num_tripstops; i++)
    {
        shared_ptr<TripStop> s(new TripStop(fp));
        assert(tripstops.size() == s->id);
        tripstops.push_back(s);
    }

    fclose(fp);
}


void TripGraph::save(string fname)
{
    FILE *fp = fopen(fname.c_str(), "w");
    if (!fp)
    {
        printf("Error: Couldn't open graph %s for writing: %s (%d).\n", 
            fname.c_str(), strerror(errno), errno);
        return;
    }

    // write timezone
    uint32_t timezone_len = timezone.size();
    assert(fwrite(&timezone_len, sizeof(uint32_t), 1, fp) == 1);
    assert(fwrite(timezone.c_str(), sizeof(char), timezone_len, fp) == 
           timezone_len);

    // write service periods
    uint32_t num_service_periods = splist.size();
    assert(fwrite(&num_service_periods, sizeof(uint32_t), 1, fp) == 1);
    for (ServicePeriodList::iterator i = splist.begin(); i != splist.end();
         i++)
        i->write(fp);

    // write tripstops
    uint32_t num_tripstops = tripstops.size();
    assert(fwrite(&num_tripstops, sizeof(uint32_t), 1, fp) == 1);
    for (TripStopList::iterator i = tripstops.begin();
         i != tripstops.end(); i++)
    {
        (*i)->write(fp);
    }

    fclose(fp);
}


void TripGraph::set_timezone(std::string _timezone)
{
    timezone = _timezone;
    setenv("TZ", timezone.c_str(), 1);
    tzset();
}


void TripGraph::add_service_period(ServicePeriod &service_period)
{
    assert(service_period.id == splist.size());
    splist.push_back(service_period);
}


void TripGraph::add_triphop(int32_t start_time, int32_t end_time, 
                            int32_t src_id, int32_t dest_id, int32_t route_id, 
                            int32_t trip_id, int32_t service_id)
{
    // will assert if src_id doesn't exist!!
    _get_tripstop(src_id)->add_triphop(start_time, end_time, dest_id, route_id, 
                                       trip_id, service_id);
}


void TripGraph::add_tripstop(int32_t id, TripStop::Type type, float lat, float lng)
{
    // id must equal size of tripstops
    assert(id == tripstops.size());

    tripstops.push_back(shared_ptr<TripStop>(new TripStop(id, type, lat, lng)));
}


void TripGraph::add_walkhop(int32_t src_id, int32_t dest_id)
{
    // will assert if src_id or dest_id doesn't exist!!
    shared_ptr<TripStop> ts_src = _get_tripstop(src_id);
    shared_ptr<TripStop> ts_dest = _get_tripstop(dest_id);

    double dist = distance(ts_src->lat, ts_src->lng,
                           ts_dest->lat, ts_dest->lng);

    ts_src->add_walkhop(dest_id, dist / EST_WALK_SPEED);
}


struct Point
{
    Point(double _lat, double _lng) { lat=_lat; lng=_lng; }
    double lat;
    double lng;
};

bool operator==(const Point &p1, const Point &p2)
{
    // We say that anything within a distance of 1 meter is identical.
    return (distance(p1.lat, p1.lng, p2.lat, p2.lng) < 1.0f);
}

Point get_closest_point(Point &a, Point &b, Point &c)
{
    // Given a line made up of a and b, and a point c,
    // return the point on the line closest to c (may be a or b).
    double ab2 = pow((b.lat - a.lat), 2) + pow((b.lng - a.lng), 2);
    double ap_ab = (c.lat - a.lat)*(b.lat-a.lat) + (c.lng-a.lng)*(b.lng-a.lng);
    double t = ap_ab / ab2;
 
    // Clamp t to be between a and b.
    if (t < 0.0f)
        t = 0.0f;
    else if (t>1.0f)
        t = 1.0f;
    
    return Point(a.lat + (b.lat - a.lat)*t, a.lng + (b.lng - a.lng)*t);
}


// This complicated-looking method attempts to link gtfs stops to osm nodes.
// If a stop lies between two osm nodes on a polyline, we will link the gtfs
// stop to both of them.
void TripGraph::link_osm_gtfs()
{
    map<int32_t, pair<int32_t, int32_t> > new_walkhops;

    // do some counting of the actual number of gtfs
    int gtfs_tripstop_count = 0;
    int gtfs_tripstop_total = 0;
    for (TripStopList::iterator i = tripstops.begin(); 
         i != tripstops.end(); i++)
    {
        if ((*i)->type == TripStop::GTFS)
            gtfs_tripstop_total++;
    }

    for (TripStopList::iterator i = tripstops.begin(); 
         i != tripstops.end(); i++)
    {
        gtfs_tripstop_count++;
        // For each GTFS stop...
        if ((*i)->type == TripStop::GTFS)
        {
            Point gtfs_pt((*i)->lat, (*i)->lng);
            
            pair<int32_t, int32_t> nearest_walkhop(-1, -1);
            double min_dist;

            // Check each other trip stop and all its walkhops...
            // FIXME: This is begging to be optimized.  We need some way to
            // exclude the bulk of tripstops that are a million miles away.
            // One idea is to do some sort of quadtree-like partitioning of
            // the tripstops; then we'd mostly only have to check other stops
            // within our partition.
            // Another idea is to put a bounding box around each tripstop and
            // its associated walkhops, saving us from having to examine each
            // walkhop of some faraway triphop.
            for (TripStopList::iterator j = tripstops.begin(); 
                 j != tripstops.end(); j++)
            {
                for (TripStop::WalkHopList::iterator k = (*j)->wlist.begin(); 
                     k != (*j)->wlist.end(); k++)
                {
                    Point trip_pt((*j)->lat, (*j)->lng);

                    shared_ptr<TripStop> dest_stop = _get_tripstop(k->dest_id);
                    Point walk_pt(dest_stop->lat, dest_stop->lng);

                    Point p = get_closest_point(trip_pt, walk_pt, gtfs_pt);

                    // Find the closest OSM hop to the GTFS stop
                    double dist = distance(gtfs_pt.lat, gtfs_pt.lng, 
                                           p.lat, p.lng);
                    if ((nearest_walkhop.first == (-1) && 
                         nearest_walkhop.second == (-1)) || dist < min_dist)
                    {
                        nearest_walkhop = pair<int32_t,int32_t>(-1, -1);
                        // If the GTFS stop is on one of the OSM nodes, use
                        // that node.  Otherwise remember both nodes.
                        if (trip_pt == p)
                            nearest_walkhop.first = (*j)->id;
                        else if (walk_pt == p)
                            nearest_walkhop.first = k->dest_id;
                        else
                        {
                            nearest_walkhop.first = (*j)->id;
                            nearest_walkhop.second = k->dest_id;
                        }

                        min_dist = dist;
                    }
                }
            }
            
            new_walkhops[(*i)->id] = nearest_walkhop;
            printf("%02.2f%% done: Linking %d -> %d, %d\n", 
                    ((float)gtfs_tripstop_count * 100.0f) / ((float)gtfs_tripstop_total),
                    (*i)->id, 
                    nearest_walkhop.first, 
                    nearest_walkhop.second);
        }
    }

    for (map<int32_t, pair<int32_t, int32_t> >::iterator i = new_walkhops.begin();
         i != new_walkhops.end(); i++)
    {
        int32_t osmstop1 = i->second.first;
        int32_t osmstop2 = i->second.second;

        assert(osmstop1 >= 0);
        add_walkhop(i->first, osmstop1);
        add_walkhop(osmstop1, i->first);

        if (osmstop2 >= 0)
        {
            add_walkhop(i->first, osmstop2);
            add_walkhop(osmstop2, i->first);
        }
    }
}


shared_ptr<TripStop> TripGraph::get_nearest_stop(double lat, double lng)
{
    // FIXME: use a quadtree to speed this up, see link_osm_gtfs() for
    // more thoughts on this
    
    shared_ptr<TripStop> closest_stop;
    double min_dist = 0.0f;
    for (TripStopList::iterator i = tripstops.begin(); 
         i != tripstops.end(); i++)
    {
        double dist = pow(((*i)->lat - lat), 2) + pow(((*i)->lng - lng), 2);
        if (!closest_stop || dist < min_dist)
        {
            closest_stop = (*i);
            min_dist = dist;
        }
    }

    return closest_stop;
}


TripStop TripGraph::get_tripstop(int32_t id)
{
    shared_ptr<TripStop> ts = _get_tripstop(id);
    return TripStop(*ts);
}


vector<pair<int, int> > TripGraph::get_service_period_ids_for_time(int secs)
{
    vector<pair<int, int> > vsp;

    for (ServicePeriodList::iterator i = splist.begin(); i != splist.end(); i++)
    {
        for (int offset = 0; offset < i->duration; offset += SECS_IN_DAY)
        {
            time_t mysecs = secs - offset;
            struct tm * t = localtime(&mysecs);
            if (i->start_time <= mysecs &&
                i->end_time >= mysecs &&
                (((t->tm_wday == 6 && i->saturday) ||
                  (t->tm_wday == 0 && i->sunday) ||
                 (t->tm_wday > 0 && t->tm_wday < 6 && i->weekday)) &&
                 !i->is_turned_off(t->tm_mday, t->tm_mon, t->tm_year)) ||
                i->is_turned_on(t->tm_mday, t->tm_mon, t->tm_year))
            {
                vsp.push_back(pair<int, int>(i->id, offset));
            }
        }
    }

    return vsp;
}


TripPath * TripGraph::find_path(double start, bool walkonly,
                                double src_lat, double src_lng, 
                                double dest_lat, double dest_lng)
{
    PathQueue uncompleted_paths;
    PathQueue completed_paths;
        
    VisitedRouteMap visited_routes;
    VisitedWalkMap visited_walks;

    shared_ptr<TripStop> start_node = get_nearest_stop(src_lat, src_lng);
    shared_ptr<TripStop> end_node = get_nearest_stop(dest_lat, dest_lng);
    DEBUGPATH("Find path. Secs: %f walkonly: %d "
              "src lat: %f src lng: %f dest_lat: %f dest_lng: %f\n",
              start, walkonly, src_lat, src_lng, dest_lat, dest_lng);
    DEBUGPATH("- Start: %d End: %d\n", start_node->id, end_node->id);

    //DEBUGPATH("..service period determination..");

    // Consider the distance required to reach the start node from the 
    // beginning, and add that to our start time.
    double dist_from_start = distance(src_lat, src_lng, 
                                      start_node->lat, start_node->lng);
    start += (dist_from_start / EST_WALK_SPEED);

    DEBUGPATH("- Start time - %f (dist from start: %f)\n", start, dist_from_start);
    shared_ptr<TripPath> start_path(new TripPath(start, EST_WALK_SPEED, 
                                                 end_node, start_node));
    if (start_node == end_node)
        return new TripPath(*start_path);

    uncompleted_paths.push(start_path);

    int num_paths_considered = 0;

    while (uncompleted_paths.size() > 0)
    {
        DEBUGPATH("Continuing\n");
        shared_ptr<TripPath> path = uncompleted_paths.top();
        uncompleted_paths.pop();
        extend_path(path, walkonly, end_node->id, num_paths_considered, 
                    visited_routes, visited_walks, uncompleted_paths, 
                    completed_paths);
        
        // If we've still got open paths, but their weight exceeds that
        // of the weight of a completed path, break.
        if (uncompleted_paths.size() > 0 && completed_paths.size() > 0 &&
            uncompleted_paths.top()->heuristic_weight > 
            completed_paths.top()->heuristic_weight)
        {
            DEBUGPATH("Breaking with %d uncompleted paths (paths "
                      "considered: %d).\n", uncompleted_paths.size(), 
                      num_paths_considered);
            return new TripPath(*(completed_paths.top()));
        }
        
        //if len(completed_paths) > 0 and len(uncompleted_paths) > 0:
        //  print "Weight of best completed path: %s, uncompleted: %s" % \
        //      (completed_paths[0].heuristic_weight, uncompleted_paths[0].heuristic_weight)
    }
    
    if (completed_paths.size())
        return new TripPath(*(completed_paths.top()));

    return NULL;
}


shared_ptr<TripStop> TripGraph::_get_tripstop(int32_t id)
{
    assert(id < tripstops.size());

    return tripstops[id];
}


void TripGraph::extend_path(shared_ptr<TripPath> &path,
                            bool walkonly,
                            int32_t goal_id,
                            int &num_paths_considered,
                            VisitedRouteMap &visited_routes,
                            VisitedWalkMap &visited_walks,
                            PathQueue &uncompleted_paths,
                            PathQueue &completed_paths)
{
    TripPathList newpaths;
    int32_t src_id = path->last_stop->id;
    int last_route_id = path->last_route_id;

#if 0
    if (path->last_action)
    {
        string last_src_id = path->last_action->src_id;
        if (cb)
            python::call<void>(cb, tripstops[last_src_id]->lat, 
                               tripstops[last_src_id]->lng,
                               tripstops[src_id]->lat, 
                               tripstops[src_id]->lng,
                               last_route_id);
    }
#endif
    time_t mysecs = (time_t)path->time;
    struct tm * tm = localtime(&mysecs);
    double elapsed_daysecs = tm->tm_sec + (60*tm->tm_min) + (60*60*tm->tm_hour);
    double daystart = path->time - elapsed_daysecs;

    // Figure out service period based on start time, then figure out
    // seconds since midnight on our particular day
    vector<pair<int, int> > vsp = get_service_period_ids_for_time(path->time);

    DEBUGPATH("Extending path at vertex %d (on %d) @ %f (walktime: %f, "
              "routetime: %f elapsed_daysecs: %f)\n", src_id, last_route_id, path->time, 
              path->walking_time, path->route_time, elapsed_daysecs);
    shared_ptr<TripStop> src_stop = _get_tripstop(src_id);

    // Keep track of outgoing route ids at this node: make sure that we 
    // don't get on a route later when we could have gotten on here.
    deque<int> outgoing_route_ids;
    if (!walkonly)
    {
        for (vector<pair<int, int> >::iterator i = vsp.begin(); i != vsp.end(); i++)
        {
            deque<int> route_ids = src_stop->get_routes(i->first); 
            for (deque<int>::iterator j = route_ids.begin(); j != route_ids.end(); j++) 
                outgoing_route_ids.push_back(*j);
        }
    }

    // Explore walkhops that are better than the ones we've already visited.
    // If we're on a bus, don't allow a transfer if we've been on for
    // less than 5 minutes (FIXME: probably better to measure distance
    // travelled?)
    if (last_route_id == -1 || path->route_time > (2 * 60))
    {
        for (TripStop::WalkHopList::iterator i = src_stop->wlist.begin();
             i != src_stop->wlist.end(); i++)
        {
            int32_t dest_id = i->dest_id;
            double walktime = i->walktime;

            // Do a quick test to make sure that the potential basis for a 
            // new path isn't worse than what we have already, before
            // incurring the cost of creating a new path and evaluating it.
            unordered_map<int32_t, shared_ptr<TripPath> > vsrc = visited_walks[src_id];
            unordered_map<int32_t, shared_ptr<TripPath> >::iterator v1 = vsrc.find(dest_id);
            if (v1 != vsrc.end() && path->heuristic_weight > v1->second->heuristic_weight)
                continue;
                
            shared_ptr<TripAction> action(
                 new TripAction(src_id, dest_id, -1, path->time, 
                               (path->time + walktime)));
            shared_ptr<TripStop> ds = _get_tripstop(dest_id);
            shared_ptr<TripPath> path2 = path->add_action(
                action, outgoing_route_ids, ds);

            DEBUGPATH("- Considering walkpath to %d\n", dest_id);

            if (v1 == vsrc.end() || 
                v1->second->heuristic_weight > path2->heuristic_weight ||
                ((v1->second->heuristic_weight - path2->heuristic_weight) < 1.0f &&
                 v1->second->walking_time > path2->walking_time))
            {
                DEBUGPATH("-- Adding walkpath to %d (walktime: %f (%f, %f))\n", dest_id, walktime, action->start_time, action->end_time);
                if (dest_id == goal_id)
                    completed_paths.push(path2);
                else
                    uncompleted_paths.push(path2);

                num_paths_considered++;
                visited_walks[src_id][dest_id] = path2;
            }
        }
    }

    
    // If we're doing a walkonly path (mostly for generating shapes?), stop
    // and return here.
    if (walkonly)
        return;

    // Find outgoing triphops from the source and get a list of paths to them. 
    for (vector<pair<int, int> >::iterator sp = vsp.begin(); sp != vsp.end();
         sp++)
    {
        deque<int> route_ids = src_stop->get_routes(sp->first);
        for (deque<int>::iterator j = route_ids.begin(); j != route_ids.end(); j++)
        {
            int LEEWAY = 0;
            if ((*j) != last_route_id)
                LEEWAY = (5*60); // give 5 mins to make a transfer

            const TripHop * t = src_stop->find_triphop(
                elapsed_daysecs + sp->second + LEEWAY, (*j), sp->first);
            if (t)
            {
                // If we've been on the route before (or could have been), 
                // don't get on again.
                if ((*j) != last_route_id && path->possible_route_ids.count(*j))
                {
                    // pass
                }
                // Disallow more than three transfers.
                else if ((*j) != last_route_id && 
                         path->traversed_route_ids > 3)
                {
                    // pass
                }
                else
                {
                    // Do a quick test to make sure that the potential basis for a 
                    // new path isn't worse than what we have already, before
                    // incurring the cost of creating a new path and evaluating it.
                    unordered_map<int, shared_ptr<TripPath> >::iterator v = visited_routes[src_id].find(*j);
                    if (v != visited_routes[src_id].end() && path->heuristic_weight > v->second->heuristic_weight)
                        continue;

                    shared_ptr<TripAction> action = shared_ptr<TripAction>(
                        new TripAction(src_id, t->dest_id, (*j), daystart + t->start_time,
                                       daystart + t->end_time));
                    shared_ptr<TripStop> ds = _get_tripstop(t->dest_id);
                    shared_ptr<TripPath> path2 = path->add_action(
                        action, outgoing_route_ids, ds);
                

                    if (v == visited_routes[src_id].end() || 
                        v->second->heuristic_weight > path2->heuristic_weight ||
                        ((v->second->heuristic_weight - path2->heuristic_weight) < 1.0f &&
                         v->second->walking_time > path2->walking_time))
                    {
                        if (t->dest_id == goal_id)
                            completed_paths.push(path2);
                        else
                            uncompleted_paths.push(path2);

                        num_paths_considered++;
                        visited_routes[src_id][(*j)] = path2;
                    }
                }
            }
        }
    }
}    


vector<TripStop> TripGraph::find_tripstops_in_range(double lat, double lng, 
                                                    TripStop::Type type,
                                                    double range)
{
    vector<TripStop> tripstops_in_range;

    for (TripStopList::iterator i = tripstops.begin(); 
         i != tripstops.end(); i++)
    {
        if ((*i)->type != type)
            continue;

        double dist = distance((*i)->lat, (*i)->lng, lat, lng);
        if (dist <= range)
            tripstops_in_range.push_back(*(*i));
    }

    return tripstops_in_range;
}
