// jsLook
addWrapper(function(Module, api) {
    /** Setup the Look class */
    api.Look = wrapperFactory(Module.Look, {
        addMaterialAssign: [api.EMPTY_STRING, api.EMPTY_STRING],
        addPropertyAssign: [api.EMPTY_STRING],
        addPropertySetAssign: [api.EMPTY_STRING],
        addVariantAssign: [api.EMPTY_STRING],
        addVisibility: [api.EMPTY_STRING]
    });

    /** Setup the LookGroup class */
    api.LookGroup = wrapperFactory(Module.LookGroup);

    /** Setup the MaterialAssign class */
    api.MaterialAssign = wrapperFactory(Module.MaterialAssign);

    /** Setup the Visibility class */
    api.Visibility = wrapperFactory(Module.Visibility);
});
