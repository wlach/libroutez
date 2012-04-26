#!/usr/bin/python
 
import zipfile
import csv
from optparse import OptionParser
 
parser = OptionParser()
parser.add_option("-m", action="store_true", dest="minimal")
(options, args) = parser.parse_args()
 

zip = zipfile.ZipFile(args[0], mode='r')

stoptext = zip.open("stops.txt")
lines = csv.reader(stoptext)

descriptors = lines.next()
(stop_lat_descriptor, stop_lng_descriptor) = (-1, -1)
id = 0
for descriptor in descriptors:
    if descriptor == "stop_lat":
        stop_lat_descriptor = id
    elif descriptor == "stop_lon":
        stop_lng_descriptor = id
    id+=1

(min_lat, min_lng, max_lat, max_lng) = (0.0, 0.0, 0.0, 0.0)
for stop_info in lines:
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

if options.minimal:
    print "%s %s %s %s" % (min_lat, min_lng, max_lat, max_lng)
else:
    print "Polygon:"
    print "%s\t%s" % (min_lat, min_lng)
    print "%s\t%s" % (min_lat, max_lng)
    print "%s\t%s" % (max_lat, max_lng)
    print "%s\t%s" % (max_lat, min_lng)
    print "min_lat, min_lng, max_lat, max_lng: %s %s %s %s" % \
        (min_lat, min_lng, max_lat, max_lng)
