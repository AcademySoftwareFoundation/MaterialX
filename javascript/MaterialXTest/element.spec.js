import { expect } from 'chai';
import Module from './_build/JsMaterialXCore.js';

describe('Element', () =>
{
    let mx, doc, valueTypes;
    const libraryPath = '../../libraries/stdlib';
    const libraryFilenames = ['stdlib_defs.mtlx', 'stdlib_ng.mtlx'];

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

    after(() =>
    {
        // Cleanup typed helper objects and document
        Object.values(valueTypes).forEach(v => v.delete());
        doc.delete();
    });

    it('should detect content document membership', async () =>
    {
        const contentDoc = mx.createDocument();
        contentDoc.setSourceUri('content.mtlx');
        const contentElem = contentDoc.addChildOfCategory('generic', 'contentElem');
        expect(contentElem.belongsToContentDocument()).to.equal(true);

        const libDoc = mx.createDocument();
        libDoc.setSourceUri('library.mtlx');
        const sourceLibElem = libDoc.addChildOfCategory('generic', 'libElem');
        sourceLibElem.setSourceUri('library.mtlx');
        contentDoc.importLibrary(libDoc);

        const libElem = contentDoc.getChild('libElem');
        expect(libElem.belongsToContentDocument()).to.equal(false);

        const stdlib = mx.createDocument();
        for (const file of libraryFilenames)
        {
            const lib = mx.createDocument();
            await mx.readFromXmlFile(lib, file, libraryPath);
            stdlib.importLibrary(lib);
            lib.delete();
        }

        const referencedContentDoc = mx.createDocument();
        referencedContentDoc.setDataLibrary(stdlib);
        const referencedNodeDef = referencedContentDoc.getChild('ND_image_color3');
        expect(referencedNodeDef).to.exist;
        expect(referencedNodeDef.belongsToContentDocument()).to.equal(false);

        const importedContentDoc = mx.createDocument();
        importedContentDoc.importLibrary(stdlib);
        const importedNodeDef = importedContentDoc.getChild('ND_image_color3');
        expect(importedNodeDef).to.exist;
        expect(importedNodeDef.belongsToContentDocument()).to.equal(false);

        importedContentDoc.delete();
        referencedContentDoc.delete();
        stdlib.delete();
        contentDoc.delete();
        libDoc.delete();
    }).timeout(10000);

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
            elem.delete();
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
            elem.delete();
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
        // No doc.delete() here; cleaned up in after()
    });
});

describe('Equivalence', () =>
{
    let mx, doc, doc2

    before(async () => {
        mx = await Module();
        doc = mx.createDocument();
        doc.addNodeGraph("graph");
        doc2 = mx.createDocument();
        doc2.addNodeGraph("graph1");
    });

    it('Compare document equivalency', () =>
    {
        let options = new mx.ElementEquivalenceOptions();
        let differences = {};
        options.performValueComparisons = false;
        let result = doc.isEquivalent(doc2, options, differences);
        expect(result).to.be.false;
        expect(differences.message).to.not.be.empty;
        result = doc.isEquivalent(doc2, options, undefined);
        expect(result).to.be.false;
    });
});
