#include "wvtest.h"
#include "tripstop.h"

using namespace std;
using namespace boost;


WVTEST_MAIN("get_multiple_triphops")
{
    TripStop t;
    t.add_triphop(500, 550, 0, 0, 0, "caturday");
    t.add_triphop(550, 600, 0, 0, 0, "caturday");
    t.add_triphop(600, 650, 0, 0, 0, "caturday");
    t.add_triphop(600, 650, 0, 0, 0, "sunday");

    // Ask for three
    vector<TripHop> v = t.find_triphops(499, 0, "caturday", 
                                                     3);
    WVPASSEQ(v.size(), 3);
    WVPASSEQ(v[0].start_time, 500);
    WVPASSEQ(v[1].start_time, 550);
    WVPASSEQ(v[2].start_time, 600);

    v = t.find_triphops(499, 0, "caturday", 2);
    WVPASSEQ(v.size(), 2);
    WVPASSEQ(v[0].start_time, 500);
    WVPASSEQ(v[1].start_time, 550);

    v = t.find_triphops(499, 0, "caturday", 4);
    WVPASSEQ(v.size(), 3);
    WVPASSEQ(v[0].start_time, 500);
    WVPASSEQ(v[1].start_time, 550);
    WVPASSEQ(v[2].start_time, 600);

    v = t.find_triphops(551, 0, "caturday", 2);
    WVPASSEQ(v.size(), 1);
    WVPASSEQ(v[0].start_time, 600);

}
