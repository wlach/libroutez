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
    {
        ServicePeriod s("all", 0, 0, 0, 7, 0, 100, 2000, true, true, true);
        g.add_service_period(s);
        g.add_triphop(500, 1000, 0, 1, 1, 1, "all");
    }

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

    ServicePeriod s("all", 1, 0, 0, 7, 0, 100, 2000, true, true, true);
    g.add_service_period(s);
    g.add_triphop(500, 1000, 0, 1, 1, 1, "all");

    char *tmpgraphname = tmpnam(NULL); // security issues in unit tests? bah.
    unlink(tmpgraphname);
    g.save(tmpgraphname);

    TripGraph g2;
    g2.load(tmpgraphname);

    // verify that we have two tripstops
    for (int i=0; i<2; i++) 
    {
        TripStop ts = g2.get_tripstop(1);
        BOOST_CHECK_EQUAL(ts.type, TripStop::OSM);
    }

    // verify that we can still solve a basic path
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


BOOST_AUTO_TEST_CASE(getting_route_ids)
{
    TripGraph g;

    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);

    ServicePeriod s("all", 0, 0, 0, 7, 0, 100, 2000, true, true, true);
    g.add_service_period(s);
    ServicePeriod s1("all2", 0, 0, 0, 7, 0, 100, 2000, true, true, true);
    g.add_service_period(s1);

    list<int> route_ids;

    // just one triphop, route id 1
    g.add_triphop(500, 1000, 0, 1, 1, 1, "all");
    route_ids = g.get_route_ids_for_stop(0, 0.0f);
    BOOST_CHECK_EQUAL(route_ids.size(), 1);
    BOOST_CHECK(*route_ids.begin() == 1);

    // an identical triphop, different service period, same route id
    g.add_triphop(500, 1000, 0, 1, 1, 1, "all2");
    route_ids = g.get_route_ids_for_stop(0, 0.0f);
    BOOST_CHECK_EQUAL(route_ids.size(), 1);
    BOOST_CHECK_EQUAL(*route_ids.begin(), 1);

    // an identical triphop, different service period, diff route id
    g.add_triphop(500, 1000, 0, 1, 2, 1, "all2");

    route_ids = g.get_route_ids_for_stop(0, 0.0f);
    BOOST_CHECK_EQUAL(route_ids.size(), 2);
    BOOST_CHECK(*route_ids.begin() == 1 || *route_ids.begin() == 2);    BOOST_CHECK(*(++(route_ids.begin())) == 1 || *(++(route_ids.begin())) == 2);
    BOOST_CHECK_NE(*(route_ids.begin()), *(++(route_ids.begin())));
}


WVTEST_MAIN("find_triphops_for_stop")
{
    setenv("TZ", "GMT", 1);
    tzset();

    TripGraph g;

    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);

    ServicePeriod s("all", 0, 0, 0, 7, 0, 100, 2000, true, true, true);
    g.add_service_period(s);
    ServicePeriod s1("all2", 0, 0, 0, 7, 0, 100, 2000, true, true, true);
    g.add_service_period(s1);

    for (int i=0; i<5; i++)
    {
        g.add_triphop(500 + i, 1000 + i, 0, 1, 1, 1, "all");
    }
    g.add_triphop(501, 1001, 0, 1, 1, 1, "all2");


    vector<TripHop> ths = g.find_triphops_for_stop(0, 1, 0, 3);
    BOOST_CHECK_EQUAL(ths.size(), 3);
    BOOST_CHECK_EQUAL(ths[0].start_time, 500);
    BOOST_CHECK_EQUAL(ths[1].start_time, 501);
    BOOST_CHECK_EQUAL(ths[2].start_time, 501);
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
        ServicePeriod s("saturday_2008", 1, 0, 108, 7, 0, 108, 2000, false, true, false);
        g.add_service_period(s);
    }

    // test something that's within a supported service period
    // Saturday Midnight Jan 5th 2008
    {
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 1);
        BOOST_CHECK_EQUAL(vsp[0].first, string("saturday_2008"));
    }    
    // test something outside a supported service period: day
    // Saturday Midnight Jan 11th 2008
    {
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(11, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }
    // test something outside a supported service period: month
    // Saturday Midnight Feb 5th 2008
    {
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }

    // test something outside a supported service period: year
    // Saturday Midnight Jan 11th 2009
    {
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 109));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }

    // add another service period (saturdays for month of january)
    {
        ServicePeriod s("saturday_jan_2008", 1, 0, 108, 31, 0, 108, 2000, false, true, 
                        false);
        g.add_service_period(s);
    }

    // test something that's within _two_ supported service periods
    // Saturday Midnight Jan 5th 2008
    {
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 2);
        BOOST_CHECK(vsp[0].first==string("saturday_2008") || vsp[0].first==string("saturday_jan_2008"));
        BOOST_CHECK(vsp[1].first==string("saturday_2008") || vsp[1].first==string("saturday_jan_2008"));
        BOOST_CHECK_NE(vsp[0].first, vsp[1].first);
    }    

    // save graph, reload, make sure service periods are still there
}


