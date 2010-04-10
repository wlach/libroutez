#!/usr/bin/python

import transitfeed
from rtree import Rtree
import libroutez.osm as osm
import math
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

def process_gtfs(tripgraph, sched, idmap):
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

              # get a stop_headsign id, if we have a stop headsign (rare, but not
              # unheard of)
              headsign_id = -1
              stop_headsign = stoptime.stop_headsign
              if stop_headsign and len(stop_headsign) > 0:
                  if not idmap.headsignmap.has_key(stop_headsign):
                      idmap.headsignmap[stop_headsign] = len(idmap.headsignmap)
                  headsign_id = idmap.headsignmap[stop_headsign]

              tripgraph.add_triphop(prevsecs, secs,
                                    idmap.stopmap[prevstop.stop_id],
                                    idmap.stopmap[stop.stop_id],
                                    idmap.routemap[trip.route_id],
                                    idmap.tripmap[trip.trip_id],
                                    idmap.spmap[trip.service_id],
                                    headsign_id)
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


def process_osm(tripgraph, osm_map, sched, idmap):

    # first, we create a new map with the appropriate nodes to be able
    # to link with the transit graph
    print "Rewriting map to link with GTFS..."

    link_map = {}

    class WayIndex:
        def __init__(self, ways):
            self.max_way_id = 0
            self.way_idx_mapping = {}
            self.idx = Rtree()
            for way in ways:
                self.add(way)

        def add(self, way):
            self.idx.add(id=self.max_way_id, coordinates=way.get_bbox(), 
                         obj=way.id)
            self.way_idx_mapping[way.id] = self.max_way_id
            self.max_way_id += 1

        def delete(self, way):
            self.idx.delete(id=self.way_idx_mapping[way.id],
                            coordinates=way.get_bbox())
            way_ids = map(lambda way: way.object, 
                          wayidx.idx.intersection(way.get_bbox(), 
                                                  objects=True))

    wayidx = WayIndex(osm_map.ways.values())
    
    MIN_DISTANCE=0.00075
    
    for stop in sched.GetStopList():
        way_ids = map(lambda way: way.object, 
                      wayidx.idx.intersection((stop.stop_lon-MIN_DISTANCE, 
                                        stop.stop_lat-MIN_DISTANCE, 
                                        stop.stop_lon+MIN_DISTANCE, 
                                        stop.stop_lat+MIN_DISTANCE), 
                                       objects=True))
    
        # fall back to doing an exhaustive search if we can't find a way in 
        # range
        if len(way_ids) == 0:        
            way_ids = osm_map.ways.keys()
    
        class ClosestWay:
            def __init__(self):
                self.way = None # way
                self.subway_nds = None # subway indices, scalar magnitude along subway
                self.dist = None
                self.intersection_pt = None
                self.intersection_scalar = None
                
        closest_way = ClosestWay()
    
        def latlng_dist(src_lat, src_lng, dest_lat, dest_lng):
            theta = src_lng - dest_lng
            src_lat_radians = math.radians(src_lat)
            dest_lat_radians = math.radians(dest_lat)
            dist = math.sin(src_lat_radians) * math.sin(dest_lat_radians) + \
                math.cos(src_lat_radians) * math.cos(dest_lat_radians) * \
                math.cos(math.radians(theta))
            dist = math.degrees(math.acos(dist)) * (60.0 * 1.1515 * 1.609344 * 1000.0)
            return dist
    
        for way_id in way_ids:
            way = osm_map.ways[way_id]
            prev_id = None
            for id in way.nds:
                if prev_id:
                    prev_node = osm_map.nodes[prev_id]
                    node = osm_map.nodes[id]
    
                    # slope of the sub-way
                    slope1_lon = node.lon - prev_node.lon
                    slope1_lat = node.lat - prev_node.lat
                    
                    # slope of the vector from the stop to the sub way
                    slope2_lon = slope1_lat * (-1)
                    slope2_lat = slope1_lon 
    
                    # the scalar value of the intersection on the way: 
                    # - < 0 = before the first pt,
                    # - > 0 = after the last pt 
                    # - between = between the two points
                    # FIXME: use an epsilon, not zero
                    intersection_scalar = (((prev_node.lon - stop.stop_lon) * slope2_lat + \
                                                (stop.stop_lat - prev_node.lat) * slope2_lon) / \
                                               (slope1_lat*slope2_lon - slope1_lon*slope2_lat))
                    way_dist = None
                    intersection_pt = None
                    EPSILON=0.0005
                    if intersection_scalar <= EPSILON:
                        way_dist = latlng_dist(prev_node.lat, prev_node.lon, stop.stop_lat, stop.stop_lon)
                    elif intersection_scalar >= (1.0 - EPSILON):
                        way_dist = latlng_dist(node.lat, node.lon, stop.stop_lat, stop.stop_lon)
                    else:
                        intersection_pt = (prev_node.lat+(intersection_scalar*slope1_lat), 
                                           prev_node.lon+(intersection_scalar*slope1_lon))
                        way_dist = latlng_dist(intersection_pt[0], intersection_pt[1], stop.stop_lat, stop.stop_lon)
                   
                    if not closest_way.way or way_dist < closest_way.dist:
                        closest_way.way = way
                        closest_way.dist = way_dist
                        closest_way.subway_nds = (prev_id, id)
                        closest_way.intersection_scalar = intersection_scalar
                        closest_way.intersection_pt = intersection_pt
                prev_id = id

        # ok, now that we have a closest way, act accordingly...
        if closest_way.intersection_scalar > EPSILON and \
                closest_way.intersection_scalar < 1.0 - EPSILON:
            # append closest_intersection_pt to nds
            indice = closest_way.way.nds.index(closest_way.subway_nds[1])
            new_node_id = "newnode-" + str(len(osm_map.nodes.keys()))
            osm_map.nodes[new_node_id] = osm.Node(new_node_id, 
                                                  closest_way.intersection_pt[1], 
                                                  closest_way.intersection_pt[0])

            # split way in two
            new_way_id1 = closest_way.way.id + "-1"
            new_way1 = osm.Way(new_way_id1, osm_map)
            new_way1.nds = closest_way.way.nds[0:indice]
            new_way1.nds.append(new_node_id)
    
            new_way_id2 = closest_way.way.id + "-2"
            new_way2 = osm.Way(new_way_id2, osm_map)
            new_way2.nds = closest_way.way.nds[indice:]
            new_way2.nds.insert(0, new_node_id)
    
            wayidx.delete(closest_way.way)
            del osm_map.ways[closest_way.way.id]
    
            osm_map.ways[new_way_id1] = new_way1
            wayidx.add(new_way1)
            osm_map.ways[new_way_id2] = new_way2
            wayidx.add(new_way2)

            link_map[stop.stop_id] = new_node_id
    
        elif closest_way.intersection_scalar <= EPSILON:        
            link_map[stop.stop_id] = closest_way.subway_nds[0]
    
        else: # closest_way.intersection_scale >= EPSILON
            link_map[stop.stop_id] = closest_way.subway_nds[-1]
    
    # map of osm ids -> libroutez ids. libroutez ids are positive integers, 
    # starting from the last gtfs id. I'm assuming that OSM ids can be pretty 
    # much anything
    print "Adding OSM to graph..."
    osm_nodemap = {}
    for node in osm_map.nodes.values():
        osm_nodemap[node.id] = len(osm_nodemap) + len(idmap.stopmap)
        tripgraph.add_tripstop(osm_nodemap[node.id], TripStop.OSM,
                               node.lat, node.lon)
    
    for way in osm_map.ways.values():
        previd = None
        for id in way.nds:
            if previd:
                tripgraph.add_walkhop(osm_nodemap[previd], osm_nodemap[id])
                tripgraph.add_walkhop(osm_nodemap[id], osm_nodemap[previd])

            previd = id

    print "Adding GTFS->OSM links to graph..."
    for stop_id in link_map.keys():
        tripgraph.add_walkhop(idmap.stopmap[stop_id],
                              osm_nodemap[link_map[stop_id]])
        tripgraph.add_walkhop(osm_nodemap[link_map[stop_id]],
                              idmap.stopmap[stop_id])

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
    process_gtfs(g, schedule, idmap)

    if options.osm:
        print "Loading OSM."
        osm_map = osm.OSM(options.osm)

        process_osm(g, osm_map, schedule, idmap)

    print "Saving idmap"
    idmap.save(args[2])

    print "Saving graph"
    g.save(args[1])
