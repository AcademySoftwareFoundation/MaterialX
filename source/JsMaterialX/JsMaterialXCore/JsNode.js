// jsNode
addWrapper(function(Module, api) {
    /** Setup the Node class */
    api.Node = wrapperFactory(Module.Node, {
        getNodeDef: [''],
        getImplementation: ['', '']
    });

    /** Setup the GraphElement class */
    api.GraphElement = wrapperFactory(Module.GraphElement, {
        addNode: [REQUIRED, '', api.DEFAULT_TYPE_STRING],
        addNodeInstance: [REQUIRED, ''],
        getNodes: [''],
        addBackdrop: [''],
        flattenSubgraphs: ['']
    });

    /** Setup the NodeGraph class */
    api.NodeGraph = wrapperFactory(Module.NodeGraph);

    /** Setup the Backdrop class */
    api.Backdrop = wrapperFactory(Module.Backdrop);
});
