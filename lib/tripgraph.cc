#include "tripgraph.h"
#include <assert.h>
#include <errno.h>
#include <map>
#include <math.h>

using namespace std;
using namespace boost;

// set to 1 to see what find_path is doing (VERY verbose)
#if 0
# define DEBUGPATH(fmt, args...) fprintf(stderr, fmt, ## args)
#else
# define DEBUGPATH
#endif

// Estimated walking speed in m/s
static const float est_walk_speed = 1.1f;

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
    if (src_lat == dest_lat && src_lng == dest_lng)
        return 0.0f;

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
        
    uint32_t num_tripstops = 0;
    if (fread(&num_tripstops, sizeof(uint32_t), 1, fp) != 1)
    {
        printf("Error: Couldn't read the number of tripstops.\n");
        return;
    }
        
    uint32_t i = 0;
    while (i < num_tripstops)
    {
        shared_ptr<TripStop> s(new TripStop(fp));
        tripstops.insert(pair<int32_t,shared_ptr<TripStop> >(s->id, s));
        i++;
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

    // write triphops
    uint32_t num_tripstops = tripstops.size();
    assert(fwrite(&num_tripstops, sizeof(uint32_t), 1, fp) == 1);
    for (TripStopDict::iterator i = tripstops.begin();
         i != tripstops.end(); i++)
    {
        i->second->write(fp);
    }

    fclose(fp);
}


void TripGraph::add_triphop(int32_t start_time, int32_t end_time, 
                            int32_t src_id, int32_t dest_id, int32_t route_id, 
                            int32_t trip_id, string service_id)
{
    // will assert if src_id doesn't exist!!
    _get_tripstop(src_id)->add_triphop(start_time, end_time, dest_id, route_id, 
                                       trip_id, service_id);
}


void TripGraph::add_tripstop(int32_t id, string type, float lat, float lng)
{
    shared_ptr<TripStop> s(new TripStop(id, type, lat, lng));
    tripstops.insert(pair<int32_t,shared_ptr<TripStop> >(id, s));
}


