%module tripgraph

%{
#include "tripgraph.h"
#include "trippath.h"
#include "tripstop.h"
%}

%include "std_string.i"
%include "std_list.i"
%include "std_vector.i"
%include "inttypes.i"
%template(ListTripAction) std::list<TripAction>;
%template(ListId) std::list<int>;
%template(ListTripHop) std::vector<TripHop>;
%include "tripgraph.h"
%include "trippath.h"
%include "tripstop.h"
