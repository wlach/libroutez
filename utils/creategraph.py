#!/usr/bin/python

import transitfeed
import libroutez.osm as osm
import sys
from libroutez.tripgraph import *

class IdMap:
    '''class which maps from gtfs ids -> libroutez ids'''
    def __init__(self):
        self.stopmap = {}
        self.routemap = {}
        self.tripmap = {}

    def save(self, fname):
        f = open(fname, 'w')
        
        print >> f, "Stops: {"
        for gtfs_stop_id in sorted(self.stopmap.keys()):
            print >>f, "    '%s': %s," % (gtfs_stop_id, self.stopmap[gtfs_stop_id])
        print >> f, "}"

        print >> f, "Routes: {"
        for gtfs_route_id in sorted(self.routemap.keys()):
            print >>f, "    '%s': %s," % (gtfs_route_id, 
                                      self.routemap[gtfs_route_id])
        print >> f, "}"

        print >> f, "Trips: {"
        for gtfs_trip_id in sorted(self.tripmap.keys()):
            print >> f, "    '%s': %s," % (gtfs_trip_id, self.tripmap[gtfs_trip_id])
        print >> f, "}"

        f.close()

def load_gtfs(tripgraph, sched, idmap):
    stops = sched.GetStopList()
    for stop in stops:
        idmap.stopmap[stop.stop_id] = len(idmap.stopmap)
        tripgraph.add_tripstop(idmap.stopmap[stop.stop_id], "gtfs", 
                               stop.stop_lat, stop.stop_lon)
      
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
          if not idmap.tripmap.get(trip.trip_id):
              idmap.tripmap[trip.trip_id] = len(idmap.tripmap)
          if not idmap.routemap.get(trip.route_id):
              idmap.routemap[trip.route_id] = len(idmap.routemap)
          tripgraph.add_triphop(prevsecs, secs, idmap.stopmap[prevstop.stop_id],
                                idmap.stopmap[stop.stop_id], 
                                idmap.routemap[trip.route_id], 
                                idmap.tripmap[trip.trip_id], 
                                str(trip.service_id))
        prevstop = stop
        prevsecs = secs

def load_osm(tripgraph, map, idmap):
    # map of osm ids -> libroutez ids. libroutez ids are positive integers, 
    # starting from the last gtfs id. I'm assuming that OSM ids can be pretty 
    # much anything
    for node in map.nodes.values():
        idmap.stopmap[node.id] = len(idmap.stopmap)
        tripgraph.add_tripstop(idmap.stopmap[node.id], "osm", node.lat, 
                               node.lon)
        
    for way in map.ways.values():
        prev_id = None
        for id in way.nds:
            if prev_id:
                tripgraph.add_walkhop(idmap.stopmap[prev_id], idmap.stopmap[id])
                tripgraph.add_walkhop(idmap.stopmap[id], idmap.stopmap[prev_id])
            prev_id = id

if __name__ == '__main__':
    if len(sys.argv) < 5:
        print "Usage: %s: <gtfs feed> <osm file> <graph> <gtfs mapping>" \
            % sys.argv[0]
        exit(1)

    g = TripGraph()
    idmap = IdMap()
    print "Loading OSM."
    map = osm.OSM(sys.argv[2])
    print "Loading schedule."
    schedule = transitfeed.Schedule(
        problem_reporter=transitfeed.ProblemReporter())
    schedule.Load(sys.argv[1])
    print "Inserting gtfs into graph"
    load_gtfs(g, schedule, idmap)
    print "Inserting osm into graph"
    load_osm(g, map, idmap)
    print "Saving idmap"
    idmap.save(sys.argv[4])
    print "Linking osm with gtfs"
    g.link_osm_gtfs()
    print "Saving graph"
    g.save(sys.argv[3])
