#include "wvtest.h"
#include "tripgraph.h"

using namespace std;


WVTEST_MAIN("basic_graph_pathfinding")
{
    TripGraph g;
    
    // simple path, just walking
    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);
    g.add_walkhop(0, 1);

    {
        TripPath *p = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        WVPASSEQ(actions.size(), 1);
        
        TripAction action = actions.front();
        WVPASSEQ(action.src_id, 0);
        WVPASSEQ(action.dest_id, 1);

        delete p;
    }

    // take the triphop if we have it
    {
        ServicePeriod s(0, 0, 0, 0, 7, 0, 100, 2000, true, true, true);
        g.add_service_period(s);
        g.add_triphop(500, 1000, 0, 1, 1, 1, 0);
    }

    {
        TripPath *p = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        WVPASSEQ(actions.size(), 1);
        
        TripAction action = actions.front();
        WVPASSEQ(action.src_id, 0);
        WVPASSEQ(action.dest_id, 1);
        WVPASSEQ(action.start_time, 500.0f);
        WVPASSEQ(action.end_time, 1000.0f);

        delete p;
    }
}


WVTEST_MAIN("basic_graph_saveload")
{
    TripGraph g;
    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);
    g.add_walkhop(0, 1);

    ServicePeriod s(0, 1, 0, 0, 7, 0, 100, 2000, true, true, true);
    g.add_service_period(s);
    g.add_triphop(500, 1000, 0, 1, 1, 1, 0);

    char *tmpgraphname = tmpnam(NULL); // security issues in unit tests? bah.
    unlink(tmpgraphname);
    g.save(tmpgraphname);

    TripGraph g2;
    g2.load(tmpgraphname);

    // verify that we have two tripstops
    for (int i=0; i<2; i++) 
    {
        TripStop ts = g2.get_tripstop(i);
        WVPASSEQ(ts.type, TripStop::OSM);
    }

    // verify that we can still solve a basic path
    {
        TripPath *p = g.find_path(0, false, 0.0, 0.0, 1.0, 0.0);
        
        std::list<TripAction> actions = p->get_actions();
        WVPASSEQ(actions.size(), 1);

        TripAction action = actions.front();
        WVPASSEQ(action.src_id, 0);
        WVPASSEQ(action.dest_id, 1);
        WVPASSEQ(action.start_time, 500.0f);
        WVPASSEQ(action.end_time, 1000.0f);
        
        delete p;
    }    
}


WVTEST_MAIN("impossible_path")
{
    TripGraph g;
    g.add_tripstop(0, TripStop::OSM, 0.0f, 0.0f);
    g.add_tripstop(1, TripStop::OSM, 1.0f, 0.0f);
    g.add_tripstop(2, TripStop::OSM, 0.0f, 1.0f);
    g.add_walkhop(0, 1);

    TripPath *p = g.find_path(0, false, 0.0, 0.0, 0.0, 1.0);
    WVPASS(!p);
}


WVTEST_MAIN("tripstops_in_range")
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
        WVPASSEQ(v.size(), 2);
        WVPASS(v[0].id == 0 || v[0].id == 2);
        WVPASS(v[1].id == 0 || v[1].id == 2);
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

WVTEST_MAIN("service_periods")
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    {
        ServicePeriod s(0, 1, 0, 108, 7, 0, 108, 2000, false, true, false);
        g.add_service_period(s);
    }

    // test something that's within a supported service period
    // Saturday Midnight Jan 5th 2008
    {
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 0, 108));
        WVPASSEQ(vsp.size(), 1);
        WVPASSEQ(vsp[0].first, 0);
    }

    // test something outside a supported service period: day
    // Saturday Midnight Jan 11th 2008
    {
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(11, 0, 108));
        WVPASSEQ(vsp.size(), 0);
    }
    // test something outside a supported service period: month
    // Saturday Midnight Feb 5th 2008
    {
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 108));
        WVPASSEQ(vsp.size(), 0);
    }

    // test something outside a supported service period: year
    // Saturday Midnight Jan 11th 2009
    {
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 1, 109));
        WVPASSEQ(vsp.size(), 0);
    }

    // add another service period (saturdays for month of january)
    {
        ServicePeriod s(1, 1, 0, 108, 31, 0, 108, 2000, false, true,
                        false);
        g.add_service_period(s);
    }

    // test something that's within _two_ supported service periods
    // Saturday Midnight Jan 5th 2008
    {
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 0, 108));
        WVPASSEQ(vsp.size(), 2);
        WVPASS(vsp[0].first==0 || vsp[0].first==1);
        WVPASS(vsp[1].first==0 || vsp[1].first==1);
        WVFAILEQ(vsp[0].first, vsp[1].first);
    }    

    // save graph, reload, make sure service periods are still there
}


