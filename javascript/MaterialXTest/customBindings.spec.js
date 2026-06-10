import { test, expect } from '@playwright/test';
import Module from './_build/JsMaterialXCore.js';
import { getMtlxStrings } from './testHelpers.js';

test.describe('Custom Bindings', () =>
{
    const examplesPath = '../../resources/Materials/Examples/StandardSurface';

    let mx;
    test.beforeAll(async () =>
    {
        mx = await Module();
    });

    test('Optional parameters work as expected', () =>
    {
        const doc = mx.createDocument();
        // Call a method without optional argument
        const nodeGraph = doc.addNodeGraph();
        expect(nodeGraph).toBeInstanceOf(mx.NodeGraph);
        expect(nodeGraph.getName()).toBe('nodegraph1'); // Auto-constructed default value
        // Call a method with optional argument
        const nodeGraph2 = doc.addNodeGraph('myGraph');
        expect(nodeGraph2).toBeInstanceOf(mx.NodeGraph);
        expect(nodeGraph2.getName()).toBe('myGraph');

        // Call a method that requires at least one parameter
        const node = nodeGraph.addNode('node');
        expect(node).toBeInstanceOf(mx.Node);

        // Omitting non-optional parameter should throw
        expect(() => { nodeGraph.addNode(); }).toThrow();

        // Cleanup
        node.delete();
        nodeGraph2.delete();
        nodeGraph.delete();
        doc.delete();
    });

    test('Vector <-> Array conversion', () =>
    {
        // Functions that return vectors in C++ should return an array in JS
        const doc = mx.createDocument();
        const nodeGraph = doc.addNodeGraph();
        const nodeGraphB = doc.addNodeGraph();
        const nodeGraphs = doc.getNodeGraphs();
        expect(nodeGraphs).toBeInstanceOf(Array);
        expect(nodeGraphs.length).toBe(2);

        // Elements fetched through the vector -> array conversion should be editable and changes should be reflected
        // in the original objects.
        // Note: We cannot simply compare these objects for equality, since they're separately constructed pointers
        // to the same object.
        const backdrop = nodeGraph.addBackdrop();
        const backDrops = nodeGraphs[0].getBackdrops();
        expect(backDrops.length).toBe(1);
        nodeGraphs[0].addBackdrop();
        expect(nodeGraph.getBackdrops().length).toBe(2);

        // Functions that expect vectors as parameters in C++ should accept arrays in JS
        // Built-in types (strings)
        const pathSegments = ['path', 'to', 'something'];
        const namePath = mx.createNamePath(pathSegments);
        expect(namePath).toBe(pathSegments.join(mx.NAME_PATH_SEPARATOR));

        // Complex (smart pointer) types
        const node1 = nodeGraph.addNode('node');
        const node2 = nodeGraph.addNode('node');
        const node3 = nodeGraph.addNode('node', 'anotherNode');
        backdrop.setContainsElements([node1, node2, node3]);
        const nodes = backdrop.getContainsElements();
        expect(nodes.length).toBe(3);
        expect(nodes[0].getName()).toBe('node1'); // Name auto-constructed from category
        expect(nodes[1].getName()).toBe('node2'); // Name auto-constructed from category
        expect(nodes[2].getName()).toBe('anotherNode'); // Name set explicitly at creation time

        // Cleanup created wrappers
        nodes.forEach(n => n.delete());
        backdrop.delete();
        node3.delete();
        node2.delete();
        node1.delete();
        nodeGraphB.delete();
        nodeGraph.delete();
        doc.delete();
    });

    test('C++ exception handling', () =>
    {
        // Exceptions that are thrown and caught in C++ shouldn't bubble up to JS
        const doc = mx.createDocument();
        const nodeGraph1 = doc.addNodeGraph();
        const nodeGraph2 = doc.addNodeGraph();
        nodeGraph1.setInheritsFrom(nodeGraph2);
        nodeGraph2.setInheritsFrom(nodeGraph1);
        expect(() => nodeGraph1.hasInheritanceCycle()).not.toThrow();
        expect(nodeGraph1.hasInheritanceCycle()).toBe(true);

        // Exceptions that are not caught in C++ should throw
        nodeGraph1.addNode('node', 'node1');
        expect(() => { nodeGraph1.addNode('node', 'node1'); }).toThrow();
        try
        {
            nodeGraph1.addNode('node', 'node1');
        } catch (err)
        {
            expect(typeof mx.getExceptionMessage(err)).toBe('string');
        }
        // Cleanup
        nodeGraph2.delete();
        nodeGraph1.delete();
        doc.delete();
    });

    test('getReferencedSourceUris', async () =>
    {
        const doc = mx.createDocument();
        const filename = 'standard_surface_look_brass_tiled.mtlx';
        await mx.readFromXmlFile(doc, filename, examplesPath);
        const sourceUris = doc.getReferencedSourceUris();
        expect(sourceUris).toBeInstanceOf(Array);
        expect(sourceUris.length).toBe(3);
        expect(typeof sourceUris[0]).toBe('string');
        expect(sourceUris.includes('standard_surface_brass_tiled.mtlx')).toBe(true);
        doc.delete();
    });

    test('Should invoke correct instance of \'validate\'', () =>
    {
        // We check whether the correct function is called by provoking an error message that is specific to the
        // function that we expect to be called.
        const message = {};

        // Should invoke Document::validate.
        const doc = mx.createDocument();
        expect(doc.validate()).toBe(true);
        doc.removeAttribute(mx.InterfaceElement.VERSION_ATTRIBUTE);
        expect(doc.validate()).toBe(true);

        // Should invoke Node::validate
        const node = doc.addNode('node');
        expect(node.validate()).toBe(true);
        node.setCategory('');
        expect(node.validate()).toBe(false);
        expect(node.validate(message)).toBe(false);
        expect(message.message).toContain('Node element is missing a category');

        // Should invoke inherited ValueElement::validate
        const token = new mx.Token(node, 'token');
        expect(token.validate()).toBe(true);
        token.setUnitType('bogus');
        expect(token.validate()).toBe(false);
        expect(token.validate(message)).toBe(false);
        expect(message.message).toContain('Unit type definition does not exist in document')

        // Cleanup
        token.delete();
        node.delete();
        doc.delete();
    });

    test('StringResolver name substitution getters', () =>
    {
        const fnTestData = {
            fnKey: 'fnValue',
            fnKey1: 'fnValue1'
        };
        const fnTestKeys = Object.keys(fnTestData);

        const gnTestData = {
            gnKey: 'gnValue',
            gnKey1: 'gnValue1'
        };
        const gnTestKeys = Object.keys(gnTestData);

        const resolver = mx.StringResolver.create();

        resolver.setFilenameSubstitution(fnTestKeys[0], fnTestData[fnTestKeys[0]]);
        resolver.setFilenameSubstitution(fnTestKeys[1], fnTestData[fnTestKeys[1]]);
        const fnSubs = resolver.getFilenameSubstitutions();
        expect(fnSubs).toBeInstanceOf(Object);
        expect(Object.keys(fnSubs).length).toBe(2);
        expect(fnSubs).toEqual(fnTestData);

        resolver.setGeomNameSubstitution(gnTestKeys[0], gnTestData[gnTestKeys[0]]);
        resolver.setGeomNameSubstitution(gnTestKeys[1], gnTestData[gnTestKeys[1]]);
        const gnSubs = resolver.getGeomNameSubstitutions();
        expect(gnSubs).toBeInstanceOf(Object);
        expect(Object.keys(gnSubs).length).toBe(2);
        expect(gnSubs).toEqual(gnTestData);
        resolver.delete();
    });

    test('getShaderNodes', async () =>
    {
        const doc = mx.createDocument();
        const fileNames = ['standard_surface_marble_solid.mtlx'];
        const mtlxStrs = getMtlxStrings(fileNames, examplesPath);
        await mx.readFromXmlString(doc, mtlxStrs[0]);
        let matNodes = doc.getMaterialNodes();
        expect(matNodes.length).toBe(1);
        const matNode = matNodes[0];

        // Should return a surface shader node but no displacement shader node
        let shaderNodes = mx.getShaderNodes(matNode);
        expect(shaderNodes).toBeInstanceOf(Array);
        expect(shaderNodes.length).toBe(1);
        expect(shaderNodes[0].getType()).toBe(mx.SURFACE_SHADER_TYPE_STRING);
        shaderNodes = mx.getShaderNodes(matNode, mx.DISPLACEMENT_SHADER_TYPE_STRING);
        expect(shaderNodes).toBeInstanceOf(Array);
        expect(shaderNodes.length).toBe(0);

        // Cleanup wrappers
        shaderNodes.forEach(s => s.delete());
        matNodes.forEach(n => n.delete());
        doc.delete();
    });

    test('createValidName', () =>
    {
        const testString = '_Note_:Please,turn.this+-into*1#valid\nname for_me';
        const replaceRegex = /[^a-zA-Z0-9_:]/g
        expect(mx.createValidName(testString)).toBe(testString.replace(replaceRegex, '_'));
        expect(mx.createValidName(testString, '-')).toBe(testString.replace(replaceRegex, '-'));
    });

    test('getVersionIntegers', () =>
    {
        const versionStringArr = mx.getVersionString().split('.').map((value) => parseInt(value, 10));

        // Global getVersionIntegers
        const globalVersion = mx.getVersionIntegers();
        expect(globalVersion).toBeInstanceOf(Array);
        expect(globalVersion.length).toBe(3);
        expect(globalVersion).toEqual(versionStringArr);

        // Document.getVersionIntegers
        versionStringArr.pop();
        const doc = mx.createDocument();
        const docVersion = doc.getVersionIntegers();
        expect(docVersion).toBeInstanceOf(Array);
        expect(docVersion.length).toBe(2);
        expect(docVersion).toEqual(versionStringArr);
        doc.delete();

        // InterfaceElement.getVersionIntegers (via NodeDef)
        const versionDoc = mx.createDocument();
        const nodeDef = versionDoc.addNodeDef('ND_test', 'float', 'test');
        nodeDef.setVersionString('2.5');
        const nodeDefVersion = nodeDef.getVersionIntegers();
        expect(nodeDefVersion).toBeInstanceOf(Array);
        expect(nodeDefVersion.length).toBe(2);
        expect(nodeDefVersion).toEqual([2, 5]);
        nodeDef.delete();
        versionDoc.delete();
    });
});
