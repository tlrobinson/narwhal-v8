#include <v8.h>
#include "../macros.h"

#define MODULE_NAME foo

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

const char *moduleName = STRINGIZE(MODULE_NAME);
extern "C" const char * getModuleName() { return moduleName; }

Persistent<Function> require_JS;
Persistent<Object>   exports;
Persistent<Object>   module;
Persistent<Object>   sys;
Persistent<Function> print_JS;

void print(const char * string)
{
    Handle<Value> argv[1] = { JS_str(string) };
    print_JS->Call(JS_GLOBAL, 1, argv);
}

Handle<Object> require(const char *id) {
     Handle<Value> argv[1] = { JS_str(id) };
     return Handle<Object>::Cast(require_JS->Call(JS_GLOBAL, 1, argv));
}

FUNCTION(bar)
    printf("bar!\n");
    print("bar, via js!");
    return JS_undefined;
END

extern "C" Handle<Value> MODULE_NAME(const Arguments& args)
{
    ARG_COUNT(5);
    require_JS  = Persistent<Function>::New(Handle<Function>::Cast(args[0]));
    exports     = Persistent<Object>::New(Handle<Object>::Cast(args[1]));
    module      = Persistent<Object>::New(Handle<Object>::Cast(args[2]));
    sys         = Persistent<Object>::New(Handle<Object>::Cast(args[3]));
    print_JS    = Persistent<Function>::New(Handle<Function>::Cast(args[4]));
    
    print("In module factory, via js!");
    
    Handle<Object> baz = require("baz");
    Handle<String> string = Handle<String>::Cast(OBJECT_GET(baz, "buzz"));
    
    Handle<Value> argv[1] = { string };
    print_JS->Call(JS_GLOBAL, 1, argv);
    
    OBJECT_SET(exports, "bar", JS_fn(bar));
    
    return JS_undefined;
}
