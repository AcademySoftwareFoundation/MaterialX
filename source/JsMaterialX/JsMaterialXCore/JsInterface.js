// jsInterface
addWrapper(function(Module, api) {

    /** Setup the PortElement class */
    api.PortElement = wrapperFactory(Module.PortElement);

    /** Setup the Input class */
    api.Input = wrapperFactory(Module.Input);

    /** Setup the Output class */
    api.Output = wrapperFactory(Module.Output);

    var funcs = [
        'setInputValueinteger',
        'setInputValueboolean',
        'setInputValuefloat',
        'setInputValuecolor2',
        'setInputValuecolor3',
        'setInputValuecolor4',
        'setInputValuevector2',
        'setInputValuevector3',
        'setInputValuevector4',
        'setInputValuematrix33',
        'setInputValuematrix44',
        'setInputValuestring',
        'setInputValueintegerarray',
        'setInputValuebooleanarray',
        'setInputValuefloatarray',
        'setInputValuestringarray'
    ];

    var defaultArgs = {
        addInput: [api.DEFAULT_TYPE_STRING, api.DEFAULT_TYPE_STRING],
        addOutput: ['', api.DEFAULT_TYPE_STRING],
        addToken: [''],
        getInputValue: [REQUIRED, ''],
        getDeclaration: ['']
    };

    for (var i = 0; i < funcs.length; i++) {
        var name = funcs[parseInt(i, 10)];
        defaultArgs[String(name)] = [REQUIRED, REQUIRED, ''];
    }

    /** Setup the InterfaceElement class */
    api.InterfaceElement = wrapperFactory(Module.InterfaceElement, defaultArgs);
});
