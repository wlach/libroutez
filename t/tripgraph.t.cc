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
        TripPath *p = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        BOOST_CHECK_EQUAL(actions.size(), 1);
        
        TripAction action = actions.front();
        BOOST_CHECK_EQUAL(action.src_id, 0);
        BOOST_CHECK_EQUAL(action.dest_id, 1);
    }

    // take the triphop if we have it
    g.add_triphop(500, 1000, 0, 1, 1, 1, "caturday");

    {
        TripPath *p = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0);
        
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
        TripPath *p = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0);
        
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

    TripPath *p = g.find_path(0, false, 0.0, 0.0, 0.0, 1.0);
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
    g.add_tripstop(3, TripStop::GTFS, 44.6432423f, -63.6045261f);

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


static time_t get_time_t(int tm_mday, int tm_mon, int tm_year)
{
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = tm_mday;
    t.tm_mon = tm_mon;
    t.tm_year = tm_year;
    t.tm_wday = -1;
    t.tm_yday = -1;
    t.tm_isdst = -1;

    return mktime(&t);
}

BOOST_AUTO_TEST_CASE(service_periods)
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    {
        ServicePeriod s("saturday_2008", 1, 1, 108, 7, 1, 108, false, true, false);
        g.add_service_period(s);
    }

    // test something that's within a supported service period
    // Saturday Midnight Jan 5th 2008
    {
        vector<string> vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 1);
        BOOST_CHECK_EQUAL(vsp[0], string("saturday_2008"));
    }    
    // test something outside a supported service period: day
    // Saturday Midnight Jan 11th 2008
    {
        vector<string> vsp = g.get_service_period_ids_for_time(get_time_t(11, 1, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }
    // test something outside a supported service period: month
    // Saturday Midnight Feb 5th 2008
    {
        vector<string> vsp = g.get_service_period_ids_for_time(get_time_t(5, 2, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }

    // test something outside a supported service period: year
    // Saturday Midnight Jan 11th 2009
    {
        vector<string> vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 109));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }

    // add another service period (saturdays for month of january)
    {
        ServicePeriod s("saturday_jan_2008", 1, 1, 108, 31, 1, 108, false, true, 
                        false);
        g.add_service_period(s);
    }

    // test something that's within _two_ supported service periods
    // Saturday Midnight Jan 5th 2008
    {
        vector<string> vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 2);
        BOOST_CHECK(vsp[0]==string("saturday_2008") || vsp[0]==string("saturday_jan_2008"));
        BOOST_CHECK(vsp[1]==string("saturday_2008") || vsp[1]==string("saturday_jan_2008"));
        BOOST_CHECK_NE(vsp[0], vsp[1]);
    }    

    // save graph, reload, make sure service periods are still there
}


BOOST_AUTO_TEST_CASE(service_periods_save_load)
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    {
        ServicePeriod s("saturday_2008", 1, 1, 108, 7, 1, 108, false, true, false);
        g.add_service_period(s);
    }
    // add another service period (saturdays for month of january)
    {
        ServicePeriod s("saturday_jan_2008", 1, 1, 108, 31, 1, 108, false, true, 
                        false);
        g.add_service_period(s);
    }


    char *tmpgraphname = tmpnam(NULL); // security issues in unit tests? bah.
    unlink(tmpgraphname);
    g.save(tmpgraphname);

    TripGraph g2;
    g2.load(tmpgraphname);

    {
        vector<string> vsp = g2.get_service_period_ids_for_time(get_time_t(5, 1, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 2);
        BOOST_CHECK(vsp[0]==string("saturday_2008") || vsp[0]==string("saturday_jan_2008"));
        BOOST_CHECK(vsp[1]==string("saturday_2008") || vsp[1]==string("saturday_jan_2008"));
        BOOST_CHECK_NE(vsp[0], vsp[1]);
    }    
}
