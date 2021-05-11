// jsDocument
addWrapper(function(Module, api) {
    /** Setup the Document class */
    api.createDocument = wrapperFunction(Module.createDocument);

    api.Document = wrapperFactory(Module.Document);

});
