import { test, expect } from '@playwright/test';
import Module from './_build/JsMaterialXCore.js';
import { getMtlxStrings } from './testHelpers.js';

test.describe('Code Examples', () =>
{
    test('Building a MaterialX Document', async () =>
    {
        const mx = await Module();
        // Create a document.
        const doc = mx.createDocument();

        // Create a node graph with a single image node and output.
        const nodeGraph = doc.addNodeGraph();
        expect(doc.getNodeGraphs().length).toBe(1);
        const image = nodeGraph.addNode('image');
        const nodes = nodeGraph.getNodes();
        expect(nodes.length).toBe(1);
        expect(nodes[0].equals(image)).toBe(true);

        image.setInputValueString('file', 'image1.tif', 'filename');
        const input = image.getInput('file');
        expect(input).not.toBeNull();
        expect(input.getValue().getData()).toBe('image1.tif');

        const output = nodeGraph.addOutput();
        const outputs = nodeGraph.getOutputs();
        expect(outputs.length).toBe(1);
        expect(outputs[0].equals(output)).toBe(true);

        output.setConnectedNode(image);
        const connectedNode = output.getConnectedNode();
        expect(connectedNode).not.toBeNull();
        expect(connectedNode instanceof mx.Node).toBe(true);

        // Create a simple shader interface.
        const simpleSrf = doc.addNodeDef('ND_simpleSrf', 'surfaceshader', 'simpleSrf');
        const nodeDefs = doc.getNodeDefs();
        expect(nodeDefs.length).toBe(1);
        expect(nodeDefs[0].equals(simpleSrf)).toBe(true);

        simpleSrf.setInputValueColor3('diffColor', new mx.Color3(1.0, 1.0, 1.0));
        let inputValue = simpleSrf.getInputValue('diffColor');
        expect(inputValue).not.toBeNull();
        expect(inputValue.getData().equals(new mx.Color3(1.0, 1.0, 1.0))).toBe(true);

        simpleSrf.setInputValueColor3('specColor', new mx.Color3(0.0, 0.0, 0.0));
        inputValue = simpleSrf.getInputValue('specColor');
        expect(inputValue).not.toBeNull();
        expect(inputValue.getData().equals(new mx.Color3(0.0, 0.0, 0.0))).toBe(true);

        const roughness = simpleSrf.setInputValueFloat('roughness', 0.25);
        inputValue = simpleSrf.getInputValue('roughness');
        expect(inputValue).not.toBeNull();
        expect(inputValue.getData()).toBe(0.25);

        // Instantiate the shader and create a material node that references it.
        const shaderNode = doc.addNodeInstance(simpleSrf);
        const materialNode = doc.addMaterialNode('', shaderNode);
        expect(doc.getMaterialNodes().length).toBe(1);

        // Connect the diffuse color of the shader to the image output, and
        // confirm that the upstream image node is reachable from the shader.
        shaderNode.setConnectedOutput('diffColor', output);
        const upstreamNode = shaderNode.getUpstreamElement();
        expect(upstreamNode).not.toBeNull();
        expect(upstreamNode.getName()).toBe(image.getName());

        // Override roughness on this shader instance; its default value is
        // inherited from the shader's nodedef.
        const instanceRoughness = shaderNode.setInputValueFloat('roughness', 0.5);
        expect(instanceRoughness.getValue().getData()).toBe(0.5);
        expect(instanceRoughness.getDefaultValue().getData()).toBe(roughness.getValue().getData());

        // Cleanup wrappers
        nodeDefs.forEach(nd => nd.delete());
        upstreamNode.delete();
        instanceRoughness.delete();
        materialNode.delete();
        shaderNode.delete();
        output.delete();
        image.delete();
        nodeGraph.delete();
        doc.delete();
    });

    test('Traversing a Document Tree', async () =>
    {
        const xmlStr = getMtlxStrings(
            ['standard_surface_greysphere_calibration.mtlx'],
            '../../resources/Materials/Examples/StandardSurface'
        )[0];
        const mx = await Module();

        // Read a document from disk.
        const doc = mx.createDocument();
        await mx.readFromXmlString(doc, xmlStr);

        // Traverse the document tree in depth-first order.
        const elements = doc.traverseTree();
        let imageCount = 0;
        for (let elem of elements)
        {
            if (elem.isANode('image'))
            {
                imageCount++;
            }
        }
        expect(imageCount).toBeGreaterThan(0);
        doc.delete();
    });

    test('Traversing a Dataflow Graph', async () =>
    {
        const xmlStr = getMtlxStrings(['standard_surface_marble_solid.mtlx'], '../../resources/Materials/Examples/StandardSurface')[0];
        const mx = await Module();

        // Read a document from disk.
        const doc = mx.createDocument();
        await mx.readFromXmlString(doc, xmlStr);

        // For each material node, locate its surface shader and traverse the
        // dataflow graph upstream, gathering the nodes that contribute to the
        // shading result.  The marble surface is driven by a procedural noise
        // network bound through a nodegraph, so the traversal crosses from the
        // top-level document into that nodegraph.
        const materialNodes = doc.getMaterialNodes();
        expect(materialNodes.length).toBe(1);

        let nodeCount = 0;
        let crossedIntoNodeGraph = false;
        for (const materialNode of materialNodes)
        {
            const shaderNodes = mx.getShaderNodes(materialNode);
            for (const shaderNode of shaderNodes)
            {
                for (const edge of shaderNode.traverseGraph())
                {
                    const upstreamElem = edge.getUpstreamElement();
                    if (upstreamElem instanceof mx.Node)
                    {
                        nodeCount++;

                        // An upstream node whose parent is a nodegraph rather
                        // than the document confirms that the traversal has
                        // descended into the bound nodegraph.
                        const parent = upstreamElem.getParent();
                        if (parent instanceof mx.NodeGraph)
                        {
                            crossedIntoNodeGraph = true;
                        }
                        if (parent) parent.delete();
                    }
                    if (upstreamElem) upstreamElem.delete();
                    if (edge) edge.delete();
                }
            }
            shaderNodes.forEach(s => s.delete());
        }
        expect(nodeCount).toBeGreaterThan(0);
        expect(crossedIntoNodeGraph).toBe(true);

        // Cleanup wrappers
        materialNodes.forEach(n => n.delete());
        doc.delete();
    });
});
