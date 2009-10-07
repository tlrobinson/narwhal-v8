#ifndef __NARWHAL__
#define __NARWHAL__

#include <v8.h>
#include "k7macros.h"

#include <stdlib.h>
#include <string.h>


// declare the module's 5 free variables and the C versions of "print" and "require" so they're available to the whole file
///*
extern v8::Persistent<v8::Function> Require;
extern v8::Persistent<v8::Object>   Exports;
extern v8::Persistent<v8::Object>   Module;
extern v8::Persistent<v8::Object>   System;
extern v8::Persistent<v8::Function> Print;

extern void print(const char * string);
extern v8::Handle<v8::Object> require(const char * id);
//*/

// MACROS:

#define NARWHAL_V8 1

//#define DEBUG_ON
#ifdef DEBUG_ON
#define DEBUG(...) LOG(__VA_ARGS__)
#define THROW_DEBUG " (%s:%d)\n"
#else
#define NOTRACE
#define DEBUG(...)
#define THROW_DEBUG
#endif

// "#define NOTRACE" at the top of files you don't want to enable tracing on
#ifdef NOTRACE
#define TRACE(...)
#else
#define TRACE(...) LOG(__VA_ARGS__);
#endif

#define ERROR(...) LOG(__VA_ARGS__)
#define LOG(...) fprintf(stderr, __VA_ARGS__); fflush(stderr);

#define THROW(format, ...) \
    { char msg[1024]; snprintf(msg, 1024, format THROW_DEBUG, ##__VA_ARGS__, __FILE__, __LINE__); \
    DEBUG("THROWING: %s", msg); \
    return ThrowException(String::New(msg)); }


#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define THIS (args.This())

#define ARGV(n) (args[n])

#define IS_STRING(value)    (value)->IsString()
#define IS_NUMBER(value)    (value)->IsNumber()
#define IS_NULL(value)      (value->IsNull())

#define TO_STRING(value)    (value->ToString())

#define GET_UTF8(variable, value) \
    _tmpStr = value->ToString(); \
    _tmpSz = _tmpStr->Utf8Length(); \
    char variable[_tmpSz+1]; \
    _tmpStr->WriteUtf8(variable);
    
#define ARGN_UTF8_CAST(variable, index) \
    if (index >= ARGC) THROW("Argument %d must be a string.", index) \
    GET_UTF8(variable, ARGV(index));

#define ARGN_UTF8(variable, index) \
    if (index >= ARGC && !IS_STRING(ARGV(index))) THROW("Argument %d must be a string.", index) \
    ARGN_UTF8_CAST(variable, index)

#define ARGN_STR(variable, index) \
    if (index >= ARGC && !IS_STRING(ARGV(index))) THROW("Argument %d must be a string.", index) \
    v8::Handle<v8::String> variable = v8::Handle<v8::String>::Cast(ARGV(index));

#define ARGN_DOUBLE(variable, index) \
    if (index >= ARGC && !IS_NUMBER(ARGV(index))) THROW("Argument %d must be a number.", index) \
    double variable = (args[index]->NumberValue())

#define ARGN_STR_OR_NULL(variable, index) \
    NWValue variable = ((index < ARGC) && IS_STRING(ARGV(index))) ? ARGV(index) : JS_null;

#define ARG_UTF8(variable)      0; ARGN_UTF8(variable, _argn); _argn++; 0
#define ARG_UTF8_CAST(variable) 0; ARGN_UTF8_CAST(variable, _argn); _argn++; 0
#define ARG_STR(variable)       0; ARGN_STR(variable, _argn); _argn++; 0
#define ARG_DOUBLE(variable)    0; ARGN_DOUBLE(variable, _argn); _argn++; 0

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

#define NWValue     v8::Handle<v8::Value>
#define NWObject    v8::Handle<v8::Object>
#define NWString    v8::Handle<v8::String>
#define NWDate      v8::Handle<v8::Date>
#define NWArray     v8::Handle<v8::Array>

typedef uint16_t NWChar;

#define ARGS_ARRAY(name, ...) Handle<Value> name[] = { __VA_ARGS__ };

#define CALL_AS_FUNCTION(object, thisObject, argc, argv) \
    Function::Cast(*object)->Call(thisObject, argc, argv)

#define CALL_AS_CONSTRUCTOR(object, argc, argv) \
    Function::Cast(*object)->NewInstance(argc, argv)

#define JS_str_utf8(str, len) v8::String::New(str, len)
#define JS_str_utf16(str, len) v8::String::New((uint16_t*)str, (len)/sizeof(uint16_t))

#define GET_UTF16(string, buffer, length) _GET_UTF16(string, (uint16_t **)buffer, length)
int _GET_UTF16(v8::Handle<v8::String> string, uint16_t **buffer, size_t *length) {
    v8::String::Value value(string);
    *length = value.length() * sizeof(uint16_t);
    //*buffer = *value;
    
    *buffer = (uint16_t *)malloc(*length);
    memcpy(*buffer, *value, *length);

    return 1;
}

NWValue JS_date(int ms) {
    return v8::Date::New(ms);
}

NWObject JS_array(size_t argc, NWValue argv[]) {
    return v8::Array::New(argc >= 0 ? argc : 0);
}

#define CALL(f, ...) f(__VA_ARGS__)

#define SET_VALUE(object,s,v)    object->Set(JS_str(s), v)
#define GET_VALUE(object,s)      object->Get(JS_str(s))
#define SET_OBJECT(object,s,v)   object->Set(JS_str(s), v)
#define GET_OBJECT(object,s)     object->Get(JS_str(s))->ToObject()

#define GET_INT(object, name)   GET_VALUE(object, name)->IntegerValue()

void SET_VALUE_AT_INDEX(v8::Handle<v8::Object> object, int index, v8::Handle<v8::Value> value) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%d", index);
    SET_OBJECT(object, buf, value);
}

#define CLASS(NAME) NAME ## _class()

#define CONSTRUCTOR FUNCTION

#define PROTECT_OBJECT(value) Persistent<Object>::New(value)

#define HANDLE_EXCEPTION(a, b)

#define TO_OBJECT(object) (object->ToObject())
#define GET_BOOL(object, name ) (GET_VALUE(object, name)->BooleanValue())

#endif