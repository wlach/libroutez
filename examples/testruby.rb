#!/usr/bin/ruby

# This example is just here to give a simple example of using the ruby
# API. 

# FIXME: It would be nice to have some more ruby examples, but I'll leave
# that to the ruby hackers.

require 'routez'

g = Routez::TripGraph.new()
g.add_tripstop("gtfs1", "gtfs", 0.0, 0.0)
g.add_tripstop("gtfs2", "gtfs", 0.5, 0.0)
g.add_triphop(500, 1, "gtfs1", "gtfs2", 1, "caturday")
g.add_walkhop("gtfs1", "gtfs2")

path = g.find_path(0, "caturday", false, 0.0, 0.0, 0.5, 0.0)

for action in path.get_actions()
  puts "src: #{action.src_id} dest: #{action.dest_id} st: #{action.start_time} et: #{action.end_time} rid: #{action.route_id}" 
end
