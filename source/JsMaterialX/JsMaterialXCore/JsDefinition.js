// JsDefinition
addWrapper(function(Module, api) {
    /** Setup the NodeDef class */
    api.NodeDef = wrapperFactory(Module.NodeDef, {
        getImplementation: [api.EMPTY_STRING]
    });

    /** Setup the Implementation class */
    api.Implementation = wrapperFactory(Module.Implementation);

    /** Setup the TypeDef class */
    api.TypeDef = wrapperFactory(Module.TypeDef, {
        addMember: [api.EMPTY_STRING]
    });

    /** Setup the Member class */
    api.Member = wrapperFactory(Module.Member);

    /** Setup the Unit class */
    api.Unit = wrapperFactory(Module.Unit);

    /** Setup the UnitDef class */
    api.UnitDef = wrapperFactory(Module.UnitDef);

    /** Setup the UnitTypeDef class */
    api.UnitTypeDef = wrapperFactory(Module.UnitTypeDef);
});
