//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

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
        var args = argGen(arguments, defaultArgs);
        var ret = func.apply(this, args);
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
        var wrapperFunc = wrapperFunction(apiFunc, defaultArgs);
        for (const [key, value] of Object.entries(apiFunc)) {
            wrapperFunc[key] = value;
        }
        proto[String(funcName)] = wrapperFunc;
    }
    return klass;
}

var _wrappers;

function addWrapper(wrapperCb) {
    if (_wrappers === undefined) _wrappers = [];
    _wrappers.push(wrapperCb);
}

Module.onRuntimeInitialized = function() {
    // Generate wrappers
    for (var i = 0; i < _wrappers.length; i++) {
        try {
            var wrapper = _wrappers[parseInt(i)];
            wrapper(Module, Module);
        } catch(e) {
            console.log(e);
        }
    }
};
