import os
import unittest

import MaterialX as mx

"""
Unit tests for MaterialX Python.
"""


#--------------------------------------------------------------------------------
_testValues = (1,
               True,
               1.0,
               mx.Color2(0.1, 0.2),
               mx.Color3(0.1, 0.2, 0.3),
               mx.Color4(0.1, 0.2, 0.3, 0.4),
               mx.Vector2(1.0, 2.0),
               mx.Vector3(1.0, 2.0, 3.0),
               mx.Vector4(1.0, 2.0, 3.0, 4.0),
               'value')

_fileDir = os.path.dirname(os.path.abspath(__file__))
_libraryDir = os.path.join(_fileDir, '../../documents/Libraries/')
_exampleDir = os.path.join(_fileDir, '../../documents/Examples/')
_searchPath = _libraryDir + ';' + _exampleDir

_libraryFilename = 'mx_stdlib_defs.mtlx'
_exampleFilenames = ('CustomNode.mtlx',
                     'Looks.mtlx',
                     'MaterialGraphs.mtlx',
                     'PaintMaterials.mtlx',
                     'PreShaderComposite.mtlx')


#--------------------------------------------------------------------------------
class TestMaterialX(unittest.TestCase):
    def test_DataTypes(self):
        # Convert between values and strings
        for value in _testValues:
            string = mx.valueToString(value)
            newValue = mx.stringToValue(string, type(value))
            self.assertTrue(newValue == value)

    def test_BuildDocument(self):
        # Create a document.
        doc = mx.createDocument()

        # Create a node graph with constant and image sources.
        nodeGraph = doc.addNodeGraph()
        self.assertTrue(nodeGraph)
        self.assertRaises(LookupError, doc.addNodeGraph, nodeGraph.getName())
        constant = nodeGraph.addNode('constant')
        image = nodeGraph.addNode('image')

        # Connect sources to outputs.
        output1 = nodeGraph.addOutput()
        output2 = nodeGraph.addOutput()
        output1.setConnectedNode(constant)
        output2.setConnectedNode(image)
        self.assertTrue(output1.getConnectedNode() == constant)
        self.assertTrue(output2.getConnectedNode() == image)
        self.assertTrue(output1.getUpstreamElement() == constant)
        self.assertTrue(output2.getUpstreamElement() == image)

        # Set constant node color.
        color = mx.Color3(0.1, 0.2, 0.3)
        constant.setParameterValue('value', color)
        self.assertTrue(constant.getParameterValue('value') == color)

        # Set image node file.
        file = 'image1.tif'
        image.setParameterValue('file', file, 'filename')
        self.assertTrue(image.getParameterValue('file') == file)

        # Validate the document.
        self.assertTrue(doc.validate()[0])

        # Test scoped attributes.
        nodeGraph.setColorSpace('lin_rec709')
        nodeGraph.setFilePrefix('prefix')
        self.assertTrue(constant.getActiveColorSpace() == 'lin_rec709')
        self.assertTrue(image.getActiveFilePrefix() == 'prefix')

        # Create a simple shader interface.
        shader = doc.addNodeDef('shader1', 'surfaceshader', 'simpleSrf')
        diffColor = shader.addInput('diffColor', 'color3')
        specColor = shader.addInput('specColor', 'color3')
        roughness = shader.addParameter('roughness', 'float')

        # Create a material that instantiates the shader.
        material = doc.addMaterial()
        shaderRef = material.addShaderRef('', 'simpleSrf')
        self.assertTrue(material.getReferencedShaderDefs())

        # Bind the diffuse color input to the constant color output.
        bindInput = shaderRef.addBindInput('diffColor')
        bindInput.setConnectedOutput(output1)
        self.assertTrue(output1 in shaderRef.getReferencedOutputs())
        self.assertTrue(diffColor.getUpstreamElement(material) == output1)

        # Create a collection.
        collection = doc.addCollection()
        self.assertTrue(doc.getCollections())
        doc.removeCollection(collection.getName())
        self.assertFalse(doc.getCollections())

        # Create a property set.
        propertySet = doc.addPropertySet()
        property = propertySet.addProperty('twosided')
        self.assertTrue(doc.getPropertySets())
        self.assertTrue(propertySet.getProperties())
        doc.removePropertySet(propertySet.getName())
        self.assertFalse(doc.getPropertySets())

        # Generate and verify require string.
        doc.generateRequireString()
        self.assertTrue('matnodegraph' in doc.getRequireString())

        # Disconnect outputs from sources.
        output1.setConnectedNode(None)
        output2.setConnectedNode(None)
        self.assertTrue(output1.getConnectedNode() == None)
        self.assertTrue(output2.getConnectedNode() == None)

    def test_TraverseGraph(self):
        # Create a document.
        doc = mx.createDocument()

        # Create a node graph with the following structure:
        #
        # [image1] [constant]     [image2]
        #        \ /                 |   
        #    [multiply]          [contrast]         [noise3d]
        #             \____________  |  ____________/
        #                          [mix]
        #                            |
        #                         [output]
        #
        nodeGraph = doc.addNodeGraph()
        image1 = nodeGraph.addNode('image')
        image2 = nodeGraph.addNode('image')
        constant = nodeGraph.addNode('constant')
        multiply = nodeGraph.addNode('multiply')
        contrast = nodeGraph.addNode('contrast')
        noise3d = nodeGraph.addNode('noise3d')
        mix = nodeGraph.addNode('mix')
        output = nodeGraph.addOutput()
        multiply.setConnectedNode('in1', image1)
        multiply.setConnectedNode('in2', constant)
        contrast.setConnectedNode('in', image2)
        mix.setConnectedNode('fg', multiply)
        mix.setConnectedNode('bg', contrast)
        mix.setConnectedNode('mask', noise3d)
        output.setConnectedNode(mix)

        # Validate the document.
        self.assertTrue(doc.validate()[0])

        # Traverse the document tree (implicit iterator).
        nodeCount = 0
        for elem in doc.traverseTree():
            if elem.isA(mx.Node):
                nodeCount += 1
        self.assertTrue(nodeCount == 7)

        # Traverse the document tree (explicit iterator).
        nodeCount = 0
        maxElementDepth = 0
        treeIter = doc.traverseTree()
        for elem in treeIter:
            if elem.isA(mx.Node):
                nodeCount += 1
            maxElementDepth = max(maxElementDepth, treeIter.getElementDepth())
        self.assertTrue(nodeCount == 7)
        self.assertTrue(maxElementDepth == 3)

        # Traverse the document tree (prune subtree).
        nodeCount = 0
        treeIter = doc.traverseTree()
        for elem in treeIter:
            if elem.isA(mx.Node):
                nodeCount += 1
            if elem.isA(mx.NodeGraph):
                treeIter.setPruneSubtree(True)
        self.assertTrue(nodeCount == 0)

        # Traverse upstream from the graph output (implicit iterator).
        nodeCount = 0
        for edge in output.traverseGraph():
            upstreamElem = edge.getUpstreamElement()
            connectingElem = edge.getConnectingElement()
            downstreamElem = edge.getDownstreamElement()
            if upstreamElem.isA(mx.Node):
                nodeCount += 1
                if downstreamElem.isA(mx.Node):
                    self.assertTrue(connectingElem.isA(mx.Input))
        self.assertTrue(nodeCount == 7)

        # Traverse upstream from the graph output (explicit iterator).
        nodeCount = 0
        maxElementDepth = 0
        maxNodeDepth = 0
        graphIter = output.traverseGraph()
        for edge in graphIter:
            upstreamElem = edge.getUpstreamElement()
            connectingElem = edge.getConnectingElement()
            downstreamElem = edge.getDownstreamElement()
            if upstreamElem.isA(mx.Node):
                nodeCount += 1
            maxElementDepth = max(maxElementDepth, graphIter.getElementDepth())
            maxNodeDepth = max(maxNodeDepth, graphIter.getNodeDepth())
        self.assertTrue(nodeCount == 7)
        self.assertTrue(maxElementDepth == 3)
        self.assertTrue(maxNodeDepth == 3)

        # Traverse upstream from the graph output (prune subgraph).
        nodeCount = 0
        graphIter = output.traverseGraph()
        for edge in graphIter:
            upstreamElem = edge.getUpstreamElement()
            connectingElem = edge.getConnectingElement()
            downstreamElem = edge.getDownstreamElement()
            if upstreamElem.isA(mx.Node):
                nodeCount += 1
                if upstreamElem.getCategory() == 'multiply':
                    graphIter.setPruneSubgraph(True)
        self.assertTrue(nodeCount == 5)

        # Create and detect a cycle.
        multiply.setConnectedNode('in2', mix)
        self.assertTrue(output.hasUpstreamCycle())
        self.assertFalse(doc.validate()[0])
        multiply.setConnectedNode('in2', constant)
        self.assertFalse(output.hasUpstreamCycle())
        self.assertTrue(doc.validate()[0])

        # Create and detect a loop.
        contrast.setConnectedNode('in', contrast)
        self.assertTrue(output.hasUpstreamCycle())
        self.assertFalse(doc.validate()[0])
        contrast.setConnectedNode('in', image2)
        self.assertFalse(output.hasUpstreamCycle())
        self.assertTrue(doc.validate()[0])

    def test_ReadXml(self):
        # Load the standard library.
        lib = mx.createDocument()
        mx.readFromXmlFile(lib, _libraryFilename, _searchPath)
        self.assertTrue(lib.validate()[0])

        for filename in _exampleFilenames:
            # Read the example document.
            doc = mx.createDocument()
            mx.readFromXmlFile(doc, filename, _searchPath)
            self.assertTrue(doc.validate()[0])

            # Copy the document.
            copiedDoc = doc.copy()
            self.assertTrue(copiedDoc == doc)
            copiedDoc.addLook()
            self.assertTrue(copiedDoc != doc)

            # Traverse the document tree.
            valueElementCount = 0
            for elem in doc.traverseTree():
                if elem.isA(mx.ValueElement):
                    valueElementCount += 1
            self.assertTrue(valueElementCount > 0)

            # Traverse upstream from each shader input.
            for material in doc.getMaterials():
                self.assertTrue(material.getReferencedShaderDefs())
                edgeCount = 0
                for shader in material.getReferencedShaderDefs():
                    for input in shader.getInputs():
                        for edge in input.traverseGraph(material):
                            edgeCount += 1
                    for param in shader.getParameters():
                        for edge in param.traverseGraph(material):
                            edgeCount += 1
                self.assertTrue(edgeCount > 0)

            # Serialize to XML.
            xmlString = mx.writeToXmlString(doc, False)

            # Verify that the serialized document is identical.
            writtenDoc = mx.createDocument()
            mx.readFromXmlString(writtenDoc, xmlString)
            self.assertTrue(writtenDoc == doc)

            # Combine document with the standard library.
            doc2 = doc.copy()
            doc2.importLibrary(lib)
            self.assertTrue(doc2.validate()[0])

            # Verify that all referenced nodes are declared.
            for elem in doc2.traverseTree():
                if elem.isA(mx.Node):
                    self.assertTrue(elem.getReferencedNodeDef())


#--------------------------------------------------------------------------------
if __name__ == '__main__':
    unittest.main()
