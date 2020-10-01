// jsMaterial
addWrapper(function(Module, api) {
    /** Setup the Material class */
    api.Material = wrapperFactory(Module.Material, {
        addShaderRef: ['', ''],
        getShaderNodeDefs: ['', ''],
        getPrimaryShaderNodeDef: ['', ''],
        getPrimaryShaderName: ['', ''],
        getPrimaryShaderInputs: ['', ''],
        getPrimaryShaderTokens: ['', ''],
        getGeometryBindings: [api.UNIVERSAL_GEOM_NAME]
    });

    /** Setup the BindParam class */
    api.BindParam = wrapperFactory(Module.BindParam);

    /** Setup the BindInput class */
    api.BindInput = wrapperFactory(Module.BindInput);

    /** Setup the BindToken class */
    api.BindToken = wrapperFactory(Module.BindToken);

    /** Setup the ShaderRef class */
    api.ShaderRef = wrapperFactory(Module.ShaderRef, {
        addBindParam: [REQUIRED, api.DEFAULT_TYPE_STRING],
        addBindInput: [REQUIRED, api.DEFAULT_TYPE_STRING]
    });
});
