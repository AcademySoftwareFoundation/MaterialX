// jsDocument
addWrapper(function(Module, api) {
    /** Setup the Document class */
    api.createDocument = wrapperFunction(Module.createDocument);

    api.Document = wrapperFactory(Module.Document, {
        addNodeGraph: [api.EMPTY_STRING],
        getMatchingPorts: [REQUIRED],
        addGeomInfo: [api.EMPTY_STRING, api.UNIVERSAL_GEOM_NAME],
        getGeomPropValue: [REQUIRED, api.UNIVERSAL_GEOM_NAME],
        addLook: [api.EMPTY_STRING],
        addLookGroup: [api.EMPTY_STRING],
        addCollection: [api.EMPTY_STRING],
        getMatchingImplementations: [REQUIRED],
        addPropertySet: [api.EMPTY_STRING],
        addVariantSet: [api.EMPTY_STRING],
        addTypeDef: [REQUIRED],
        addNodeDef: [api.EMPTY_STRING, api.DEFAULT_TYPE_STRING, api.EMPTY_STRING],
        getMatchingNodeDefs: [REQUIRED],
        addImplementation: [api.EMPTY_STRING],
        getGeomPropValue: [REQUIRED, api.UNIVERSAL_GEOM_NAME],
        addNodeDefFromGraph: [REQUIRED, REQUIRED, REQUIRED, REQUIRED, REQUIRED, REQUIRED, REQUIRED, '']
    });

});
