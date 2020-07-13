// jsElement
addWrapper(function (Module, api) {
    /** Setup the CopyOptions class */
    api.CopyOptions = Module.CopyOptions;

    /** Setup the Element class */
    api.Element = wrapperFactory(Module.Element, {
        getNamePath: [null],
        addChildOfCategory: [REQUIRED, '', true],
        copyContentFrom: [REQUIRED, api.CopyOptions],
        getUpstreamEdge: [null, 0],
        getUpstreamElement: [null, 0],
        validate: [''],
        createStringResolver: ['', null, '', ''],
        traverseGraph: [null],
    });

    /** Setup the TypedElement class */
    api.TypedElement = wrapperFactory(Module.TypedElement);

    var funcs = [
        'setValueinteger',
        'setValueboolean',
        'setValuefloat',
        'setValuecolor2',
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
        defaultArgs[String(name)] = [REQUIRED, ''];
    }

    /** Setup the ValueElement class */
    api.ValueElement = wrapperFactory(Module.ValueElement, defaultArgs);

    /** Setup the Token class */
    api.Token = wrapperFactory(Module.Token);

    /** Setup the StringResolver class */
    api.StringResolver = wrapperFactory(Module.StringResolver);
});
