#!/usr/bin/python

import transitfeed
import libroutez.osm as osm
import time
import sys
from libroutez.tripgraph import *
from optparse import OptionParser

class IdMap:
    '''class which maps from gtfs ids -> libroutez ids'''
    def __init__(self):
        self.spmap = {}
        self.stopmap = {}
        self.routemap = {}
        self.tripmap = {}

    def save(self, fname):
        f = open(fname, 'w')

        print >> f, "Service Periods: {"
        for gtfs_sp_id in sorted(self.spmap.keys()):
            print >>f, "    '%s': %s," % (gtfs_sp_id, self.spmap[gtfs_sp_id])
        print >> f, "}"
        
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
    print "Setting timezone to %s" % sched.GetDefaultAgency().agency_timezone
    tripgraph.set_timezone(str(sched.GetDefaultAgency().agency_timezone))

    stops = sched.GetStopList()
    for stop in stops:
        idmap.stopmap[stop.stop_id] = len(idmap.stopmap)
        tripgraph.add_tripstop(idmap.stopmap[stop.stop_id], TripStop.GTFS, 
                               stop.stop_lat, stop.stop_lon)

    for sp_id in sched.service_periods.keys():
        idmap.spmap[sp_id] = len(idmap.spmap)

    service_period_bounds = {}
      
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
          if not idmap.tripmap.has_key(trip.trip_id):
              idmap.tripmap[trip.trip_id] = len(idmap.tripmap)
          if not idmap.routemap.has_key(trip.route_id):
              idmap.routemap[trip.route_id] = len(idmap.routemap)

          if not service_period_bounds.has_key(trip.service_id):
              service_period_bounds[trip.service_id] = prevsecs
          elif prevsecs > service_period_bounds[trip.service_id]:
              service_period_bounds[trip.service_id] = prevsecs

          if prevstop.stop_id != stop.stop_id:
              # only add triphop if we're not going to ourselves. there are
              # some feeds (cough, cough, Halifax) which actually do this
              tripgraph.add_triphop(prevsecs, secs, idmap.stopmap[prevstop.stop_id],
                                    idmap.stopmap[stop.stop_id], 
                                    idmap.routemap[trip.route_id], 
                                    idmap.tripmap[trip.trip_id], 
                                    idmap.spmap[trip.service_id])
        prevstop = stop
        prevsecs = secs

    for sp_id in sched.service_periods.keys():
        sp = sched.service_periods[sp_id]
        if not sp.start_date or not sp.end_date:
            continue
        tm_start = time.strptime(sp.start_date, "%Y%m%d")
        tm_end = time.strptime(sp.end_date, "%Y%m%d")
        # FIXME: currently assume weekday service is uniform, i.e. 
        # monday service == mon-fri service
        if service_period_bounds.has_key(sp_id):
            s = ServicePeriod(idmap.spmap[sp_id],
                              tm_start.tm_mday, tm_start.tm_mon - 1, 
                              (tm_start.tm_year - 1900),
                              tm_end.tm_mday, tm_end.tm_mon - 1, 
                              (tm_end.tm_year - 1900),
                              int(service_period_bounds[sp_id]),
                              sp.day_of_week[0], sp.day_of_week[5], 
                              sp.day_of_week[6])
            for ex in sp.date_exceptions.keys():
                tm_ex = time.strptime(ex, "%Y%m%d")
                if sp.date_exceptions[ex] == 1:
                    s.add_exception_on(tm_ex.tm_mday, tm_ex.tm_mon - 1,
                                       tm_ex.tm_year - 1900)
                else:
                    s.add_exception_off(tm_ex.tm_mday, tm_ex.tm_mon - 1,
                                        tm_ex.tm_year - 1900)

            tripgraph.add_service_period(s)
        else:
            print "WARNING: It appears as if we have a service period with no "
            "bound. This implies that it's not actually being used for anything."


def load_osm(tripgraph, map, idmap):
    # map of osm ids -> libroutez ids. libroutez ids are positive integers, 
    # starting from the last gtfs id. I'm assuming that OSM ids can be pretty 
    # much anything
    osm_nodemap = {}
    for node in map.nodes.values():
        osm_nodemap[node.id] = len(osm_nodemap) + len(idmap.stopmap)
        tripgraph.add_tripstop(osm_nodemap[node.id], TripStop.OSM,
                               node.lat, node.lon)
        
    for way in map.ways.values():
        previd = None
        for id in way.nds:
            if previd:
                tripgraph.add_walkhop(osm_nodemap[previd], osm_nodemap[id])
                tripgraph.add_walkhop(osm_nodemap[id], osm_nodemap[previd])
            previd = id

if __name__ == '__main__':

    usage = "usage: %prog [options] <gtfs feed> <graph> <gtfs mapping>"
    parser = OptionParser(usage)
    parser.add_option('--osm', dest='osm',
                      help='Path of OSM file (optional)')

    (options, args) = parser.parse_args()

    if len(args) < 3:
        parser.error("incorrect number of arguments")
        exit(1)

    print "Loading schedule."
    schedule = transitfeed.Schedule(
        problem_reporter=transitfeed.ProblemReporter())
    schedule.Load(args[0])
    print "Creating graph"
    g = TripGraph()
    print "Inserting gtfs into graph"
    idmap = IdMap()
    load_gtfs(g, schedule, idmap)

    if options.osm:
        print "Loading OSM."
        map = osm.OSM(options.osm)
        print "Inserting osm into graph"
        load_osm(g, map, idmap)
        print "Linking osm with gtfs"
        g.link_osm_gtfs()

    print "Saving idmap"
    idmap.save(args[2])

    print "Saving graph"
    g.save(args[1])