void TripGraph::add_walkhop(int32_t src_id, int32_t dest_id)
{
    // will assert if src_id or dest_id doesn't exist!!
    shared_ptr<TripStop> ts_src = _get_tripstop(src_id);
    shared_ptr<TripStop> ts_dest = _get_tripstop(dest_id);

    double dist = distance(ts_src->lat, ts_src->lng,
                           ts_dest->lat, ts_dest->lng);

    ts_src->add_walkhop(dest_id, dist / est_walk_speed);
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

    int tripstop_count = 0;
    int tripstop_total = tripstops.size();
    for (TripStopDict::iterator i = tripstops.begin(); 
         i != tripstops.end(); i++)
    {
        tripstop_count++;
        // For each GTFS stop...
        if (strcmp(i->second->type, "gtfs") == 0)
        {
            Point gtfs_pt(i->second->lat, i->second->lng);
            
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
            for (TripStopDict::iterator j = tripstops.begin(); 
                 j != tripstops.end(); j++)
            {
                for (TripStop::WalkHopList::iterator k = j->second->wlist.begin(); 
                     k != j->second->wlist.end(); k++)
                {
                    Point trip_pt(j->second->lat, j->second->lng);

                    shared_ptr<TripStop> dest_stop = _get_tripstop(k->dest_id);
                    Point walk_pt(dest_stop->lat, dest_stop->lng);

                    Point p = get_closest_point(trip_pt, walk_pt, gtfs_pt);

                    // Find the closest OSM hop to the GTFS stop
                    double dist = distance(gtfs_pt.lat, gtfs_pt.lng, 
                                           p.lat, p.lng);
                    if ((nearest_walkhop.first == (-1) && 
                         nearest_walkhop.second == (-1)) || dist < min_dist)
                    {
                        nearest_walkhop = pair<int32_t,int32_t>();
                        // If the GTFS stop is on one of the OSM nodes, use
                        // that node.  Otherwise remember both nodes.
                        if (trip_pt == p)
                            nearest_walkhop.first = j->first;
                        else if (walk_pt == p)
                            nearest_walkhop.first = k->dest_id;
                        else
                        {
                            nearest_walkhop.first = j->first;
                            nearest_walkhop.second = k->dest_id;
                        }

                        min_dist = dist;
                    }
                }
            }
            
            new_walkhops[i->first] = nearest_walkhop;
            printf("%02.2f%% done: Linking %d -> %d, %d\n", 
                    ((float)tripstop_count * 100.0f) / ((float)tripstop_total),
                    i->first, 
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
    for (TripStopDict::iterator i = tripstops.begin(); 
         i != tripstops.end(); i++)
    {
        shared_ptr<TripStop> s = i->second;
        double dist = pow((s->lat - lat), 2) + pow((s->lng - lng), 2);
        if (!closest_stop || dist < min_dist)
        {
            closest_stop = s;
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


TripPath TripGraph::find_path(int secs, string service_period, bool walkonly,
                              double src_lat, double src_lng, 
                              double dest_lat, double dest_lng)
{
    PathQueue uncompleted_paths;
    PathQueue completed_paths;
        
    VisitedRouteMap visited_routes;
    VisitedWalkMap visited_walks;

    shared_ptr<TripStop> start_node = get_nearest_stop(src_lat, src_lng);
    shared_ptr<TripStop> end_node = get_nearest_stop(dest_lat, dest_lng);
    DEBUGPATH("Start: %s End: %s\n", start_node->id, end_node->id);

    // Consider the distance required to reach the start node from the 
    // beginning, and add that to our start time.
    double dist_from_start = distance(src_lat, src_lng, 
                                      start_node->lat, start_node->lng);
    secs += (int)(dist_from_start / est_walk_speed);
    
    DEBUGPATH("Start time - %d\n", secs);
    shared_ptr<TripPath> start_path(new TripPath(secs, est_walk_speed, 
                                                 end_node, start_node));
    if (start_node == end_node)
        return TripPath(*start_path);

    uncompleted_paths.push(start_path);

    TripPath best_completed_path;

    for (int i=0; i<3; i++)
    {
        int num_paths_considered = 0;

        while (uncompleted_paths.size() > 0)
        {
            shared_ptr<TripPath> path = uncompleted_paths.top();
            uncompleted_paths.pop();
            extend_path(path, service_period, walkonly, end_node->id, 
                        num_paths_considered, visited_routes, visited_walks, 
                        uncompleted_paths, completed_paths);

            // If we've still got open paths, but their weight exceeds that
            // of the weight of a completed path, break.
            if (uncompleted_paths.size() > 0 && completed_paths.size() > 0 &&
                uncompleted_paths.top()->heuristic_weight > 
                completed_paths.top()->heuristic_weight)
            {
                DEBUGPATH("Breaking with %d uncompleted paths (paths "
                          "considered: %d).\n", uncompleted_paths.size(), 
                          num_paths_considered);
                return TripPath(*(completed_paths.top()));
            }

            //if len(completed_paths) > 0 and len(uncompleted_paths) > 0:
            //  print "Weight of best completed path: %s, uncompleted: %s" % \
            //      (completed_paths[0].heuristic_weight, uncompleted_paths[0].heuristic_weight)
        }

        if (completed_paths.size())
            best_completed_path = TripPath(*(completed_paths.top()));
    }

    return best_completed_path;

    return TripPath();
}


shared_ptr<TripStop> TripGraph::_get_tripstop(int32_t id)
{
    TripStopDict::iterator ts = tripstops.find(id);
    assert(ts != tripstops.end());

    return ts->second;
}


void TripGraph::extend_path(shared_ptr<TripPath> &path, 
                            string &service_period, 
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
    
    DEBUGPATH("Extending path at vertex %s (on %d) @ %f (walktime: %f, "
              "routetime:%f)\n", src_id, last_route_id, path->time, 
              path->walking_time, path->route_time);
    shared_ptr<TripStop> src_stop = _get_tripstop(src_id);

    // Keep track of outgoing route ids at this node: make sure that we 
    // don't get on a route later when we could have gotten on here.
    unordered_set<int> outgoing_route_ids = src_stop->get_routes(service_period);

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

            DEBUGPATH("- Considering walkpath to %s\n", dest_id);

            if (v1 == vsrc.end() || 
                v1->second->heuristic_weight > path2->heuristic_weight ||
                ((v1->second->heuristic_weight - path2->heuristic_weight) < 1.0f &&
                 v1->second->walking_time > path2->walking_time))
            {
                DEBUGPATH("-- Adding walkpath to %s\n", dest_id);
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
    for (unordered_set<int>::iterator i = outgoing_route_ids.begin();
         i != outgoing_route_ids.end(); i++)
    {
        int LEEWAY = 0;
        if ((*i) != last_route_id)
            LEEWAY = (5*60); // give 5 mins to make a transfer

        shared_ptr<TripHop> t = src_stop->find_triphop((int)path->time + LEEWAY, 
                                                       (*i), 
                                                       service_period);
        if (t)
        {
            // If we've been on the route before (or could have been), 
            // don't get on again.
            if ((*i) != last_route_id && path->possible_route_ids.count(*i))
            {
                // pass
            }
            // Disallow more than three transfers.
            else if ((*i) != last_route_id && 
                     path->traversed_route_ids > 3)
            {
                // pass
            }
            else
            {
                // Do a quick test to make sure that the potential basis for a 
                // new path isn't worse than what we have already, before
                // incurring the cost of creating a new path and evaluating it.
                unordered_map<int, shared_ptr<TripPath> >::iterator v = visited_routes[src_id].find(*i);
                if (v != visited_routes[src_id].end() && path->heuristic_weight > v->second->heuristic_weight)
                    continue;

                shared_ptr<TripAction> action = shared_ptr<TripAction>(
                    new TripAction(src_id, t->dest_id, (*i), t->start_time,
                                   t->end_time));
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
                    visited_routes[src_id][(*i)] = path2;
                }
            }
        }
    }
}    
