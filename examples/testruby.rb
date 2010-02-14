#!/usr/bin/ruby

# This example is just here to give a simple example of using the ruby
# API. 

# FIXME: It would be nice to have some more ruby examples, but I'll leave
# that to the ruby hackers.

# This is all slightly icky, but it's probably enough to 
# help you get started

require 'routez'

g = Routez::TripGraph.new()
g.add_tripstop(0, Routez::TripStop::OSM, 0.0, 0.0)
g.add_tripstop(1, Routez::TripStop::OSM, 1.0, 0.0)
g.add_walkhop(0, 1)

path = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0)

path.get_actions().each do |action|
  puts "src: #{action.src_id} dest: #{action.dest_id} st: #{action.start_time} et: #{action.end_time} rid: #{action.route_id}" 
end

s = Routez::ServicePeriod.new(0, 1, 0, 0, 7, 0, 100, 2000, true, true, true)
g.add_service_period(s);
g.add_triphop(500, 1000, 0, 1, 1, 1, 0)
path2 = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0)

path2.get_actions().each do |action|
  puts "src: #{action.src_id} dest: #{action.dest_id} st: #{action.start_time} et: #{action.end_time} rid: #{action.route_id}" 
end


g = Routez::TripGraph.new()

g.add_tripstop(0, Routez::TripStop::GTFS, 44.6554236, -63.5936968) # north and agricola
g.add_tripstop(1, Routez::TripStop::OSM, 44.6546407, -63.5948438) # north and robie (just north of north&agricola)
g.add_tripstop(2, Routez::TripStop::GTFS, 44.6567144, -63.5919115) # north and northwood (just south of north&agricola)
g.add_tripstop(3, Routez::TripStop::GTFS, 44.6432423, -63.6045261) # Quinpool and Connaught (a few kms away from north&agricola)

stops = g.find_tripstops_in_range(44.6554236, -63.5936968, Routez::TripStop::GTFS, 500.0)

stops.each do |stop|
  puts "id: #{stop.id} lat: #{stop.lat} lon: #{stop.lng} type: #{stop.type}"
end
