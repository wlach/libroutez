include config.mk

# Always always compile with fPIC
CFLAGS += -fPIC
CXXFLAGS += -fPIC

default: tripgraph.py _tripgraph.so libroutez.so testgraph

config.mk:
	@echo "Please run ./configure. Stop."
	@exit 1

%.o: %.cc
	g++ $< -c -o $@ $(CXXFLAGS) $(PYTHON_CFLAGS) $(RUBY_CFLAGS) -g

%.o: %.cc %.h
	g++ $< -c -o $@ $(CXXFLAGS) $(PYTHON_CFLAGS) $(RUBY_CFLAGS) -g

TRIPGRAPH_OBJECTS=tripgraph.o trippath.o tripstop.o 

# libroutez: the main library
libroutez.so: $(TRIPGRAPH_OBJECTS)
	g++ $(TRIPGRAPH_OBJECTS) -shared -o libroutez.so -fPIC -g

# tripgraph_py.py/_tripgraph_py.so: the python library
# FIXME: rename to routez/_routez.so when we move this guy out of the
# main django directory
tripgraph.py tripgraph_wrap_py.cc: tripgraph.i
	swig -c++ -python -o tripgraph_wrap_py.cc $<
_tripgraph.so: libroutez.so tripgraph_wrap_py.o
	g++ -shared -o _tripgraph.so tripgraph_wrap_py.o libroutez.so $(PYTHON_LDFLAGS) -fPIC

# tripgraph_rb.so: the ruby library

#tripgraph_wrap_rb.cc: tripgraph.i
# commented out until I can get seperate things out
#	swig -c++ -ruby -o tripgraph_wrap_rb.cc $<
#tripgraph.so: libroutez.so tripgraph_wrap_rb.o
#	g++ -shared -o tripgraph.so tripgraph_wrap_rb.o libroutez.so $(RUBY_LDFLAGS) -fPIC

# stupid test program
testgraph: testgraph.cc libroutez.so
	g++ testgraph.cc -o testgraph libroutez.so -fPIC -g

clean:
	rm -f _tripgraph.so *.o testgraph tripgraph.py tripgraph_wrap_py.cc *~