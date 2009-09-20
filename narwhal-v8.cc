// Copyright 2009 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <v8-debug.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>
#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <narwhal.h>

int global_argc = 0;
char** global_argv = NULL;
char** global_envp = NULL;

void RunShell(v8::Handle<v8::Context> context);
bool ExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions);
v8::Handle<v8::Value> Print(const v8::Arguments& args);
v8::Handle<v8::Value> Read(const v8::Arguments& args);
v8::Handle<v8::Value> Load(const v8::Arguments& args);
v8::Handle<v8::Value> Quit(const v8::Arguments& args);
v8::Handle<v8::Value> Version(const v8::Arguments& args);
v8::Handle<v8::String> ReadFile(const char* name);
void ReportException(v8::TryCatch* handler);

v8::Handle<v8::Value> IsFile(const v8::Arguments& args);
v8::Handle<v8::Value> Require(const v8::Arguments& args);

int RunMain(int argc, char* argv[], char* envp[]) {
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    v8::HandleScope handle_scope;
    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    // Bind the global 'print' function to the C++ Print callback.
    global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
    // Bind the global 'read' function to the C++ Read callback.
    global->Set(v8::String::New("read"), v8::FunctionTemplate::New(Read));
    // Bind the global 'load' function to the C++ Load callback.
    global->Set(v8::String::New("load"), v8::FunctionTemplate::New(Load));
    // Bind the 'quit' function
    global->Set(v8::String::New("quit"), v8::FunctionTemplate::New(Quit));
    // Bind the 'version' function
    global->Set(v8::String::New("version"), v8::FunctionTemplate::New(Version));
    global->Set(v8::String::New("requireNative"), v8::FunctionTemplate::New(Require));
    global->Set(v8::String::New("isFile"), v8::FunctionTemplate::New(IsFile));

    //v8::Debug::EnableAgent("narwhal-v8", 5858);

    // Create a new execution environment containing the built-in
    // functions
    v8::Handle<v8::Context> context = v8::Context::New(NULL, global);
    // Enter the newly created execution environment.
    v8::Context::Scope context_scope(context);

    global_argc = argc;
    global_argv = argv;
    global_envp = envp;
    
    // TODO: cleanup all this. strcpy, sprintf, etc BAD!
    char buffer[1024];
    
    // start with the executable name from argv[0]
    char *executable = argv[0];

    // follow any symlinks
    size_t len;
    while ((int)(len = readlink(executable, buffer, sizeof(buffer))) >= 0) {
        buffer[len] = '\0';
        // make relative to symlink's directory
        if (buffer[0] != '/') {
            char tmp[1024];
            strcpy(tmp, buffer);
            sprintf(buffer, "%s/%s", dirname(executable), tmp);
        }
        executable = buffer;
    }
    
    // make absolute
    if (executable[0] != '/') {
        char tmp[1024];
        getcwd(tmp, sizeof(tmp));
        sprintf(buffer, "%s/%s", tmp, executable);
        executable = buffer;
    }
    
    char NARWHAL_HOME[1024], NARWHAL_ENGINE_HOME[1024];

    // try getting NARWHAL_ENGINE_HOME from env variable. fall back to 2nd ancestor of executable path
    if (getenv("NARWHAL_ENGINE_HOME"))
        strcpy(NARWHAL_ENGINE_HOME, getenv("NARWHAL_ENGINE_HOME"));
    else
        strcpy(NARWHAL_ENGINE_HOME, dirname(dirname(executable)));

    // try getting NARWHAL_HOME from env variable. fall back to 2nd ancestor of NARWHAL_ENGINE_HOME
    if (getenv("NARWHAL_HOME"))
        strcpy(NARWHAL_HOME, getenv("NARWHAL_HOME"));
    else
        strcpy(NARWHAL_HOME, dirname(dirname(NARWHAL_ENGINE_HOME)));

    // inject NARWHAL_HOME and NARWHAL_ENGINE_HOME
    snprintf(buffer, sizeof(buffer), "NARWHAL_HOME='%s';", NARWHAL_HOME);
    if (!ExecuteString(v8::String::New(buffer), v8::String::New("[setup:NARWHAL_HOME]"), false, true))
        return 1;

    snprintf(buffer, sizeof(buffer), "NARWHAL_ENGINE_HOME='%s';", NARWHAL_ENGINE_HOME);
    if (!ExecuteString(v8::String::New(buffer), v8::String::New("[setup:NARWHAL_ENGINE_HOME]"), false, true))
        return 1;

    // read and execute bootstrap.js
    snprintf(buffer, sizeof(buffer), "%s/bootstrap.js", NARWHAL_ENGINE_HOME);
    v8::Handle<v8::String> source = ReadFile(buffer);
    
    if (source.IsEmpty()) {
        printf("Error reading bootstrap.js from %s\n", buffer);
        return 1;
    }
    if (!ExecuteString(source, v8::String::New(buffer), false, true)) {
        printf("Error executing bootstrap.js from %s\n", buffer);
        return 1;
    }

    // run the shell. eventually this will be replaced by narwhal's REPL.
    RunShell(context);

    return 0;
}


