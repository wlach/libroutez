include config.mk

# Always always compile with fPIC
CFLAGS += -fPIC
CXXFLAGS += -fPIC

default: python/tripgraph.py python/_tripgraph.so libroutez.so \
	examples/testgraph

config.mk:
	@echo "Please run ./configure. Stop."
	@exit 1

%.o: %.cc
	g++ $< -c -o $@ $(CXXFLAGS) $(PYTHON_CFLAGS) $(RUBY_CFLAGS) -I./include -g

%.o: %.cc %.h
	g++ $< -c -o $@ $(CXXFLAGS) $(PYTHON_CFLAGS) $(RUBY_CFLAGS) -I./include -g

TRIPGRAPH_OBJECTS=lib/tripgraph.o lib/trippath.o lib/tripstop.o 

# libroutez: the main library
libroutez.so: $(TRIPGRAPH_OBJECTS)
	g++ $(TRIPGRAPH_OBJECTS) -shared -o libroutez.so -fPIC -g

# python bindings
python/tripgraph.py python/tripgraph_wrap_py.cc: tripgraph.i
	swig -c++ -python -I./include -o python/tripgraph_wrap_py.cc $<
python/_tripgraph.so: libroutez.so python/tripgraph_wrap_py.o
	g++ -shared -o $@ python/tripgraph_wrap_py.o libroutez.so $(PYTHON_LDFLAGS) -fPIC

# tripgraph_rb.so: the ruby library

#tripgraph_wrap_rb.cc: tripgraph.i
# commented out until I can get seperate things out
#	swig -c++ -ruby -o tripgraph_wrap_rb.cc $<
#tripgraph.so: libroutez.so tripgraph_wrap_rb.o
#	g++ -shared -o tripgraph.so tripgraph_wrap_rb.o libroutez.so $(RUBY_LDFLAGS) -fPIC

# stupid test program
examples/testgraph: examples/testgraph.cc libroutez.so
	g++ $< -o $@ libroutez.so -fPIC -g -I./include

clean:
	rm -f lib/*.o examples/testgraph \
	python/_tripgraph.so python/tripgraph.py python/tripgraph_wrap_py.cc 