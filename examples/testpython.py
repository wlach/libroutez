#!/usr/bin/python

# This example is just to give a quick example of using the API for python
# hackers. 

# FIXME: flesh this out a bit more

from libroutez.tripgraph import *


if __name__ == '__main__':
    g = TripGraph()

    g.add_tripstop(0, TripStop.GTFS, 0.0, 0.0)
    g.add_tripstop(1, TripStop.GTFS, 0.5, 0.0)
    s = ServicePeriod(0, 1, 0, 0, 7, 0, 100, 2000, True, True, True)
    g.add_service_period(s);
    g.add_triphop(500, 1000, 0, 1, 1, 1, 0)
    g.add_walkhop(0, 1)

    path = g.find_path(0, False, 0.0, 0.0, 0.5, 0.0)

    for action in path.get_actions():
        print "src: %s dest: %s st: %s et: %s rid: %s" % \
        (action.src_id, action.dest_id, action.start_time, action.end_time,
         action.route_id)

