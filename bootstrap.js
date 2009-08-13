(function (evalGlobal, global) {

    var debug = true;

    var prefix = "";
    if (typeof NARWHAL_HOME != "undefined") {
        prefix = NARWHAL_HOME;
        delete NARWHAL_HOME;
    } else {
        print("Warning: NARWHAL_HOME unknown!");
    }

    var enginePrefix = "";
    if (typeof NARWHAL_ENGINE_HOME != "undefined") {
        enginePrefix = NARWHAL_ENGINE_HOME;
        delete NARWHAL_ENGINE_HOME;
    } else {
        print("Warning: NARWHAL_ENGINE_HOME unknown!");
    }

    var prefixes = [enginePrefix, prefix];

    var _isFile = isFile, _read = read, _print = print;
    delete read, isFile, print;
    
    function NativeLoader() {
        var loader = {};
        var factories = {};
        
        loader.reload = function(topId, path) {
            _print("loading native: " + topId + " (" + path + ")");
            factories[topId] = requireNative(topId, path);
        }
        
        loader.load = function(topId, path) {
            if (!factories.hasOwnProperty(topId))
                loader.reload(topId, path);
            return factories[topId];
        }
        
        return loader;
    };

    eval(_read(prefix + "/narwhal.js"))({
        global: global,
        evalGlobal: evalGlobal,
        engine: 'v8',
        engines: ['v8', 'c', 'default'],
        debug: debug,
        print: function (string) { _print(String(string)); },
        evaluate: function (text) {
             return eval("(function(require,exports,module,system,print){" + text + "/**/\n})");
        },
        fs: {
            read: function(path) { return _read(path); },
            isFile: function(path) { return _isFile(path); }
        },
        prefix: prefix,
        prefixes: prefixes,
        loaders: [[".dylib", NativeLoader()]]
    });

})(function () {
    return eval(arguments[0]);
}, this);
