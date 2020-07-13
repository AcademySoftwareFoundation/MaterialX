/**
 * Creates a js array from the passed in vector instance
 * @param {Vector} vec - Wasm vector
 * @param {Boolean} [needDelete=true] - deletes the vector after generating the array.
 * @return {Array} - Array representing the wasm vector
 */
function vecToArray(vec, needDelete=true) {
    var size = vec.size();
    var result = [];
    for (var i = 0; i < size; i++) {
        result.push(vec.get(i));
    }
    if(needDelete) {
        // avoid memory leak
        vec.delete();
    }
    return result;
}

/**
 * Generates valid arguments when calling the function
 * Throws an error if arguments are missing.
 * @param {*} args - Function arguments
 * @param {*} defaultArgs - The default arguments
 * @returns {*} arguments
 */
function argGen(args, defaultArgs = []) {
    var undefinedArgs = [];
    var args1 = [];
    var missingArgs = false;

    var keys = Object.keys(args);
    if (defaultArgs.length === 0) {
        for (let i = 0; i < keys.length; i++) {
            defaultArgs.push(REQUIRED);
        }
    }
    for (let k = 0; k < defaultArgs.length; k++) {
        var defaultArg = defaultArgs[parseInt(k)];
        if (defaultArg === REQUIRED && args[parseInt(k)] === undefined) {
            undefinedArgs.push(`arg${k}`);
            missingArgs = true;
        } else {
            args1[parseInt(k)] = args[parseInt(k)] !== undefined ? args[parseInt(k)] : defaultArg;
            if (args1[parseInt(k)] !== undefined && args1[parseInt(k)] !== null && args1[parseInt(k)].hasOwnProperty('prototype') && !missingArgs) {
                args1[parseInt(k)] = new args1[parseInt(k)]();
            }
        }
    }

    if (undefinedArgs.length > 0) {
        throw new Error(`Missing function arguments: ${undefinedArgs.toString()}`);
    }
    return args1;
}

function catchPtrError(func, handle, args, defaultArgs) {
    var funcName = func.name;
    try {
        var args1 = argGen(args, defaultArgs);
        return func.apply(handle, args1);
    } catch (exception) {
        throw new Error(`${funcName}: ${Module.getExceptionMessage(exception)}`);
    }
}

/**
 * This variable is used to specify that a function's parameter is required.
 */
var REQUIRED = 'requiredArgument';

/**
 * Wrap the Module's function to catch any uncaught pointers.
 * If the module's function 
 * @param {Function} func - Module function 
 * @param {*} defaultArgs 
 * @returns {*}
 */
function wrapperFunction(func, defaultArgs = []) {
    return function() {
        var ret = catchPtrError(func, this, arguments, defaultArgs);
        // Convert the vector into an array.
        if (ret && ret.constructor && ret.constructor.name && ret.constructor.name.indexOf('vector') === 0) {
            ret = vecToArray(ret);
        }
        return ret;
    };
}

/**
 * Wraps the class prototype functions to catch ptr errors.
 * @param {Object} klass - Module Class
 * @param {Object} [funcArgOverride={}] - An object that consists of the name of a function in the prototype as the key
 *                                        and an array of parameters for the function. This is used to specify which parameters should be optional and default.
 * @returns {Object} - the wrapped class.
 */
function wrapperFactory(klass, funcArgOverride = {}) {
    var proto = klass.prototype;
    var funcNames = Object.keys(proto);
    for (var i = 0; i < funcNames.length; i++) {
        var funcName = funcNames[parseInt(i)];
        var apiFunc = proto[String(funcName)];
        var defaultArgs = funcArgOverride[String(funcName)];
        proto[String(funcName)] = wrapperFunction(apiFunc, defaultArgs);
    }
    return klass;
}

var _wrappers;

function addWrapper(wrapperCb) {
    if (_wrappers === undefined) _wrappers = [];
    _wrappers.push(wrapperCb);
}

Module.onRuntimeInitialized = function() {
    var MaterialX = {};
    // Generate wrappers
    for (var i = 0; i < _wrappers.length; i++) {
        var wrapper = _wrappers[parseInt(i)];
        wrapper(Module, MaterialX);
    }
    Module['getMaterialX'] = function() {
        return MaterialX;
    };
};
