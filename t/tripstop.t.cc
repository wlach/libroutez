#include "wvtest.h"
#include "tripstop.h"

using namespace std;
using namespace tr1;


WVTEST_MAIN("save/load")
{
    TripStop t1(1, TripStop::OSM, 44.5f, 54.4f);
    t1.add_triphop(500, 550, 0, 0, 0, 0);
    t1.add_triphop(550, 600, 0, 0, 0, 0);

    char *tmpname = tmpnam(NULL); // security issues in unit tests? bah.
    unlink(tmpname);
    FILE *fp1 = fopen(tmpname, "w");
    t1.write(fp1);
    fclose(fp1);

    FILE *fp2 = fopen(tmpname, "r");
    TripStop t2(fp2);

    WVPASSEQ(t2.id, t1.id);
    WVPASSEQ(t2.type, t1.type);
    WVPASSEQ(t2.lat, t1.lat);
    WVPASSEQ(t2.lng, t1.lng);
    shared_ptr<TripStop::ServiceDict> tdict = t2.tdict;
    WVPASSEQ(tdict->size(), 1);
    WVPASSEQ(((*tdict))[0].size(), 1);
    WVPASSEQ(((*tdict))[0][0].size(), 2);
    WVPASSEQ(((*tdict))[0][0][0].start_time, 500);
    WVPASSEQ(((*tdict))[0][0][1].start_time, 550);

    fclose(fp2);
}


WVTEST_MAIN("get_multiple_triphops")
{
    TripStop t;
    t.add_triphop(500, 550, 0, 0, 0, 0);
    t.add_triphop(550, 600, 0, 0, 0, 0);
    t.add_triphop(600, 650, 0, 0, 0, 0);
    t.add_triphop(600, 650, 0, 0, 0, 1);

    // Ask for different amounts...

    vector<TripHop> v = t.find_triphops(499, 0, 0, 3);
    WVPASSEQ(v.size(), 3);
    WVPASSEQ(v[0].start_time, 500);
    WVPASSEQ(v[1].start_time, 550);
    WVPASSEQ(v[2].start_time, 600);

    v = t.find_triphops(499, 0, 0, 2);
    WVPASSEQ(v.size(), 2);
    WVPASSEQ(v[0].start_time, 500);
    WVPASSEQ(v[1].start_time, 550);

    v = t.find_triphops(499, 0, 0, 4);
    WVPASSEQ(v.size(), 3);
    WVPASSEQ(v[0].start_time, 500);
    WVPASSEQ(v[1].start_time, 550);
    WVPASSEQ(v[2].start_time, 600);

    v = t.find_triphops(551, 0, 0, 2);
    WVPASSEQ(v.size(), 1);
    WVPASSEQ(v[0].start_time, 600);

}
