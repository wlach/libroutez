#include "tripstop.h"

using namespace boost;
using namespace std;
using namespace tr1;


static bool operator<(const TripHop& x, const TripHop& y)
{
    return x.start_time < y.start_time;
}


TripStop::TripStop(FILE *fp) 
{
    assert(fread(id, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
    assert(fread(type, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
    assert(fread(&lat, sizeof(float), 1, fp) == 1);
    assert(fread(&lng, sizeof(float), 1, fp) == 1);
        
    uint32_t num_triphops = 0;
    assert(fread(&num_triphops, sizeof(uint32_t), 1, fp) == 1);
    for (int i=0; i<num_triphops; i++)
    {
        char service_period[MAX_ID_LEN];
        int32_t route_id;
        assert(fread(service_period, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
        assert(fread(&route_id, sizeof(int32_t), 1, fp) == 1);
        TripHop *t = new TripHop;
        assert(fread(t, sizeof(TripHop), 1, fp) == 1);
        tdict[service_period][route_id].push_back(shared_ptr<TripHop>(t));
    }

    uint32_t num_walkhops = 0;
    assert(fread(&num_walkhops, sizeof(uint32_t), 1, fp) == 1);
    for (int i=0; i<num_walkhops; i++)
    {
        char dest_id[MAX_ID_LEN];
        float walktime;
        assert(fread(dest_id, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
        assert(fread(&walktime, sizeof(float), 1, fp) == 1);
        add_walkhop(dest_id, walktime);
    }
}


TripStop::TripStop(string _id, string _type, float _lat, float _lng)
{
    assert(_id.length() < MAX_ID_LEN);
    assert(_type.length() < MAX_ID_LEN);
    strcpy(id, _id.c_str());
    strcpy(type, _type.c_str());
    
    lat = _lat;
    lng = _lng;
}


void TripStop::write(FILE *fp)
{
    assert(fwrite(id, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
    assert(fwrite(type, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
    assert(fwrite(&lat, sizeof(float), 1, fp) == 1);
    assert(fwrite(&lng, sizeof(float), 1, fp) == 1);
    
    uint32_t num_triphops = 0;
    for (ServiceDict::iterator i = tdict.begin(); i != tdict.end(); i++)
    {
        for (TripHopDict::iterator j = i->second.begin(); 
             j != i->second.end(); j++)
            num_triphops += j->second.size();
    }
    
    assert(fwrite(&num_triphops, sizeof(uint32_t), 1, fp) == 1);
    for (ServiceDict::iterator i = tdict.begin(); i != tdict.end(); i++)
    {
        char service_period[MAX_ID_LEN];
        strcpy(service_period, i->first.c_str());
        for (TripHopDict::iterator j = i->second.begin(); j != i->second.end(); j++)
        {
            int32_t route_id = j->first;
            for (TripHopList::iterator k = j->second.begin(); k != j->second.end(); k++)                
            {
                shared_ptr<TripHop> t = (*k);
                assert(fwrite(&service_period, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
                assert(fwrite(&route_id, sizeof(int32_t), 1, fp) == 1);
                assert(fwrite(k->get(), sizeof(TripHop), 1, fp) == 1);
            }            
        }
    }

    uint32_t num_walkhops = wdict.size();
    assert(fwrite(&num_walkhops, sizeof(uint32_t), 1, fp) == 1);
    for (WalkHopDict::iterator i = wdict.begin(); i != wdict.end(); i++)
    {
        char dest_id[MAX_ID_LEN];
        strcpy(dest_id, i->first.c_str());

        assert(fwrite(&dest_id, 1, MAX_ID_LEN, fp) == MAX_ID_LEN);
        assert(fwrite(&i->second, sizeof(float), 1, fp) == 1);        
    }
}


static bool sort_triphops(const shared_ptr<TripHop> &x, 
                          const shared_ptr<TripHop> &y)
{
    return x->start_time < y->start_time;
}


void TripStop::add_triphop(int32_t start_time, int32_t end_time, 
                           string dest_id, int32_t route_id,
                           string service_id)
{
    tdict[service_id][route_id].push_back(shared_ptr<TripHop>(
                                              new TripHop(start_time, 
                                                          end_time, dest_id)));
    sort(tdict[service_id][route_id].begin(), tdict[service_id][route_id].end(), 
        sort_triphops);
}


void TripStop::add_walkhop(string dest_id, float walktime)
{
    wdict[dest_id] = walktime;
}


shared_ptr<TripHop> TripStop::find_triphop(int time, int route_id, 
                                           string service_id)
{
    for (TripHopList::iterator i = tdict[service_id][route_id].begin();
         i != tdict[service_id][route_id].end(); i++)
    {
        if ((*i)->start_time >= time)
            return (*i);
    }

    return shared_ptr<TripHop>();
}


unordered_set<int> TripStop::get_routes(std::string service_id)
{
    unordered_set<int> routes;

    for (TripHopDict::iterator i = tdict[service_id].begin();
         i != tdict[service_id].end(); i++)
    {
            routes.insert(i->first);
    }

    return routes;
}

