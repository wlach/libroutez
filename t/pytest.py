#!/usr/bin/python

# This is here mostly just to test that the python bindings actually work

from libroutez.tripgraph import *
import time
from wvtest import *

last=None

@wvtest
def basic_find_path():
    graph = TripGraph()
    graph.add_tripstop(0, TripStop.OSM, 0.0, 0.0)
    graph.add_tripstop(1, TripStop.OSM, 1.0, 0.0)

    # no path available
    p = graph.find_path(0, True, 0.0, 0.0, 1.0, 0.0)
    WVPASSEQ(p, None)

    # walking only
    graph.add_walkhop(0, 1)
    p = graph.find_path(0, True, 0.0, 0.0, 1.0, 0.0)
    actions = p.get_actions()
    WVPASSEQ(len(actions), 1)


@wvtest 
def get_service_period_offsets():
    graph = TripGraph()
    graph.add_service_period(ServicePeriod(0, 1, 0, 108, 7, 0, 108, 2000,
                                           False, True, False))
    t = time.mktime((2008, 1, 5, 0, 0, 0, 0, 0, -1))
    splist = graph.get_service_period_ids_for_time(int(t))
    WVPASSEQ(splist[0][0], 0)
    WVPASSEQ(splist[0][1], 0)


@wvtest
def tripstop():
    ts = TripStop(0, TripStop.OSM, 0.0, 0.0);
    ts.add_triphop(500, 1000, 1, 1, 1, 0, 0);
    route_ids = ts.get_routes(0)
    WVPASSEQ(len(route_ids), 1)
    WVPASSEQ(route_ids[0], 1)
