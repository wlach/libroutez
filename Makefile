include config.mk

default:  libroutez.so examples/testgraph \
	python/libroutez/tripgraph.py python/libroutez/_tripgraph.so \
	ruby/routez.so

include install.mk

# Always always compile with fPIC
CFLAGS += -fPIC
CXXFLAGS += -fPIC

# libroutez should be compiled as a shared library by default
ifeq (${OS},MACOS)
	LDFLAGS += -dynamiclib
else
	LDFLAGS += -shared
endif

config.mk:
	@echo "Please run ./configure. Stop."
	@exit 1

%.o: %.cc 
	g++ $< -c -o $@ $(CXXFLAGS) $(PYTHON_CFLAGS) $(RUBY_CFLAGS) -D WVTEST_CONFIGURED -I./include -I./wvtest/cpp -g
	@g++ $< -MM $(CXXFLAGS) $(PYTHON_CFLAGS) $(RUBY_CFLAGS) -D WVTEST_CONFIGURED -I./include -I./wvtest/cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

TRIPGRAPH_OBJECTS=lib/tripgraph.o lib/trippath.o lib/tripstop.o lib/serviceperiod.o

# libroutez: the main library
libroutez.so: $(TRIPGRAPH_OBJECTS)
	g++ $(TRIPGRAPH_OBJECTS) $(LDFLAGS) -o libroutez.so -fPIC -g

# python bindings
python/libroutez/tripgraph.py python/libroutez/tripgraph_wrap_py.cc: tripgraph.i
	swig -classic -c++ -python -I./include -outdir python/libroutez -o python/libroutez/tripgraph_wrap_py.cc $<
python/libroutez/_tripgraph.so: libroutez.so python/libroutez/tripgraph_wrap_py.o
	g++ -o $@ python/libroutez/tripgraph_wrap_py.o libroutez.so $(LDFLAGS) $(PYTHON_LDFLAGS) -fPIC

# ruby bindings
ruby/routez_wrap_rb.cc: routez.i tripgraph.i 
	swig -c++ -ruby -I./include  -o $@ $<

ruby/routez.so: libroutez.so ruby/routez_wrap_rb.o
	g++ -o ruby/routez.so ruby/routez_wrap_rb.o libroutez.so $(LDFLAGS) $(RUBY_LDFLAGS) -I./include -fPIC

# stupid test programs
examples/loadgraph: examples/loadgraph.o libroutez.so
	g++ $< -o $@ libroutez.so -fPIC -g -I./include
examples/testgraph: examples/testgraph.o libroutez.so
	g++ $< -o $@ libroutez.so -fPIC -g -I./include

# unit test suite
TEST_OBJS=t/tripgraph.t.o t/tripstop.t.o t/all.t.o
WVTEST_OBJS=wvtest/cpp/wvtest.o wvtest/cpp/wvtestmain.o
t/all.t: $(TEST_OBJS) $(WVTEST_OBJS) libroutez.so
	g++ $(TEST_OBJS) $(WVTEST_OBJS) -o $@ libroutez.so -fPIC -g

.PHONY: test test-cpp test-python
test: test-cpp test-python

test-cpp: t/all.t
	LD_LIBRARY_PATH=$(PWD) valgrind --tool=memcheck wvtest/wvtestrun t/all.t

test-python: python/libroutez/tripgraph.py python/libroutez/_tripgraph.so
	LD_LIBRARY_PATH=$(PWD) PYTHONPATH=$(PWD)/t:$(PWD)/python:$(PYTHONPATH) wvtest/wvtestrun python wvtest/python/wvtestmain.py pytest.py

clean:
	rm -f *.so lib/*.o python/*.pyc */*.pyc examples/testgraph \
	python/libroutez/_tripgraph.so python/libroutez/tripgraph.py \
	python/libroutez/tripgraph_wrap_py.cc python/libroutez/*.o \
	ruby/routez.so ruby/*.o ruby/routez_wrap_rb.cc \
	wvtest/cpp/*.o wvtest/cpp/*.d \
	t/*.o t/*.d t/all.t *.d

-include $(TRIPGRAPH_OBJECTS:.o=.d)
