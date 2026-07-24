import { test, expect } from '@playwright/test';
import Module from './_build/JsMaterialXCore.js';

test.describe('Element', () =>
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

    test.beforeAll(async () =>
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

    test.afterAll(() =>
    {
        // Cleanup typed helper objects and document
        Object.values(valueTypes).forEach(v => v.delete());
        doc.delete();
    });

    test.describe('value setters', () =>
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
            elem.delete();
        };

        test('should work with expected type', () =>
        {
            checkValue(valueTypes, (returnedValue, typeName) =>
            {
                expect(returnedValue).toBeInstanceOf(mx[`${typeName}`]);
                expect(returnedValue.equals(valueTypes[typeName])).toBe(true);
            });
        });

        test('should work with expected primitive type', () =>
        {
            checkValue(primitiveValueTypes, (returnedValue, typeName) =>
            {
                expect(returnedValue).toEqual(primitiveValueTypes[typeName]);
            });
        });

        test('should fail for incorrect type', () =>
        {
            const elem = doc.addChildOfCategory('geomprop');
            expect(() => elem.Matrix33(true)).toThrow();
        });
    });

    test.describe('typed value setters', () =>
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
            elem.delete();
        };

        test('should work with expected custom type', () =>
        {
            checkTypes(valueTypes, (returnedValue, originalValue) =>
            {
                expect(returnedValue.equals(originalValue)).toBe(true);
            });
        });

        test('should work with expected primitive type', () =>
        {
            checkTypes(primitiveValueTypes, (returnedValue, originalValue) =>
            {
                expect(returnedValue).toEqual(originalValue);
            });
        });

        test('should fail for incorrect type', () =>
        {
            const elem = doc.addChildOfCategory('geomprop');
            expect(() => elem.setTypedAttributeColor3('wrongType', true)).toThrow();
        });
    });

    test('factory invocation should match specialized functions', () =>
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
            expect(doc[specializedFn]()).toBeInstanceOf(type);
            expect(doc.addChildOfCategory(factoryName)).toBeInstanceOf(type);
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
            expect(doc[specializedFn]()).toBeInstanceOf(specialElemType[typeName]);
            expect(doc.addChildOfCategory(factoryName)).toBeInstanceOf(specialElemType[typeName]);
        });
        // No doc.delete() here; cleaned up in afterAll()
    });
});

test.describe('Equivalence', () =>
{
    let mx, doc, doc2

    test.beforeAll(async () => {
        mx = await Module();
        doc = mx.createDocument();
        doc.addNodeGraph("graph");
        doc2 = mx.createDocument();
        doc2.addNodeGraph("graph1");
    });

    test('Compare document equivalency', () =>
    {
        let options = new mx.ElementEquivalenceOptions();
        let differences = {};
        options.performValueComparisons = false;
        let result = doc.isEquivalent(doc2, options, differences);
        expect(result).toBe(false);
        expect(differences.message).toBeTruthy();
        result = doc.isEquivalent(doc2, options, undefined);
        expect(result).toBe(false);
    });
});
