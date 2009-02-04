#include <boost/test/unit_test.hpp>
#include "tripgraph.h"

BOOST_AUTO_TEST_CASE(basic_graph_pathfinding)
{
    TripGraph g;
    
    // simple path, just walking
    g.add_tripstop(0, "osm", 0.0f, 0.0f);
    g.add_tripstop(1, "osm", 1.0f, 0.0f);
    g.add_walkhop(0, 1);

    {
        TripPath *p = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        BOOST_CHECK_EQUAL(actions.size(), 1);
        
        TripAction action = actions.front();
        BOOST_CHECK_EQUAL(action.src_id, 0);
        BOOST_CHECK_EQUAL(action.dest_id, 1);
    }

    // take the triphop if we have it
    g.add_triphop(500, 1000, 0, 1, 1, 1, "caturday");

    {
        TripPath *p = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        BOOST_CHECK_EQUAL(actions.size(), 1);
        
        TripAction action = actions.front();
        BOOST_CHECK_EQUAL(action.src_id, 0);
        BOOST_CHECK_EQUAL(action.dest_id, 1);
        BOOST_CHECK_EQUAL(action.start_time, 500.0f);
        BOOST_CHECK_EQUAL(action.end_time, 1000.0f);

        delete p;
    }
}


BOOST_AUTO_TEST_CASE(basic_graph_saveload)
{
    TripGraph g;
    g.add_tripstop(0, "osm", 0.0f, 0.0f);
    g.add_tripstop(1, "osm", 1.0f, 0.0f);
    g.add_walkhop(0, 1);
    g.add_triphop(500, 1000, 0, 1, 1, 1, "caturday");

    char *tmpgraphname = tmpnam(NULL); // security issues in unit tests? bah.
    unlink(tmpgraphname);
    g.save(tmpgraphname);

    TripGraph g2;
    g2.load(tmpgraphname);

    {
        TripPath *p = g.find_path(0, "caturday", false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        BOOST_CHECK_EQUAL(actions.size(), 1);

        TripAction action = actions.front();
        BOOST_CHECK_EQUAL(action.src_id, 0);
        BOOST_CHECK_EQUAL(action.dest_id, 1);
        BOOST_CHECK_EQUAL(action.start_time, 500.0f);
        BOOST_CHECK_EQUAL(action.end_time, 1000.0f);
        
        delete p;
    }    
}


BOOST_AUTO_TEST_CASE(impossible_path)
{
    TripGraph g;
    g.add_tripstop(0, "osm", 0.0f, 0.0f);
    g.add_tripstop(1, "osm", 1.0f, 0.0f);
    g.add_tripstop(2, "osm", 0.0f, 1.0f);
    g.add_walkhop(0, 1);

    TripPath *p = g.find_path(0, "caturday", false, 0.0, 0.0, 0.0, 1.0);
    BOOST_CHECK(!p);
}
