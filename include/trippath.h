#ifndef __TRIPPATH_H
#define __TRIPPATH_H
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include "tripstop.h"


struct TripAction
{
    TripAction(int32_t _src_id, int32_t _dest_id, int _route_id, 
               double _start_time, double _end_time);
    TripAction() {} // for swig, which wants to call resize for some dumb reason
    TripAction(const TripAction &other);
    ~TripAction() { }

    TripAction &operator=(const TripAction &other);

    int32_t src_id, dest_id;
    double start_time, end_time;
    int route_id;

    // pointer to the action which preceded this one
    boost::shared_ptr<TripAction> parent;
};


struct TripPath
{
  public:
    TripPath(double _time, double _fastest_speed, 
             boost::shared_ptr<TripStop> &_dest_stop, 
             boost::shared_ptr<TripStop> &_last_stop);
    TripPath() {}

    boost::shared_ptr<TripPath> add_action(
        boost::shared_ptr<TripAction> &action, 
        std::list<int> &_possible_route_ids,
        boost::shared_ptr<TripStop> &_last_stop);

    // the following are mostly for the benefit of language bindings
    // C++ code should be able to access this directly with less overhead...
    std::list<TripAction> get_actions();
    //boostpython::object get_last_action();

    double time;
    double fastest_speed;
    boost::shared_ptr<TripStop> dest_stop;
    boost::shared_ptr<TripStop> last_stop;
    boost::shared_ptr<TripAction> last_action;

    double walking_time;
    double route_time;
    int traversed_route_ids;
    boost::unordered_set<int> possible_route_ids;
    int last_route_id;
    double weight;
    double heuristic_weight;

private:
    void _get_heuristic_weight();

    // Given an action just after the end of a walk in the path, delays
    // that walk by the given number of seconds.
    void delay_walk(boost::shared_ptr<TripAction> walk, float secs);
};

#endif // __TRIPPATH_H
