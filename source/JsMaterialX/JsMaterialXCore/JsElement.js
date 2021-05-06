// jsElement
addWrapper(function (Module, api) {

    /** Setup the Element class */
    api.Element = wrapperFactory(Module.Element, {
        getNamePath: [null],
        addChildOfCategory: [REQUIRED, api.EMPTY_STRING],
        copyContentFrom: [REQUIRED],
        getUpstreamEdge: [0],
        getUpstreamElement: [0],
        validate: [''],
        createStringResolver: [api.EMPTY_STRING],
        traverseGraph: [],
    });

    /** Setup the TypedElement class */
    api.TypedElement = wrapperFactory(Module.TypedElement);

    var funcs = [
        'setValueinteger',
        'setValueboolean',
        'setValuefloat',
        'setValuecolor3',
        'setValuecolor4',
        'setValuevector2',
        'setValuevector3',
        'setValuevector4',
        'setValuematrix33',
        'setValuematrix44',
        'setValuestring',
        'setValueintegerarray',
        'setValuebooleanarray',
        'setValuefloatarray',
        'setValuestringarray',
    ];

    var defaultArgs = {
        getResolvedValueString: [null],
    };

    for (var i = 0; i < funcs.length; i++) {
        var name = funcs[parseInt(i, 10)];
        defaultArgs[String(name)] = [REQUIRED, api.EMPTY_STRING];
    }

    /** Setup the ValueElement class */
    api.ValueElement = wrapperFactory(Module.ValueElement, defaultArgs);

    /** Setup the Token class */
    api.Token = wrapperFactory(Module.Token);

    /** Setup the StringResolver class */
    api.StringResolver = wrapperFactory(Module.StringResolver);

    api.prettyPrint = Module.prettyPrint;
});
