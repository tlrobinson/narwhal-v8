#ifndef __NARWHAL__
#define __NARWHAL__

#include <v8.h>
#include "k7macros.h"

// declare the module's 5 free variables and the C versions of "print" and "require" so they're available to the whole file
extern v8::Persistent<v8::Function> Require;
extern v8::Persistent<v8::Object>   Exports;
extern v8::Persistent<v8::Object>   Module;
extern v8::Persistent<v8::Object>   System;
extern v8::Persistent<v8::Function> Print;

extern void print(const char * string);
extern v8::Handle<v8::Object> require(const char * id);

// MACROS:

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define NARWHAL_MODULE(MODULE_NAME) \
    v8::Persistent<v8::Function> Require;\
    v8::Persistent<v8::Object>   Exports;\
    v8::Persistent<v8::Object>   Module;\
    v8::Persistent<v8::Object>   System;\
    v8::Persistent<v8::Function> Print;\
    \
    void print(const char * string)\
    {\
        v8::Handle<v8::Value> argv[1] = { JS_str(string) };\
        Print->Call(JS_GLOBAL, 1, argv);\
    }\
    \
    v8::Handle<v8::Object> require(const char *id) {\
         v8::Handle<v8::Value> argv[1] = { JS_str(id) };\
         return v8::Handle<v8::Object>::Cast(Require->Call(JS_GLOBAL, 1, argv));\
    }\
    \
    const char *moduleName = STRINGIZE(MODULE_NAME);\
    extern "C" const char * getModuleName() { return moduleName; }\
    \
    extern "C" Handle<Value> MODULE_NAME(const Arguments& args)\
    {\
        ARG_COUNT(5);\
        Require  = Persistent<Function>::New(Handle<Function>::Cast(args[0]));\
        Exports  = Persistent<Object>::New(Handle<Object>::Cast(args[1]));\
        Module   = Persistent<Object>::New(Handle<Object>::Cast(args[2]));\
        System   = Persistent<Object>::New(Handle<Object>::Cast(args[3]));\
        Print    = Persistent<Function>::New(Handle<Function>::Cast(args[4]));\

#define END_NARWHAL_MODULE \
    return JS_undefined;}\

#endif