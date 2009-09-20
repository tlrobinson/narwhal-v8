#ifndef __NARWHAL__
#define __NARWHAL__

#include <v8.h>
#include "k7macros.h"

// declare the module's 5 free variables and the C versions of "print" and "require" so they're available to the whole file
/*extern v8::Persistent<v8::Function> Require;
extern v8::Persistent<v8::Object>   Exports;
extern v8::Persistent<v8::Object>   Module;
extern v8::Persistent<v8::Object>   System;
extern v8::Persistent<v8::Function> Print;

extern void print(const char * string);
extern v8::Handle<v8::Object> require(const char * id);
*/

// MACROS:

#define NARWHAL_V8 1

#define NOTRACE

#ifdef NOTRACE
#define TRACE(...)
#else
#define TRACE(...) fprintf(stderr, __VA_ARGS__);
#endif

#define DEBUG(...)
//#define DEBUG(...) fprintf(stderr, __VA_ARGS__);

//#define THROW_DEBUG
#define THROW_DEBUG " (%s:%d)\n"

#define THROW(format, ...) \
    { char msg[1024]; snprintf(msg, 1024, format THROW_DEBUG, ##__VA_ARGS__, __FILE__, __LINE__); \
    DEBUG("THROWING: %s", msg); \
    return ThrowException(String::New(msg)); }


#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define THIS (args.This())

#define ARGV(n) (args[n])
#define IS_STRING(value) (value)->IsString()

#define ARGN_UTF8_CAST(variable, index) \
    if (index >= ARGC) THROW("Argument %d must be a string.", index) \
    _tmpStr = v8::Handle<v8::String>::Cast(ARGV(index)); \
    _tmpSz = _tmpStr->Utf8Length(); \
    char variable[_tmpSz+1]; \
    _tmpStr->WriteUtf8(variable);

#define ARGN_UTF8(variable, index) \
    if (index >= ARGC && !IS_STRING(ARGV(index))) THROW("Argument %d must be a string.", index) \
    ARGN_UTF8_CAST(variable, index)

#define ARGN_STR(variable, index) \
    if (index >= ARGC && !IS_STRING(ARGV(index))) THROW("Argument %d must be a string.", index) \
    v8::Handle<v8::String> variable = v8::Handle<v8::String>::Cast(ARGV(index));
    
#define ARG_UTF8(variable)      0; ARGN_UTF8(variable, _argn); _argn++; 0
#define ARG_UTF8_CAST(variable) 0; ARGN_UTF8_CAST(variable, _argn); _argn++; 0
#define ARG_STR(variable)       0; ARGN_STR(variable, _argn); _argn++; 0

#define FUNCTION(f,...) \
    v8::Handle<v8::Value> f(const v8::Arguments& args) { \
        TRACE(" *** C: %s\n", STRINGIZE(f)) \
        v8::HandleScope handlescope; \
        int _argn = 0; \
        v8::Handle<v8::String> _tmpStr; \
        size_t _tmpSz; \
        __VA_ARGS__;

#define END \
    }

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

#define EXPORTS(name, object) OBJECT_SET(Exports, name, object);

#define DESTRUCTOR(f) void f(Persistent<Value> value, void *) \
    { Local<Object> object = value->ToObject(); \

#define JSString    v8::Handle<v8::Value>

#define JS_str_utf8(str, len) v8::String::New(str, len)
#define JS_str_utf16(str, len) v8::String::New((uint16_t*)str, (len)/sizeof(uint16_t))

#define GET_UTF16(string, buffer, length) _GET_UTF16(string, (uint16_t **)buffer, length)
int _GET_UTF16(v8::Handle<v8::String> string, uint16_t **buffer, size_t *length) {
    v8::String::Value value(string);
    *buffer = *value;
    *length = value.length() * sizeof(uint16_t);

    return 1;
}

#define CALL(f, ...) f(__VA_ARGS__)

#endif