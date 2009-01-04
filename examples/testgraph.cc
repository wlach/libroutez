#include "tripgraph.h"

int main(int argc, char *argv[])
{
    TripGraph g;
    g.load("mygraph.routez");

    TripPath p = g.find_path(46167, "weekday", false, 44.657304, -63.591096, 
                44.7321379, -63.657304);
    boost::shared_ptr<TripAction> a(p.last_action);
    while (a)
    {
        printf("%s->%s; route: %d; start time: %.2f; end time: %.2f\n", 
            a->src_id.c_str(), a->dest_id.c_str(), a->route_id, 
            a->start_time, a->end_time);
        a = a->parent;
    }
}
