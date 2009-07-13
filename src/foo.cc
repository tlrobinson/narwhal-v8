#include <narwhal.h>

FUNCTION(Bar)
    printf("bar, via printf!\n");
    print("bar, via js!\n");
    
    Handle<Object> narwhal = require("narwhal");
    Handle<String> string = Handle<String>::Cast(OBJECT_GET(narwhal, "LEFT"));
    
    Handle<Value> argv[1] = { string };
    Print->Call(JS_GLOBAL, 1, argv);
    
    return JS_undefined;
END

NARWHAL_MODULE(foo)

    OBJECT_SET(Exports, "bar", JS_fn(Bar));
    
END_NARWHAL_MODULE
