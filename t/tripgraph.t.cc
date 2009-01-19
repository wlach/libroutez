#include <boost/test/unit_test.hpp>
#include "tripgraph.h"

BOOST_AUTO_TEST_CASE(basic_graph_pathfinding)
{
    TripGraph g;
    
    g.add_tripstop("osm1", "osm", 0.0f, 0.0f);
    g.add_tripstop("osm2", "osm", 1.0f, 0.0f);
    g.add_walkhop("osm1", "osm2");

    {
        TripPath p = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p.get_actions();
        BOOST_CHECK_EQUAL(actions.size(), 1);
        
        TripAction action = actions.front();
        BOOST_CHECK_EQUAL(action.src_id, "osm1");
        BOOST_CHECK_EQUAL(action.dest_id, "osm2");
    }

    g.add_triphop(500, 1000, "osm1", "osm2", 1, 1, "caturday");

    {
        TripPath p = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p.get_actions();
        BOOST_CHECK_EQUAL(actions.size(), 1);
        
        TripAction action = actions.front();
        BOOST_CHECK_EQUAL(action.src_id, "osm1");
        BOOST_CHECK_EQUAL(action.dest_id, "osm2");
        BOOST_CHECK_EQUAL(action.start_time, 500.0f);
        BOOST_CHECK_EQUAL(action.end_time, 1000.0f);
    }
    
}


