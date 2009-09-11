#include <stdlib.h>
#include "tripgraph.h"

using namespace std;
using namespace boost;


void print_actions(shared_ptr<TripAction> &action)
{
    shared_ptr<TripAction> parent(action->parent);
    if (parent)
        print_actions(parent);

    printf("%d->%d; route: %d; start time: %.2f; end time: %.2f\n", 
           action->src_id, action->dest_id, action->route_id, 
           action->start_time, action->end_time);
}


int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        printf("Usage: %s <graph file> <src lat> <src lng> <dest lat> "
               "<dest lng> <start time>\n", argv[0]);
        return 1;
    }

    float src_lat = atof(argv[2]);
    float src_lng = atof(argv[3]);
    float dest_lat = atof(argv[4]);
    float dest_lng = atof(argv[5]);
    int start_time = atoi(argv[6]);

    printf("Loading graph...\n");
    TripGraph g;
    g.load(argv[1]);

    printf("Calculating path...\n");
    TripPath *p = g.find_path(start_time, false, src_lat, src_lng, 
                              dest_lat, dest_lng);

    if (p)
        print_actions(p->last_action);
    else
        printf("Couldn't find path.\n");

    delete p;

    return 0;
}
