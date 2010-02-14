#ifndef __TRIPPATH_H
#define __TRIPPATH_H
#include <tr1/unordered_set>
#include <tr1/memory>
#include <deque>
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
    std::tr1::shared_ptr<TripAction> parent;
};


struct TripPath
{
  public:
    TripPath(double _time, double _fastest_speed, 
             std::tr1::shared_ptr<TripStop> &_dest_stop, 
             std::tr1::shared_ptr<TripStop> &_last_stop);
    TripPath() {}

    std::tr1::shared_ptr<TripPath> add_action(
        std::tr1::shared_ptr<TripAction> &action, 
        std::deque<int> &_possible_route_ids,
        std::tr1::shared_ptr<TripStop> &_last_stop);

    // the following are mostly for the benefit of language bindings
    // C++ code should be able to access this directly with less overhead...
    std::deque<TripAction> get_actions();
    //tr1python::object get_last_action();

    double time;
    double fastest_speed;
    std::tr1::shared_ptr<TripStop> dest_stop;
    std::tr1::shared_ptr<TripStop> last_stop;
    std::tr1::shared_ptr<TripAction> last_action;

    double walking_time;
    double route_time;
    int traversed_route_ids;
    std::tr1::unordered_set<int> possible_route_ids;
    int last_route_id;
    double weight;
    double heuristic_weight;

private:
    void _get_heuristic_weight();

    // Given an action just after the end of a walk in the path, delays
    // that walk by the given number of seconds.
    void delay_walk(std::tr1::shared_ptr<TripAction> walk, float secs);
};

#endif // __TRIPPATH_H
