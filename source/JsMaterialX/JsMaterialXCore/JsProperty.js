// jsProperty
addWrapper(function(Module, api) {
    /** Setup the Property class */
    api.Property = wrapperFactory(Module.Property);

    /** Setup the PropertyAssign class */
    api.PropertyAssign = wrapperFactory(Module.PropertyAssign);

    /** Setup the PropertySet class */
    api.PropertySet = wrapperFactory(Module.PropertySet);

    /** Setup the PropertySetAssign class */
    api.PropertySetAssign = wrapperFactory(Module.PropertySetAssign);
});
