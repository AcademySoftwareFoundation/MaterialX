// jsTypes
addWrapper(function(Module, api) {
    api.Vector2 = wrapperFactory(Module.Vector2);
    api.Vector3 = wrapperFactory(Module.Vector3);
    api.Vector4 = wrapperFactory(Module.Vector4);
    api.Color3 = wrapperFactory(Module.Color3);
    api.Color4 = wrapperFactory(Module.Color4);
    api.Matrix33 = wrapperFactory(Module.Matrix33);
    api.Matrix44 = wrapperFactory(Module.Matrix44);
    api.DEFAULT_TYPE_STRING = Module.DEFAULT_TYPE_STRING();
    api.EMPTY_STRING = Module.EMPTY_STRING();
    api.FILENAME_TYPE_STRING = Module.FILENAME_TYPE_STRING();
    api.GEOMNAME_TYPE_STRING = Module.GEOMNAME_TYPE_STRING();
    api.SURFACE_SHADER_TYPE_STRING = Module.SURFACE_SHADER_TYPE_STRING();
    api.DISPLACEMENT_SHADER_TYPE_STRING = Module.DISPLACEMENT_SHADER_TYPE_STRING();
    api.VOLUME_SHADER_TYPE_STRING = Module.VOLUME_SHADER_TYPE_STRING();
    api.LIGHT_SHADER_TYPE_STRING = Module.LIGHT_SHADER_TYPE_STRING();
    api.MULTI_OUTPUT_TYPE_STRING = Module.MULTI_OUTPUT_TYPE_STRING();
    api.NONE_TYPE_STRING = Module.NONE_TYPE_STRING();
    api.VALUE_STRING_TRUE = Module.VALUE_STRING_TRUE();
    api.VALUE_STRING_FALSE = Module.VALUE_STRING_FALSE();
    api.NAME_PREFIX_SEPARATOR = Module.NAME_PREFIX_SEPARATOR();
    api.NAME_PATH_SEPARATOR = Module.NAME_PATH_SEPARATOR();
    api.ARRAY_VALID_SEPARATORS = Module.ARRAY_VALID_SEPARATORS();
    api.ARRAY_PREFERRED_SEPARATOR = Module.ARRAY_PREFERRED_SEPARATOR();
});
