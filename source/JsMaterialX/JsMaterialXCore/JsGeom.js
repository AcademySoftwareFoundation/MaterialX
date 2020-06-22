// jsGeom
addWrapper(function(Module, api) {
    /** Setup the GeomElement class */
    api.GeomElement = wrapperFactory(Module.GeomElement);

    /** Setup the GeomInfo class */
    api.GeomInfo = wrapperFactory(Module.GeomInfo);

    /** Setup the GeomProp class */
    api.GeomProp = wrapperFactory(Module.GeomProp);

    /** Setup the GeomPropDef class */
    api.GeomPropDef = wrapperFactory(Module.GeomPropDef);

    /** Setup the Collection class */
    api.Collection = wrapperFactory(Module.Collection);

    api.geomStringsMatch = wrapperFunction(Module.geomStringsMatch);

    api.UNIVERSAL_GEOM_NAME = Module.UNIVERSAL_GEOM_NAME();
});
