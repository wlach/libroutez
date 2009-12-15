%module tripgraph

%{
#include "serviceperiod.h"
#include "tripgraph.h"
#include "trippath.h"
#include "tripstop.h"
%}

%include "std_string.i"
%include "std_deque.i"
%include "std_vector.i"
%include "std_pair.i"
%include "inttypes.i"
%template(ListTripAction) std::deque<TripAction>;
%template(ListId) std::deque<int>;
%template(ListTripHop) std::vector<TripHop>;
%template(ListTripStop) std::vector<TripStop>;
%template(ServicePeriodTuple) std::pair<int, int>;
%template(ListServicePeriodTuple) std::vector<std::pair<int, int> >;
%include "serviceperiod.h"
%include "tripgraph.h"
%include "trippath.h"
%include "tripstop.h"
