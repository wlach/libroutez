#include <boost/test/unit_test.hpp>
#include "tripgraph.h"

BOOST_AUTO_TEST_CASE(basic_graph_pathfinding)
{
    TripGraph g;
    
    g.add_tripstop("osm1", "osm", 0.0f, 0.0f);
    g.add_tripstop("osm2", "osm", 1.0f, 0.0f);
    g.add_walkhop("osm1", "osm2");

    TripPath p = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0);

    std::list<TripAction> actions = p.get_actions();
    BOOST_CHECK(actions.size() == 1);

    TripAction action = actions.front();
    BOOST_CHECK(action.src_id == "osm1");
    BOOST_CHECK(action.dest_id == "osm2");
}

