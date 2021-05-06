// jsUtil
addWrapper(function(Module, api) {
    // TODO: Wrap functions and provide default args
    api.getVersionString = Module.getVersionString;
    api.createValidName = function(str, char) {
        return Module.createValidName(str, char.charCodeAt(0));
    };

    api.isValidName = Module.isValidName;
    api.incrementName = Module.incrementName;

    api.getVersionIntegers = function() {
        var vec = Module.getVersionIntegers();
        return vecToArray(vec);
    };

    // TODO: Do we really need to map such helper functions? JS already has a String.split method.
    api.splitString = function(str, spl) {
        var vecStr = Module.splitString(str, spl);
        var size = vecStr.size();
        var result = [];
        for (var i = 0; i < size; i++) {
            result.push(vecStr.get(i));
        }
        return result;
    };

    api.replaceSubstrings = Module.replaceSubstrings;
});
