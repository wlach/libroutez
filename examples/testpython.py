#!/usr/bin/python

# This example is just to give a quick example of using the API for python
# hackers. 

# FIXME: flesh this out a bit more

import routez.tripgraph

if __name__ == '__main__':
    g = routez.tripgraph.TripGraph()

    g.add_tripstop("gtfs1", "gtfs", 0.0, 0.0)
    g.add_tripstop("gtfs2", "gtfs", 0.5, 0.0)
    g.add_triphop(500, 1, "gtfs1", "gtfs2", 1, "caturday")
    g.add_walkhop("gtfs1", "gtfs2")

    path = g.find_path(0, "caturday", False, 0.0, 0.0, 0.5, 0.0)

    for action in path.get_actions():
        print "src: %s dest: %s st: %s et: %s rid: %s" % \
        (action.src_id, action.dest_id, action.start_time, action.end_time,
         action.route_id)

