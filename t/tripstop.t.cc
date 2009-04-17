#include <boost/test/unit_test.hpp>
#include "tripstop.h"

using namespace std;
using namespace boost;


BOOST_AUTO_TEST_CASE(get_multiple_triphops)
{
    TripStop t;
    t.add_triphop(500, 550, 0, 0, 0, "caturday");
    t.add_triphop(550, 600, 0, 0, 0, "caturday");
    t.add_triphop(600, 650, 0, 0, 0, "caturday");
    t.add_triphop(600, 650, 0, 0, 0, "sunday");

    // Ask for three
    vector<TripHop> v = t.find_triphops(499, 0, "caturday", 
                                                     3);
    BOOST_CHECK_EQUAL(v.size(), 3);
    BOOST_CHECK_EQUAL(v[0].start_time, 500);
    BOOST_CHECK_EQUAL(v[1].start_time, 550);
    BOOST_CHECK_EQUAL(v[2].start_time, 600);

    v = t.find_triphops(499, 0, "caturday", 2);
    BOOST_CHECK_EQUAL(v.size(), 2);
    BOOST_CHECK_EQUAL(v[0].start_time, 500);
    BOOST_CHECK_EQUAL(v[1].start_time, 550);

    v = t.find_triphops(499, 0, "caturday", 4);
    BOOST_CHECK_EQUAL(v.size(), 3);
    BOOST_CHECK_EQUAL(v[0].start_time, 500);
    BOOST_CHECK_EQUAL(v[1].start_time, 550);
    BOOST_CHECK_EQUAL(v[2].start_time, 600);

    v = t.find_triphops(551, 0, "caturday", 2);
    BOOST_CHECK_EQUAL(v.size(), 1);
    BOOST_CHECK_EQUAL(v[0].start_time, 600);

}
