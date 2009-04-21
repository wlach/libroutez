#include <boost/test/unit_test.hpp>
#include "tripgraph.h"

using namespace std;


BOOST_AUTO_TEST_CASE(basic_graph_pathfinding)
{
    TripGraph g;
    
    // simple path, just walking
    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);
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
    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);
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
    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);
    g.add_tripstop(2, TripStop::OSM, 0.0f, 1.0f);
    g.add_walkhop(0, 1);

    TripPath *p = g.find_path(0, "caturday", false, 0.0, 0.0, 0.0, 1.0);
    BOOST_CHECK(!p);
}


BOOST_AUTO_TEST_CASE(tripstops_in_range)
{
    TripGraph g;
    // north and agricola
    g.add_tripstop(0, TripStop::GTFS, 44.6554236f, -63.5936968f);
    // north and robie (just north of north&agricola)
    g.add_tripstop(1, TripStop::OSM, 44.6546407f, -63.5948438f);
    // north and northwood (just south of north&agricola)
    g.add_tripstop(2, TripStop::GTFS, 44.6567144f, -63.5919115f);
    // Quinpool and Connaught (a few kms away from north&agricola)
    g.add_tripstop(2, TripStop::GTFS, 44.6432423f, -63.6045261f);

    {
        vector<TripStop> v = g.find_tripstops_in_range(44.6554236f, 
                                                       -63.5936968f, 
                                                       TripStop::GTFS, 
                                                       500.0f);      
        BOOST_CHECK_EQUAL(v.size(), 2);
        BOOST_CHECK(v[0].id == 0 || v[0].id == 2);
        BOOST_CHECK(v[1].id == 0 || v[1].id == 2);
    }
}

