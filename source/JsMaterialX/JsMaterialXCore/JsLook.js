// jsLook
addWrapper(function(Module, api) {
    /** Setup the Look class */
    api.Look = wrapperFactory(Module.Look, {
        addMaterialAssign: ['', ''],
        addPropertyAssign: [''],
        addPropertySetAssign: [''],
        addVariantAssign: [''],
        addVisibility: ['']
    });

    /** Setup the LookGroup class */
    api.LookGroup = wrapperFactory(Module.LookGroup);

    /** Setup the MaterialAssign class */
    api.MaterialAssign = wrapperFactory(Module.MaterialAssign);

    /** Setup the Visibility class */
    api.Visibility = wrapperFactory(Module.Visibility);
});
