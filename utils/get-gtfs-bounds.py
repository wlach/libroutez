#!/usr/bin/python
 
import zipfile
from optparse import OptionParser
 
if __name__ == '__main__':
    parser = OptionParser()
    (options, args) = parser.parse_args()
 
    zip = zipfile.ZipFile(args[0], mode='r')
    stoptext = zip.read("stops.txt")
    lines = stoptext.split('\n')
    
    descriptors = lines[0].split(',')
    (stop_lat_descriptor, stop_lng_descriptor) = (-1, -1)
    id = 0
    for descriptor in descriptors:
        if descriptor == "stop_lat":
            stop_lat_descriptor = id
        elif descriptor == "stop_lon":
            stop_lng_descriptor = id
        id+=1
 
    (min_lat, min_lng, max_lat, max_lng) = (0.0, 0.0, 0.0, 0.0)
    for line in lines[1:-2:]:
        stop_info = line.split(',')
        (lat, lng) = (float(stop_info[stop_lat_descriptor]),
                      float(stop_info[stop_lng_descriptor]))
        if min_lat == 0.0 or lat < min_lat:
            min_lat = lat
        if min_lng == 0.0 or lng < min_lng:
            min_lng = lng
        if max_lat == 0.0 or lat > max_lat:
            max_lat = lat
        if max_lng == 0.0 or lng > max_lng:
            max_lng = lng
 
    print "Polygon:"
    print "%s\t%s" % (min_lat, min_lng)
    print "%s\t%s" % (min_lat, max_lng)
    print "%s\t%s" % (max_lat, max_lng)
    print "%s\t%s" % (max_lat, min_lng)
    print "min_lat, min_lng, max_lat, max_lng: %s %s %s %s" % \
        (min_lat, min_lng, max_lat, max_lng)
