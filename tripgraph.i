%module tripgraph

%{
#include "tripgraph.h"
#include "trippath.h"
#include "tripstop.h"
%}

%include "std_string.i"
%include "std_list.i"
%include "inttypes.i"
%template(ListTripAction) std::list<TripAction>;
%include "tripgraph.h"
%include "trippath.h"
%include "tripstop.h"