int main(int argc, char* argv[], char* envp[]) {
  int result = RunMain(argc, argv, envp);
  v8::V8::Dispose();
  return result;
}


// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}


// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
v8::Handle<v8::Value> Print(const v8::Arguments& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope;
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
  return v8::Undefined();
}


// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.  This function loads the content of the file named in
// the argument into a JavaScript string.
v8::Handle<v8::Value> Read(const v8::Arguments& args) {
  if (args.Length() != 1) {
    return v8::ThrowException(v8::String::New("Bad parameters"));
  }
  v8::String::Utf8Value file(args[0]);
  if (*file == NULL) {
    return v8::ThrowException(v8::String::New("Error loading file"));
  }
  v8::Handle<v8::String> source = ReadFile(*file);
  if (source.IsEmpty()) {
    return v8::ThrowException(v8::String::New("Error loading file"));
  }
  return source;
}


// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
v8::Handle<v8::Value> Load(const v8::Arguments& args) {
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope;
    v8::String::Utf8Value file(args[i]);
    if (*file == NULL) {
      return v8::ThrowException(v8::String::New("Error loading file"));
    }
    v8::Handle<v8::String> source = ReadFile(*file);
    if (source.IsEmpty()) {
      return v8::ThrowException(v8::String::New("Error loading file"));
    }
    if (!ExecuteString(source, v8::String::New(*file), false, false)) {
      return v8::ThrowException(v8::String::New("Error executing file"));
    }
  }
  return v8::Undefined();
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
v8::Handle<v8::Value> Quit(const v8::Arguments& args) {
  // If not arguments are given args[0] will yield undefined which
  // converts to the integer value 0.
  int exit_code = args[0]->Int32Value();
  exit(exit_code);
  return v8::Undefined();
}


v8::Handle<v8::Value> Version(const v8::Arguments& args) {
  return v8::String::New(v8::V8::GetVersion());
}


// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return v8::Handle<v8::String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  v8::Handle<v8::String> result = v8::String::New(chars, size);
  delete[] chars;
  return result;
}


// The read-eval-execute loop of the shell.
void RunShell(v8::Handle<v8::Context> context) {
  printf("Narwhal version %s, V8 version %s\n", "0.1", v8::V8::GetVersion());
  //static const int kBufferSize = 256;
  while (true) {
    //char buffer[kBufferSize];
    //printf("> ");
    //char* str = fgets(buffer, kBufferSize, stdin);
    
    char *str = readline("> ");
    if (str && *str)
      add_history(str);
    
    if (str == NULL) break;
    v8::HandleScope handle_scope;
    ExecuteString(v8::String::New(str),
                  v8::String::New("(shell)"),
                  true,
                  true);
                  
    free(str);
  }
  printf("\n");
}


// Executes a string within the current v8 context.
bool ExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions) {
  v8::HandleScope handle_scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
  if (script.IsEmpty()) {
    // Print errors that happened during compilation.
    if (report_exceptions)
      ReportException(&try_catch);
    return false;
  } else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      // Print errors that happened during execution.
      if (report_exceptions)
        ReportException(&try_catch);
      return false;
    } else {
      if (print_result && !result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value.
        v8::String::Utf8Value str(result);
        const char* cstr = ToCString(str);
        printf("%s\n", cstr);
      }
      return true;
    }
  }
}


void ReportException(v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope;
  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Handle<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    printf("%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber();
    printf("%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = ToCString(sourceline);
    printf("%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      printf(" ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      printf("^");
    }
    printf("\n");
  }
}


typedef v8::Handle<v8::Object> (*module_builder_t)(v8::Handle<v8::Object> __module__);
typedef v8::Handle<v8::Value> (*factory_t)(const v8::Arguments&);
typedef const char *(*getModuleName_t)(void);

FUNCTION(Require)
{
	ARG_COUNT(2)
	ARGN_UTF8(topId,0);
    ARGN_UTF8(path,1);
    
    void *handle = dlopen(path, RTLD_LOCAL | RTLD_LAZY);
    //printf("handle=%p\n", handle);
    if (handle == NULL) {
        printf("dlopen error: %s\n", dlerror());
    	return JS_null;
    }
    
    getModuleName_t getModuleName = (getModuleName_t)dlsym(handle, "getModuleName");
    if (getModuleName == NULL) {
        printf("dlsym (getModuleName) error: %s\n", dlerror());
    	return JS_null;
    }
    //printf("getModuleName=%p moduleName=%s\n", getModuleName, getModuleName());
    
    factory_t func = (factory_t)dlsym(handle, getModuleName());
    //printf("func=%p\n", func);
    if (func == NULL) {
        printf("dlsym (%s) error: %s\n", getModuleName, dlerror());
    	return JS_null;
    }
    
    return JS_fn(func);
}
END


v8::Handle<v8::Value> IsFile(const v8::Arguments& args) {
    v8::HandleScope handle_scope;
    v8::String::Utf8Value str(args[0]);

	struct stat stat_info;
	// TODO: Check errors
	if (stat(ToCString(str), &stat_info) != -1) {
		return v8::Boolean::New(S_ISREG(stat_info.st_mode));
	} else {
		return v8::Boolean::New(0);
	}
}