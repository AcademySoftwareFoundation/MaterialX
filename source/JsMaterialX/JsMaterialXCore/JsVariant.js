// jsVariant
addWrapper(function(Module, api) {
    /** Setup the Variant class */
    api.Variant = wrapperFactory(Module.Variant);

    /** Setup the VariantSet class */
    api.VariantSet = wrapperFactory(Module.VariantSet, {
        'addVariant': ['']
    });

    /** Setup the VariantAssign class */
    api.VariantAssign = wrapperFactory(Module.VariantAssign);
});
