#include <narwhal.h>
#include <stdlib.h>

FUNCTION(Exit)
    if (ARGC == 0)
    {
        exit(0);
    }
    else if (ARGC == 1)
    {
        ARG_int(code, 0);
        exit(code);
    }
    return ThrowException(String::New("os.exit() takes 0 or 1 arguments."));
END

NARWHAL_MODULE(os_platform)

    OBJECT_SET(Exports, "exit", JS_fn(Exit));
    
END_NARWHAL_MODULE