BOOST_AUTO_TEST_CASE(service_periods_overlapping)
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    {
        ServicePeriod s1("saturday_2008", 1, 0, 108, 7, 0, 108, 90000, false, true, false);
        g.add_service_period(s1);
        ServicePeriod s2("weekday_2008", 1, 0, 108, 7, 0, 108, 90000, true, false, false);
        g.add_service_period(s2);
    }

    vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 0, 108));
    BOOST_CHECK_EQUAL(vsp.size(), 2);
    BOOST_CHECK(vsp[0].first==string("saturday_2008") || vsp[0].first==string("weekday_2008"));
    BOOST_CHECK(vsp[1].first==string("saturday_2008") || vsp[1].first==string("weekday_2008"));
    BOOST_CHECK_NE(vsp[0].first, vsp[1].first);
    
    int weekday_index = (vsp[0].first == string("weekday_2008")) ? 0 : 1;
    BOOST_CHECK_EQUAL(vsp[weekday_index].second, 86400);
}


BOOST_AUTO_TEST_CASE(service_periods_turned_on_or_off)
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    // turn off weekday service on the 2nd (wednesday)
    // turn on saturday service on the 3rd (keeping weekday service)
    {
        ServicePeriod s1("saturday_2008", 1, 0, 108, 7, 0, 108, 80000, false, true, false);
        s1.add_exception_on(3, 0, 108);
        BOOST_CHECK_EQUAL(s1.is_turned_on(3, 0, 108), true);
        BOOST_CHECK_EQUAL(s1.is_turned_on(4, 0, 108), false);
        g.add_service_period(s1);
        ServicePeriod s2("weekday_2008", 1, 0, 108, 7, 0, 108, 80000, true, false, false);
        s2.add_exception_off(2, 0, 108);
        BOOST_CHECK_EQUAL(s2.is_turned_off(2, 0, 108), true);
        BOOST_CHECK_EQUAL(s2.is_turned_off(3, 0, 108), false);
        g.add_service_period(s2);
    }

    {    
        // should be no service on the 2nd
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(2, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }

    {
        // should be two service periods on the 3rd (saturday and weekday)
        vector<pair<string, int> > vsp = g.get_service_period_ids_for_time(get_time_t(3, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 2);
        BOOST_CHECK(vsp[0].first==string("saturday_2008") || vsp[0].first==string("weekday_2008"));
        BOOST_CHECK(vsp[1].first==string("saturday_2008") || vsp[1].first==string("weekday_2008"));
        BOOST_CHECK_NE(vsp[0].first, vsp[1].first);
    }    
}


BOOST_AUTO_TEST_CASE(service_periods_save_load)
{
    TripGraph g;

    // use the same setup as the previous test: saturday and weekday schedules 
    // with a few exceptions

    // from the 1st to the 7th (i.e. 1st saturday only)
    // turn off weekday service on the 2nd (wednesday)
    // turn on saturday service on the 3rd (keeping weekday service)
    {
        ServicePeriod s1("saturday_2008", 1, 0, 108, 7, 0, 108, 80000, false, true, false);
        s1.add_exception_on(3, 0, 108);
        g.add_service_period(s1);
        ServicePeriod s2("weekday_2008", 1, 0, 108, 7, 0, 108, 80000, true, false, false);
        s2.add_exception_off(2, 0, 108);
        g.add_service_period(s2);
    }

    char *tmpgraphname = tmpnam(NULL); // security issues in unit tests? bah.
    unlink(tmpgraphname);
    g.save(tmpgraphname);

    TripGraph g2;
    g2.load(tmpgraphname);

    {    
        // should be no service on the 2nd
        vector<pair<string, int> > vsp = g2.get_service_period_ids_for_time(get_time_t(2, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 0);
    }

    {
        // should be two service periods on the 3rd (saturday and weekday)
        vector<pair<string, int> > vsp = g2.get_service_period_ids_for_time(get_time_t(3, 0, 108));
        BOOST_CHECK_EQUAL(vsp.size(), 2);
        BOOST_CHECK(vsp[0].first==string("saturday_2008") || vsp[0].first==string("weekday_2008"));
        BOOST_CHECK(vsp[1].first==string("saturday_2008") || vsp[1].first==string("weekday_2008"));
        BOOST_CHECK_NE(vsp[0].first, vsp[1].first);
    }    
}
