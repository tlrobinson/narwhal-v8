print("requireNative="+requireNative);

var topId = "foo",
    path = "lib/foo.dylib";

var factory = requireNative(topId, path);

print("factory="+factory);

var require = function(id) { print("require:"+id); return { buzz : "buzz string" }; },
    exports = {},
    module = {},
    system = {};

factory(require, exports, module, system, function(id) { print("print:"+id); });

print("done");
