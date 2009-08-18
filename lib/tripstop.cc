#include "tripstop.h"
#include <algorithm>
#include "defuns.h"


using namespace std;
using namespace boost;


TripStop::TripStop(FILE *fp) 
{
    assert(fread(&id, sizeof(int32_t), 1, fp) == 1);
    assert(fread(&type, sizeof(Type), 1, fp) == 1);
    assert(fread(&lat, sizeof(float), 1, fp) == 1);
    assert(fread(&lng, sizeof(float), 1, fp) == 1);
        
    uint8_t have_triphops;
    assert(fread(&have_triphops, sizeof(uint8_t), 1, fp) == 1);

    if (have_triphops)
    {
        tdict = shared_ptr<ServiceDict>(new ServiceDict);

        uint32_t num_service_periods;
        assert(fread(&num_service_periods, sizeof(uint32_t), 1, fp) == 1);
        for (uint32_t i=0; i<num_service_periods; i++)
        {
            char service_period[MAX_ID_LEN];
            assert(fread(service_period, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);

            uint32_t num_route_ids;
            assert(fread(&num_route_ids, sizeof(uint32_t), 1, fp) == 1);
            for (uint32_t j=0; j<num_route_ids; j++)
            {
                int32_t route_id;
                assert(fread(&route_id, sizeof(int32_t), 1, fp) == 1);

                uint32_t num_triphops = 0;
                assert(fread(&num_triphops, sizeof(uint32_t), 1, fp) == 1);
                (*tdict)[service_period][route_id].reserve(num_triphops);
                for (uint32_t k=0; k<num_triphops; k++)
                {
                    TripHop *t = new TripHop;
                    assert(fread(t, sizeof(TripHop), 1, fp) == 1);
                    assert(t->end_time >= t->start_time); // FIXME: should be >, no?
                    (*tdict)[service_period][route_id].push_back(shared_ptr<TripHop>(t));
                }
            }

        }
    }

    uint32_t num_walkhops = 0;
    assert(fread(&num_walkhops, sizeof(uint32_t), 1, fp) == 1);
    for (int i=0; i<num_walkhops; i++)
    {
        int32_t dest_id;
        float walktime;
        assert(fread(&dest_id, sizeof(int32_t), 1, fp) == 1);
        assert(fread(&walktime, sizeof(float), 1, fp) == 1);
        assert(walktime >= 0.0f); // FIXME, should be >, no?
        add_walkhop(dest_id, walktime);
    }
}


TripStop::TripStop(int32_t _id, Type _type, float _lat, float _lng) 
{
    id = _id;
    type = _type;
    lat = _lat;
    lng = _lng;

    // assume that we do want to a tdict if we're allocating a new tripstop
    tdict = shared_ptr<ServiceDict>(new ServiceDict);
}


void TripStop::write(FILE *fp)
{
    assert(fwrite(&id, sizeof(int32_t), 1, fp) == 1);
    assert(fwrite(&type, sizeof(Type), 1, fp) == 1);
    assert(fwrite(&lat, sizeof(float), 1, fp) == 1);
    assert(fwrite(&lng, sizeof(float), 1, fp) == 1);

    
    uint8_t have_triphops = tdict ? 1 : 0;
    assert(fwrite(&have_triphops, sizeof(uint8_t), 1, fp) == 1);

    if (tdict)
    {
        uint32_t num_service_periods = tdict->size();
        assert(fwrite(&num_service_periods, sizeof(uint32_t), 1, fp) == 1);

        for (ServiceDict::iterator i = tdict->begin(); i != tdict->end(); i++)
        {
            char service_period[MAX_ID_LEN];
            strncpy(service_period, i->first.c_str(), MAX_ID_LEN);
            assert(fwrite(service_period, sizeof(char), MAX_ID_LEN, fp) == MAX_ID_LEN);
            uint32_t num_route_ids = i->second.size();
            assert(fwrite(&num_route_ids, sizeof(uint32_t), 1, fp) == 1);
            for (TripHopDict::iterator j = i->second.begin();
                 j != i->second.end(); j++)
            {
                int32_t route_id = j->first;
                assert(fwrite(&route_id, sizeof(int32_t), 1, fp) == 1);
                uint32_t num_triphops = j->second.size();
                assert(fwrite(&num_triphops, sizeof(uint32_t), 1, fp) == 1);
                for (TripHopList::iterator k = j->second.begin();
                     k != j->second.end(); k++)
                {
                    shared_ptr<TripHop> t = (*k);
                    assert(fwrite(k->get(), sizeof(TripHop), 1, fp) == 1);
                }
            }
        }
    }

    uint32_t num_walkhops = wlist.size();
    assert(fwrite(&num_walkhops, sizeof(uint32_t), 1, fp) == 1);
    for (WalkHopList::iterator i = wlist.begin(); i != wlist.end(); i++)
    {
        assert(fwrite(&(*i), sizeof(WalkHop), 1, fp) == 1);
    }
}


static bool sort_triphops(const shared_ptr<TripHop> &x, 
                          const shared_ptr<TripHop> &y)
{
    return x->start_time < y->start_time;
}


void TripStop::add_triphop(int32_t start_time, int32_t end_time, 
                           int32_t dest_id, int32_t route_id, int32_t trip_id,
                           string service_id)
{
    if (!tdict)
        tdict = shared_ptr<ServiceDict>(new ServiceDict);
    
    (*tdict)[service_id][route_id].push_back(shared_ptr<TripHop>(
                                              new TripHop(start_time, 
                                                          end_time, dest_id,
                                                          trip_id)));
    ::sort((*tdict)[service_id][route_id].begin(), 
           (*tdict)[service_id][route_id].end(), sort_triphops);
}


void TripStop::add_walkhop(int32_t dest_id, float walktime)
{
    wlist.push_front(WalkHop(dest_id, walktime));
}


shared_ptr<TripHop> TripStop::find_triphop(int time, int route_id, 
                                           string service_id)
{
    if (tdict) 
    {
        for (TripHopList::iterator i = (*tdict)[service_id][route_id].begin();
             i != (*tdict)[service_id][route_id].end(); i++)
        {
            if ((*i)->start_time >= time)
                return (*i);
        }
    }

    return shared_ptr<TripHop>();
}


vector<TripHop> TripStop::find_triphops(int time, int route_id,
                                        string service_id, 
                                        int num)
{
    vector<TripHop> tlist;

    if (tdict) 
    {
        for (TripHopList::iterator i = (*tdict)[service_id][route_id].begin(); 
             (i != ((*tdict)[service_id][route_id].end()) && tlist.size() < num); 
             i++)
        {
            if ((*i)->start_time >= time)
                tlist.push_back(*(*i));
        }
    }

    return tlist;
}


list<int> TripStop::get_routes(std::string service_id)
{
    list<int> routes;

    if (tdict) 
    {
        for (TripHopDict::iterator i = (*tdict)[service_id].begin();
             i != (*tdict)[service_id].end(); i++)
        {
            routes.push_back(i->first);
        }
    }

    return routes;
}

