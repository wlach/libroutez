#!/usr/bin/python

# The code in this module was gratuitously stolen from 
# graphserver (http://graphserver.sourceforge.net/)
# Copyright (c) 2007, Brandon Martin-Anderson

import xml.sax
import copy
import sys
from math import *

class Node:
    def __init__(self, id, lon, lat):
        self.id = id
        self.lon = lon
        self.lat = lat
        self.tags = {}

class Way:
    def __init__(self, id, osm):
        self.osm = osm
        self.id = id
        self.nds = []
        self.tags = {}

    def split(self, dividers):
        # slice the node-array using this nifty recursive function
        def slice_array(ar, dividers):
            for i in range(1,len(ar)-1):
                if dividers[ar[i]]>1:
                    #print "slice at %s"%ar[i]
                    left = ar[:i+1]
                    right = ar[i:]

                    rightsliced = slice_array(right, dividers)

                    return [left]+rightsliced
            return [ar]

        slices = slice_array(self.nds, dividers)

        # create a way object for each node-array slice
        ret = []
        i=0
        for slice in slices:
            littleway = copy.copy( self )
            littleway.id += "-%d"%i
            littleway.nds = slice
            ret.append( littleway )
            i += 1

        return ret

    def get_bbox(self):
        (min_lat, min_lon, max_lat, max_lon) = (None, None, None, None)
        for id in self.nds:
            node = self.osm.nodes[ id ]
            if not min_lat or node.lat < min_lat:
                min_lat = node.lat
            if not min_lon or node.lon < min_lon:
                min_lon = node.lon
            if not max_lat or node.lat > max_lat:
                max_lat = node.lat
            if not max_lon or node.lon > max_lon:
                max_lon = node.lon
        return (min_lon, min_lat, max_lon, max_lat)

    def get_projected_points(self, reprojection_func=lambda x,y:(x,y)):
        """nodedir is a dictionary of nodeid->node objects. If reprojection_func is None, returns unprojected points"""
        ret = []

        for nodeid in self.nds:
            node = self.osm.nodes[ nodeid ]
            ret.append( reprojection_func(node.lon,node.lat) )

        return ret

    def to_canonical(self, srid, reprojection_func=None):
        """Returns canonical string for this geometry"""

        return "SRID=%d;LINESTRING(%s)"%(srid, ",".join( ["%f %f"%(x,y) for x,y in self.get_projected_points()] ) )

    @property
    def fromv(self):
        return self.nds[0]

    @property
    def tov(self):
        return self.nds[-1]

class OSM:

    def __init__(self, filename_or_stream):
        """ File can be either a filename or stream/file object."""
        nodes = {}
        ways = {}

        superself = self

        class OSMHandler(xml.sax.ContentHandler):
            @classmethod
            def setDocumentLocator(self,loc):
                pass

            @classmethod
            def startDocument(self):
                pass

            @classmethod
            def endDocument(self):
                pass

            @classmethod
            def startElement(self, name, attrs):
                if name=='node':
                    if (int(attrs['id']) % 1000) == 0:
                        print "Parsing node %s" % attrs['id']
                    self.currElem = Node(attrs['id'], float(attrs['lon']), float(attrs['lat']))
                elif name=='way':
                    if (int(attrs['id']) % 1000) == 0:
                        print "Parsing way %s" % attrs['id']
                    self.currElem = Way(attrs['id'], superself)
                elif name=='tag':
                    pass
                    #self.currElem.tags[attrs['k']] = attrs['v']
                elif name=='nd':
                    self.currElem.nds.append( attrs['ref'] )

            @classmethod
            def endElement(self,name):
                if name=='node':
                    nodes[self.currElem.id] = self.currElem
                elif name=='way':
                    ways[self.currElem.id] = self.currElem

            @classmethod
            def characters(self, chars):
                pass

        xml.sax.parse(filename_or_stream, OSMHandler)

        self.nodes = nodes
        self.ways = ways
        self.optimize()

    def optimize(self):
        """Optimize ways for building graph."""
        #count times each node is used
        node_histogram = dict.fromkeys( self.nodes.keys(), 0 )
        print "Counting and pruning ways"
        for way in self.ways.values():
            #if a way has only one node, delete it out of the osm collection
            #similarly if it's not a road
            if len(way.nds) < 2:# or not way.tags.get('highway') or way.tags['highway'] == 'footway':  
                del self.ways[way.id]
            else:
                for node in way.nds:
                    # toss out any ways that don't have all nodes on map
                    if not self.nodes.get(node) and self.ways.get(way.id):
                        del self.ways[way.id]
                    elif self.ways.get(way.id):
                        node_histogram[node] += 1

        # delete nodes that don't appear in ways
        for node in self.nodes.values():
            if node_histogram[node.id] == 0:
                del self.nodes[node.id]

        #use that histogram to split all ways, replacing the member set of ways
        print "Splitting ways"
        new_ways = {}
        for id, way in self.ways.iteritems():
            split_ways = way.split(node_histogram)
            for split_way in split_ways:
                new_ways[split_way.id] = split_way
        self.ways = new_ways

    @property
    def connecting_nodes(self):
        """List of nodes that are the endpoint of one or more ways"""

        ret = {}
        for way in self.ways.values():
            ret[way.fromv] = self.nodes[way.fromv]
            ret[way.tov] = self.nodes[way.tov]

        return ret
