CPP       =g++
CPPFLAGS  =-g #-save-temps
INCLUDES  =-Iv8/include -Iinclude
LIBS      =-lreadline -L/usr/lib -liconv
MODULES   =$(patsubst %.cc,%.dylib,$(patsubst src/%,lib/%,$(wildcard src/*.cc)))

# change to libv8_g.dylib for debug version:
V8NAME    =libv8.dylib
#V8NAME    =libv8_g.dylib
V8        =v8/$(V8NAME)

all: $(V8) bin/narwhal-v8 modules

modules: $(MODULES)

bin/narwhal-v8: narwhal-v8.cc $(V8)
	mkdir -p bin
	$(CPP) $(CPPFLAGS) $(INCLUDES) -o $@ narwhal-v8.cc $(V8) $(LIBS)
	# comment out these lines to use installed system V8
	install_name_tool -change "$(V8NAME)" "@executable_path/$(V8NAME)" $@
	cp $(V8) bin/$(V8NAME)

lib/%.dylib: src/%.cc
	mkdir -p lib
	$(CPP) $(CPPFLAGS) $(INCLUDES) -dynamiclib -o $@ $< $(V8) $(LIBS)
	# comment out these lines to use installed system V8
	install_name_tool -change "$(V8NAME)" "@executable_path/$(V8NAME)" $@

v8/libv8.dylib: v8
	cd v8 && scons library=shared

v8/libv8_g.dylib: v8
	cd v8 && scons library=shared mode=debug

v8:
	svn checkout http://v8.googlecode.com/svn/trunk/ v8

clean:
	rm -rf bin/narwhal-v8 bin/*.dylib bin/*.dSYM lib/*.dylib lib/*.dSYM *.o *.ii *.s

cleaner: clean
	rm -rf v8/$(V8NAME)

pristine: clean
	rm -rf v8
