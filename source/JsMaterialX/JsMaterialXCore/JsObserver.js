// jsObserver
addWrapper(function(Module, api) {
    /** Setup the Observer class */
    api.Observer = wrapperFactory(Module.Observer);

    api.createObservedDocument = wrapperFunction(Module.createObservedDocument);

    /** Setup the ObservedDocument class */
    api.ObservedDocument = wrapperFactory(Module.ObservedDocument);
});
