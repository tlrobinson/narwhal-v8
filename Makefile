CPP       =g++
CPPFLAGS  =
INCLUDES  =-Iv8/include -Iinclude
V8        =v8/libv8.dylib
LIBS      =-lreadline
MODULES   =$(patsubst %.cc,%.dylib,$(patsubst src/%,lib/%,$(wildcard src/*.cc)))


all: v8/libv8.dylib bin/narwhal-v8 modules

modules: $(MODULES)

bin/narwhal-v8: narwhal-v8.cc $(V8)
	mkdir -p bin
	$(CPP) $(CPPFLAGS) $(INCLUDES) -o $@ narwhal-v8.cc $(V8) $(LIBS)

lib/%.dylib: src/%.cc
	mkdir -p lib
	$(CPP) $(CPPFLAGS) $(INCLUDES) -dynamiclib -o $@ $< $(V8)

v8/libv8.dylib: v8
	cd v8 && scons library=shared

v8:
	svn checkout http://v8.googlecode.com/svn/trunk/ v8

clean:
	rm -rf bin/narwhal-v8 bin/*.dSYM lib/*.dylib lib/*.dSYM

cleaner: clean
	rm -rf v8/libv8.dylib

pristine: clean
	rm -rf v8
