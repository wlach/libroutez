#!/usr/bin/ruby

# This example is just here to give a simple example of using the ruby
# API. 

# FIXME: It would be nice to have some more ruby examples, but I'll leave
# that to the ruby hackers.

require 'routez'

g = Routez::TripGraph.new()
g.add_tripstop(0, Routez::TripStop::OSM, 0.0, 0.0)
g.add_tripstop(1, Routez::TripStop::OSM, 1.0, 0.0)
g.add_walkhop(0, 1)

path = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0)

path.get_actions().each do |action|
  puts "src: #{action.src_id} dest: #{action.dest_id} st: #{action.start_time} et: #{action.end_time} rid: #{action.route_id}" 
end

g.add_triphop(500, 1000, 0, 1, 1, 1, "caturday")
path2 = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0)

path2.get_actions().each do |action|
  puts "src: #{action.src_id} dest: #{action.dest_id} st: #{action.start_time} et: #{action.end_time} rid: #{action.route_id}" 
end
