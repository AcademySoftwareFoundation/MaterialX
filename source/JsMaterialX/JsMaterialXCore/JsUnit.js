// jsUnit
addWrapper(function (Module, api) {
    /** Setup the UnitConverter class */
    api.ValueElement = wrapperFactory(Module.UnitConverter);
    /** Setup the UnitConverter class */
    api.ValueElement = wrapperFactory(Module.LinearUnitConverter);
    /** Setup the UnitConverterRegistry class */
    api.ValueElement = wrapperFactory(Module.UnitConverterRegistry);
});