WVTEST_MAIN("service_periods_overlapping")
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    // (weekday and saturday schedules)
    {
        ServicePeriod s1(0, 1, 0, 108, 7, 0, 108, 90000, false, true, false);
        g.add_service_period(s1);
        ServicePeriod s2(1, 1, 0, 108, 7, 0, 108, 90000, true, false, false);
        g.add_service_period(s2);
    }

    vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(5, 0, 108));
    WVPASSEQ(vsp.size(), 2);
    WVPASS(vsp[0].first==0 || vsp[0].first==1);
    WVPASS(vsp[1].first==0 || vsp[1].first==1);
    WVFAILEQ(vsp[0].first, vsp[1].first);
    
    int weekday_index = (vsp[0].first == 1) ? 0 : 1;
    WVPASSEQ(vsp[weekday_index].second, 86400);
}


WVTEST_MAIN("service_periods_turned_on_or_off")
{
    TripGraph g;

    // from the 1st to the 7th (i.e. 1st saturday only)
    // turn off weekday service on the 2nd (wednesday)
    // turn on saturday service on the 3rd (keeping weekday service)
    {
        ServicePeriod s1(0, 1, 0, 108, 7, 0, 108, 80000, false, true, false);
        s1.add_exception_on(3, 0, 108);
        WVPASSEQ(s1.is_turned_on(3, 0, 108), true);
        WVPASSEQ(s1.is_turned_on(4, 0, 108), false);
        g.add_service_period(s1);
        ServicePeriod s2(1, 1, 0, 108, 7, 0, 108, 80000, true, false, false);
        s2.add_exception_off(2, 0, 108);
        WVPASSEQ(s2.is_turned_off(2, 0, 108), true);
        WVPASSEQ(s2.is_turned_off(3, 0, 108), false);
        g.add_service_period(s2);
    }

    {    
        // should be no service on the 2nd
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(2, 0, 108));
        WVPASSEQ(vsp.size(), 0);
    }

    {
        // should be two service periods on the 3rd (saturday and weekday)
        vector<pair<int, int> > vsp = g.get_service_period_ids_for_time(get_time_t(3, 0, 108));
        WVPASSEQ(vsp.size(), 2);
        WVPASS(vsp[0].first==0 || vsp[0].first==1);
        WVPASS(vsp[1].first==0 || vsp[1].first==1);
        WVFAILEQ(vsp[0].first, vsp[1].first);
    }    
}


WVTEST_MAIN("service_periods_save_load")
{
    TripGraph g;

    // use the same setup as the previous test: saturday and weekday schedules 
    // with a few exceptions

    // from the 1st to the 7th (i.e. 1st saturday only)
    // turn off weekday service on the 2nd (wednesday)
    // turn on saturday service on the 3rd (keeping weekday service)
    {
        ServicePeriod s1(0, 1, 0, 108, 7, 0, 108, 80000, false, true, false);
        s1.add_exception_on(3, 0, 108);
        g.add_service_period(s1);
        ServicePeriod s2(1, 1, 0, 108, 7, 0, 108, 80000, true, false, false);
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
        vector<pair<int, int> > vsp = g2.get_service_period_ids_for_time(get_time_t(2, 0, 108));
        WVPASSEQ(vsp.size(), 0);
    }

    {
        // should be two service periods on the 3rd (saturday and weekday)
        vector<pair<int, int> > vsp = g2.get_service_period_ids_for_time(get_time_t(3, 0, 108));
        WVPASSEQ(vsp.size(), 2);
        WVPASS(vsp[0].first==0 || vsp[0].first==1);
        WVPASS(vsp[1].first==0 || vsp[1].first==1);
        WVFAILEQ(vsp[0].first, vsp[1].first);
    }    
}
