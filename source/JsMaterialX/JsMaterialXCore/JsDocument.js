// jsDocument
addWrapper(function(Module, api) {
    /** Setup the Document class */
    api.createDocument = wrapperFunction(Module.createDocument);

    api.Document = wrapperFactory(Module.Document, {
        importLibrary: [REQUIRED, null],
        addNodeGraph: [''],
        getMatchingPorts: [REQUIRED],
        addMaterial: [''],
        addGeomInfo: ['', api.UNIVERSAL_GEOM_NAME],
        getGeomAttrValue: [REQUIRED, api.UNIVERSAL_GEOM_NAME],
        addLook: [''],
        addLookGroup: [''],
        addCollection: [''],
        getMatchingImplementations: [REQUIRED],
        addPropertySet: [''],
        addVariantSet: [''],
        addTypeDef: [''],
        addNodeDef: ['', api.DEFAULT_TYPE_STRING, ''],
        getMatchingNodeDefs: [REQUIRED],
        addImplementation: [''],
        getGeomPropValue: [REQUIRED, api.UNIVERSAL_GEOM_NAME],
        addNodeDefFromGraph: [REQUIRED, REQUIRED, REQUIRED, REQUIRED, '']
    });

});
