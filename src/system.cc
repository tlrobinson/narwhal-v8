#include <narwhal.h>
#include <dlfcn.h>
#include <string.h>

NARWHAL_MODULE(system_)

    Handle<Object> system_default = require("system.js");

    EXPORTS("stdin", OBJECT_GET(system_default, "stdin"));
    EXPORTS("stdout", OBJECT_GET(system_default, "stdout"));
    EXPORTS("stderr", OBJECT_GET(system_default, "stderr"));
    EXPORTS("fs",    OBJECT_GET(system_default, "fs"));
    EXPORTS("log",   OBJECT_GET(system_default, "log"));
    
    char **envp = *((char***)dlsym(RTLD_DEFAULT, "global_envp"));
    Handle<Object> js_envp = v8::Object::New();
    if (envp != NULL) {
        char *key, *value;
        while (key = *(envp++)) {
            value = strchr(key, '=') + 1;
            *(value - 1) = '\0';
            js_envp->Set(v8::String::New(key), v8::String::New(value));
            //printf("ENVP[%s]=%s\n", key, value);
        }
    }
    EXPORTS("env", js_envp);
    
    int argc = *((int*)dlsym(RTLD_DEFAULT, "global_argc"));
    char **argv = *((char***)dlsym(RTLD_DEFAULT, "global_argv"));
    
    Handle<Array> js_argv = Array::New(argc);
    for (int i = 0; i < argc; i++) {
        js_argv->Set(JS_int(i), JS_str(argv[i]));
        //printf("ARGV[%d]=%s", i, argv[i]);
    }
    EXPORTS("args", js_argv);
    
    
END_NARWHAL_MODULE
