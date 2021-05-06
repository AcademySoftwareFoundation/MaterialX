// jsNode
addWrapper(function(Module, api) {
    /** Setup the Node class */
    api.Node = wrapperFactory(Module.Node, {
        getNodeDef: [api.EMPTY_STRING],
        getImplementation: [api.EMPTY_STRING]
    });

    /** Setup the GraphElement class */
    api.GraphElement = wrapperFactory(Module.GraphElement, {
        addNode: [REQUIRED, api.EMPTY_STRING, api.DEFAULT_TYPE_STRING],
        addNodeInstance: [REQUIRED, api.EMPTY_STRING],
        getNodes: [api.EMPTY_STRING],
        addBackdrop: [api.EMPTY_STRING],
        flattenSubgraphs: [api.EMPTY_STRING, null]
    });

    /** Setup the NodeGraph class */
    api.NodeGraph = wrapperFactory(Module.NodeGraph);

    /** Setup the Backdrop class */
    api.Backdrop = wrapperFactory(Module.Backdrop);
});
