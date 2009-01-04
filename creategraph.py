#!/usr/bin/python

import transitfeed
import osm
import sys
from tripgraph import *

def load_gtfs(tripgraph, sched):
    stops = sched.GetStopList()
    for stop in stops:
      tripgraph.add_tripstop(str("gtfs" + stop.stop_id), "gtfs", stop.stop_lat, 
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
            "need to a more recent version of the google transit feed "
            "package (see README)"
                                                                                          secs, prevsecs, prevstop.stop_id, stop.stop_id)
          #print "Adding triphop %s %s %s->%s" % (trip.trip_id, trip.trip_headsign, prevstop.stop_id, stop.stop_id)
          tripgraph.add_triphop(prevsecs, secs, str("gtfs"+prevstop.stop_id),
                                str("gtfs"+stop.stop_id), int(trip.route_id), 
                                str(trip.service_id))        
        prevstop = stop
        prevsecs = secs

def load_osm(tripgraph, map):
    for node in map.nodes.values():
        tripgraph.add_tripstop(str("osm"+node.id), str("osm"), node.lat, node.lon)
        
    for way in map.ways.values():
        prev_id = None
        for id in way.nds:
            if prev_id:
                tripgraph.add_walkhop(str("osm"+prev_id), str("osm"+id))
                tripgraph.add_walkhop(str("osm"+id), str("osm"+prev_id))
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
    load_osm(g, map)
    print "Linking osm with gtfs"
    g.link_osm_gtfs()
    print "Saving graph"
    g.save(sys.argv[3])
