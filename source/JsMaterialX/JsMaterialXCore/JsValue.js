// jsValue
addWrapper(function(Module, api) {
    var typedValues = [
        'TypedValue_integer',
        'TypedValue_boolean',
        'TypedValue_float',
        'TypedValue_color2',
        'TypedValue_color3',
        'TypedValue_color4',
        'TypedValue_vector2',
        'TypedValue_vector3',
        'TypedValue_vector4',
        'TypedValue_matrix33',
        'TypedValue_matrix44',
        'TypedValue_string',
        'TypedValue_integerarray',
        'TypedValue_booleanarray',
        'TypedValue_floatarray',
        'TypedValue_stringarray'
    ];

    function iterateTypedValues(cb) {
        for (var i = 0; i < typedValues.length; i++) {
            var typedValue = typedValues[parseInt(i, 10)];
            cb && cb(typedValue);
        }
    }

    /** Setup the Value class */
    api.Value = wrapperFactory(Module.Value);

    /** Setup the typedValue classes */
    iterateTypedValues(function(typedValue) {
        api[String(typedValue)] = wrapperFactory(Module[typedValue]);
    });
});
