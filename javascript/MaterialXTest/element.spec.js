import { expect } from 'chai';
import Module from './_build/JsMaterialXCore.js';

describe('Element', () =>
{
    let mx, doc, valueTypes;

    const primitiveValueTypes = {
        Integer: 10,
        Boolean: true,
        String: 'test',
        Float: 15,
        IntegerArray: [1, 2, 3, 4, 5],
        FloatArray: [12, 14], // Not using actual floats to avoid precision problems
        StringArray: ['first', 'second'],
        BooleanArray: [true, true, false],
    }

    before(async () =>
    {
        mx = await Module();
        doc = mx.createDocument();
        valueTypes = {
            Color3: new mx.Color3(1, 0, 0.5),
            Color4: new mx.Color4(0, 1, 0.5, 1),
            Vector2: new mx.Vector2(0, 1),
            Vector3: new mx.Vector3(0, 1, 2),
            Vector4: new mx.Vector4(0, 1, 2, 1),
            Matrix33: new mx.Matrix33(0, 1, 2, 3, 4, 5, 6, 7, 8),
            Matrix44: new mx.Matrix44(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
        };
    });

    describe('value setters', () =>
    {
        const checkValue = (types, assertionCallback) =>
        {
            const elem = doc.addChildOfCategory('geomprop');
            Object.keys(types).forEach((typeName) =>
            {
                const setFn = `setValue${typeName}`;
                elem[setFn](types[typeName]);
                assertionCallback(elem.getValue().getData(), typeName);
            });
        };

        it('should work with expected type', () =>
        {
            checkValue(valueTypes, (returnedValue, typeName) =>
            {
                expect(returnedValue).to.be.an.instanceof(mx[`${typeName}`]);
                expect(returnedValue.equals(valueTypes[typeName])).to.equal(true);
            });
        });

        it('should work with expected primitive type', () =>
        {
            checkValue(primitiveValueTypes, (returnedValue, typeName) =>
            {
                expect(returnedValue).to.eql(primitiveValueTypes[typeName]);
            });
        });

        it('should fail for incorrect type', () =>
        {
            const elem = doc.addChildOfCategory('geomprop');
            expect(() => elem.Matrix33(true)).to.throw();
        });
    });

    describe('typed value setters', () =>
    {
        const checkTypes = (types, assertionCallback) =>
        {
            const elem = doc.addChildOfCategory('geomprop');
            Object.keys(types).forEach((typeName) =>
            {
                const setFn = `setTypedAttribute${typeName}`;
                const getFn = `getTypedAttribute${typeName}`;
                elem[setFn](typeName, types[typeName]);
                assertionCallback(elem[getFn](typeName), types[typeName]);
            });
        };

        it('should work with expected custom type', () =>
        {
            checkTypes(valueTypes, (returnedValue, originalValue) =>
            {
                expect(returnedValue.equals(originalValue)).to.equal(true);
            });
        });

        it('should work with expected primitive type', () =>
        {
            checkTypes(primitiveValueTypes, (returnedValue, originalValue) =>
            {
                expect(returnedValue).to.eql(originalValue);
            });
        });

        it('should fail for incorrect type', () =>
        {
            const elem = doc.addChildOfCategory('geomprop');
            expect(() => elem.setTypedAttributeColor3('wrongType', true)).to.throw();
        });
    });

    it('factory invocation should match specialized functions', () =>
    {
        // List based in source/MaterialXCore/Element.cpp
        const elemtypeArr = [
            'Backdrop',
            'Collection',
            'GeomInfo',
            'MaterialAssign',
            'PropertySetAssign',
            'Visibility',
            'GeomPropDef',
            'Look',
            'LookGroup',
            'PropertySet',
            'TypeDef',
            'AttributeDef',
            'NodeGraph',
            'Implementation',
            'Node',
            'NodeDef',
            'Variant',
            'Member',
            'TargetDef',
            'GeomProp',
            'Input',
            'Output',
            'Property',
            'PropertyAssign',
            'Unit',
            'UnitDef',
            'UnitTypeDef',
            'VariantAssign',
            'VariantSet',
        ];

        elemtypeArr.forEach((typeName) =>
        {
            const specializedFn = `addChild${typeName}`;
            const factoryName = typeName.toLowerCase();
            const type = mx[typeName];
            expect(doc[specializedFn]()).to.be.an.instanceof(type);
            expect(doc.addChildOfCategory(factoryName)).to.be.an.instanceof(type);
        });

        const specialElemType = {
            'MaterialX': mx.Document,
            'Comment': mx.CommentElement,
            'Generic': mx.GenericElement,
        };

        Object.keys(specialElemType).forEach((typeName) =>
        {
            const specializedFn = `addChild${typeName}`;
            const factoryName = typeName.toLowerCase();
            expect(doc[specializedFn]()).to.be.an.instanceof(specialElemType[typeName]);
            expect(doc.addChildOfCategory(factoryName)).to.be.an.instanceof(specialElemType[typeName]);
        });
    });
});

describe('Equivalence', () =>
{
    let mx, doc, doc2, inputMap, inputMap2, floatInputs;

    before(async () => {
        mx = await Module();

        doc = mx.createDocument();
        inputMap = new Map([
            ["color3", "  1.0,   +2.0,  3.0   "],
            ["color4", "1.0,   2.00, 0.3000, -4"],
            ["integer", "  12 "],
            ["matrix33", "01.0, 2.0, 0000.2310, 01.0, 2.0, 0000.2310, 01.0, 2.0, 0000.2310"],
            ["matrix44", "01.0, 2.0, 0000.2310, 0.100, 01.0, 2.0, 0000.2310, 0.100, 01.0, 2.0, 0000.2310, 0.100, 01.0, 2.0, 0000.2310, 0.100"],
            ["vector2", "1.0, 0.012345608"],
            ["vector3", "  1.0,   +2.0,  3.0   "],
            ["vector4", "1.0,   2.00, 0.3000, -4"],
            ["string", "mystring"],
            ["boolean", "false"],
            ["filename", "filename1"],
            ["float", "  1.2e-10  "],
            ["float", "  00.1000  "]
        ]);

        let index = 0;
        let child = doc.addNodeGraph("mygraph");
        let graph = child;
        const comment = doc.addChildOfCategory(mx.CommentElement.CATEGORY);
        comment.setDocString("Comment 1");

        inputMap.forEach((value, key) => {
            if (index == 0)
            {
                let input = graph.addInput(`input_${index}`, key);
                if (key === "float") {
                    input.setAttribute(mx.ValueElement.UI_MIN_ATTRIBUTE, "  0.0100 ");
                    input.setAttribute(mx.ValueElement.UI_MAX_ATTRIBUTE, "  01.0100 ");
                    index++;
                } else {
                    input.setName(`input_${key}`);
                }
                input.setValueString(value, key);
            }
        });

        doc2 = mx.createDocument();
        inputMap2 = new Map([
            ["color3", "  1.0, 2.0,  3.0   "],
            ["color4", "1, 2, 0.3, -4"],
            
            ["integer", "12"],
            ["matrix33", "1, 2, 0.231, 1, 2, 0.231, 1, 2, 0.231, 1, 2, 0.231"],
            ["matrix44", "1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1"],
            ["vector2", "1, 0.012345611"],
            ["string", "mystring"],
            ["boolean", "false"],
            ["color3", "1, 2, 3"],
            ["vector3", "1, 2, 3"],
            ["vector4", "1, 2, 0.3, -4"],
            ["filename", "filename1"],
            ["float", "1.2e-10"],
            ["float", "0.1"] 
        ]);

        index = 0;
        let child2 = doc2.addNodeGraph("mygraph");
        let graph2 = child2;
        floatInputs = [];

        inputMap2.forEach((value, key) => {
            if (index == 0) {
                let input = graph2.addInput(`input_${index}`, key);
                input.setValueString(value, key);
                if (key === "float") {
                    input.setAttribute(mx.ValueElement.UI_MIN_ATTRIBUTE, "  0.01");
                    input.setAttribute(mx.ValueElement.UI_MAX_ATTRIBUTE, "  1.01");
                    floatInputs.push(input);
                    index++;
                } else {
                    input.setName(`input_${key}`);
                }
            }
        });

        const comment2 = doc2.addChildOfCategory(mx.CommentElement.CATEGORY);
        comment2.setDocString("Comment 2");
        const comment3 = doc2.addChildOfCategory(mx.CommentElement.CATEGORY);
        comment3.setDocString("Comment 3");         
    });

    it('Compare document equivalency', () =>
    {
        let options = new mx.ElementEquivalenceOptions();

        let differences = {};
        options.performValueComparisons = false;
        let result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.false;
        console.log(differences.message);

        options.performValueComparisons = true;
        result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.true;

        let currentPrecision = mx.Value.getFloatPrecision();
        options.floatPrecision = 8;
        result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.false;
        options.floatPrecision = currentPrecision;

        options.setAttributeExclusionList([mx.ValueElement.UI_MIN_ATTRIBUTE, mx.ValueElement.UI_MAX_ATTRIBUTE]);
        floatInputs.forEach(input => {
            input.setAttribute(mx.ValueElement.UI_MIN_ATTRIBUTE, "0.9");
            input.setAttribute(mx.ValueElement.UI_MAX_ATTRIBUTE, "100.0");
        });
        result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.true;
        floatInputs.forEach(input => {
            input.setAttribute(mx.ValueElement.UI_MIN_ATTRIBUTE, "  0.01");
            input.setAttribute(mx.ValueElement.UI_MAX_ATTRIBUTE, "  1.01");
        });

        let mismatchElement = doc.getDescendant("mygraph/input_color4");
        let previousName = mismatchElement.getName();
        mismatchElement.setName("mismatch_color4");
        result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.false;

        mismatchElement.setName(previousName);
        result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.true;

        let nodeGraph = doc.getNodeGraph("mygraph");
        expect(nodeGraph).to.exist;
        doc.addNodeDef("ND_mygraph");
        nodeGraph.setNodeDefString("ND_mygraph");
        let nodeGraph2 = doc2.getNodeGraph("mygraph");
        expect(nodeGraph2).to.exist;
        doc2.addNodeDef("ND_mygraph");
        nodeGraph2.setNodeDefString("ND_mygraph");
        result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.false;
    });
});
