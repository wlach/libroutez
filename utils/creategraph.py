#!/usr/bin/python

import transitfeed
import libroutez.osm as osm
import sys
from libroutez.tripgraph import *

def load_gtfs(tripgraph, sched):
    stops = sched.GetStopList()
    for stop in stops:
      tripgraph.add_tripstop(int(stop.stop_id), "gtfs", stop.stop_lat, 
                        stop.stop_lon)
      
    trips = sched.GetTripList()
    for trip in trips:
      interpolated_stops = trip.GetTimeInterpolatedStops()
      prevstop = None
      prevsecs = 0
      for (secs, stoptime, is_timepoint) in interpolated_stops:
        stop = stoptime.stop
        if prevstop:
          # stupid side-effect of google's transit feed python script being broken
          if int(secs) < int(prevsecs):
            print "WARNING: Negative edge in gtfs. This probably means you "
            "need a more recent version of the google transit feed " 
            "package (see README)"
          tripgraph.add_triphop(prevsecs, secs, int(prevstop.stop_id),
                                int(stop.stop_id), int(trip.route_id), 
                                int(trip.trip_id), str(trip.service_id))        
        prevstop = stop
        prevsecs = secs

def load_osm(tripgraph, map, startid):
    # map of osm ids -> libroutez ids. libroutez ids are positive integers, 
    # starting from the last gtfs id. I'm assuming that OSM ids can be pretty 
    # much anything
    nodemap = {}
    curid = startid

    for node in map.nodes.values():
        nodemap[node.id] = curid
        tripgraph.add_tripstop(curid, "osm", node.lat, node.lon)
        curid += 1
        
    for way in map.ways.values():
        prev_id = None
        for id in way.nds:
            if prev_id:
                tripgraph.add_walkhop(nodemap[prev_id], nodemap[id])
                tripgraph.add_walkhop(nodemap[id], nodemap[prev_id])
            prev_id = id

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Usage: %s: <gtfs feed> <osm file> <graph>" % sys.argv[0]
        exit(1)

    g = TripGraph()
    schedule = transitfeed.Schedule(
        problem_reporter=transitfeed.ProblemReporter())
    print "Loading schedule."
    schedule.Load(sys.argv[1])
    print "Loading OSM."
    map = osm.OSM(sys.argv[2])
    print "Inserting gtfs into graph"
    load_gtfs(g, schedule)
    print "Inserting osm into graph"
    load_osm(g, map, len(schedule.GetStopList()))
    print "Linking osm with gtfs"
    g.link_osm_gtfs()
    print "Saving graph"
    g.save(sys.argv[3])
