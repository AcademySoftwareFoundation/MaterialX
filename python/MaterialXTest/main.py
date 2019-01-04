import math
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
               mx.Matrix33(0.0),
               mx.Matrix44(1.0),
               'value')

_fileDir = os.path.dirname(os.path.abspath(__file__))
_libraryDir = os.path.join(_fileDir, '../../documents/Libraries/stdlib/')
_exampleDir = os.path.join(_fileDir, '../../documents/Examples/')
_searchPath = _libraryDir + ';' + _exampleDir

_libraryFilenames = ('stdlib_defs.mtlx',
                     'stdlib_ng.mtlx',
                     'stdlib_osl_impl.mtlx')
_exampleFilenames = ('CustomNode.mtlx',
                     'Looks.mtlx',
                     'MaterialBasic.mtlx',
                     'MultiOutput.mtlx',
                     'NodeGraphs.mtlx',
                     'PaintMaterials.mtlx',
                     'PostShaderComposite.mtlx',
                     'PreShaderComposite.mtlx',
                     'BxDF/alSurface.mtlx',
                     'BxDF/Disney_BRDF_2012.mtlx',
                     'BxDF/Disney_BSDF_2015.mtlx')

_epsilon = 1e-4


#--------------------------------------------------------------------------------
class TestMaterialX(unittest.TestCase):
    def test_DataTypes(self):
        for value in _testValues:
            # Convert between values and strings.
            string = mx.valueToString(value)
            newValue = mx.stringToValue(string, type(value))
            self.assertTrue(newValue == value)

            # Convert between types and strings.
            string = mx.typeToName(type(value))
            newType = mx.nameToType(string)
            self.assertTrue(newType == type(value))

    def test_Vectors(self):
        v1 = mx.Vector3(1, 2, 3)
        v2 = mx.Vector3(2, 4, 6)

        # Indexing operators
        self.assertTrue(v1[2] == 3)
        v1[2] = 4
        self.assertTrue(v1[2] == 4)
        v1[2] = 3

        # Component-wise operators
        self.assertTrue(v2 + v1 == mx.Vector3(3, 6, 9))
        self.assertTrue(v2 - v1 == mx.Vector3(1, 2, 3))
        self.assertTrue(v2 * v1 == mx.Vector3(2, 8, 18))
        self.assertTrue(v2 / v1 == mx.Vector3(2, 2, 2))
        self.assertTrue(v1 * 2 == v2)
        self.assertTrue(v2 / 2 == v1)

        # Geometric methods
        v3 = mx.Vector4(4)
        self.assertTrue(v3.getMagnitude() == 8)
        self.assertTrue(v3.getNormalized().getMagnitude() == 1)
        self.assertTrue(v1.dot(v2) == 28)
        self.assertTrue(v1.cross(v2) == mx.Vector3())

        # Vector copy
        v4 = v2.copy()
        self.assertTrue(v4 == v2)
        v4[0] += 1;
        self.assertTrue(v4 != v2)

    def test_Matrices(self):
        # Translation and scale
        trans = mx.Matrix44.createTranslation(mx.Vector3(1, 2, 3))
        scale = mx.Matrix44.createScale(mx.Vector3(2))
        self.assertTrue(trans == mx.Matrix44(1, 0, 0, 0,
                                             0, 1, 0, 0,
                                             0, 0, 1, 0,
                                             1, 2, 3, 1))
        self.assertTrue(scale == mx.Matrix44(2, 0, 0, 0,
                                             0, 2, 0, 0,
                                             0, 0, 2, 0,
                                             0, 0, 0, 1))

        # Indexing operators
        self.assertTrue(trans[3, 2] == 3)
        trans[3, 2] = 4
        self.assertTrue(trans[3, 2] == 4)
        trans[3, 2] = 3

        # Matrix methods
        self.assertTrue(trans.getTranspose() == mx.Matrix44(1, 0, 0, 1,
                                                            0, 1, 0, 2,
                                                            0, 0, 1, 3,
                                                            0, 0, 0, 1))
        self.assertTrue(scale.getTranspose() == scale)
        self.assertTrue(trans.getDeterminant() == 1)
        self.assertTrue(scale.getDeterminant() == 8)
        self.assertTrue(trans.getInverse() ==
                        mx.Matrix44.createTranslation(mx.Vector3(-1, -2, -3)))

        # Matrix product
        prod1 = trans * scale
        prod2 = scale * trans
        prod3 = trans * 2
        prod4 = trans
        prod4 *= scale
        self.assertTrue(prod1 == mx.Matrix44(2, 0, 0, 0,
                                             0, 2, 0, 0,
                                             0, 0, 2, 0,
                                             2, 4, 6, 1))
        self.assertTrue(prod2 == mx.Matrix44(2, 0, 0, 0,
                                             0, 2, 0, 0,
                                             0, 0, 2, 0,
                                             1, 2, 3, 1))
        self.assertTrue(prod3 == mx.Matrix44(2, 0, 0, 0,
                                             0, 2, 0, 0,
                                             0, 0, 2, 0,
                                             2, 4, 6, 2))
        self.assertTrue(prod4 == prod1)

        # Matrix division
        quot1 = prod1 / scale
        quot2 = prod2 / trans
        quot3 = prod3 / 2
        quot4 = quot1
        quot4 /= trans
        self.assertTrue(quot1 == trans)
        self.assertTrue(quot2 == scale)
        self.assertTrue(quot3 == trans)
        self.assertTrue(quot4 == mx.Matrix44.IDENTITY)

        # 2D rotation
        rot1 = mx.Matrix33.createRotation(math.pi / 2)
        rot2 = mx.Matrix33.createRotation(math.pi)
        self.assertTrue((rot1 * rot1).isEquivalent(rot2, _epsilon))
        self.assertTrue(rot2.isEquivalent(
            mx.Matrix33.createScale(mx.Vector2(-1)), _epsilon))
        self.assertTrue((rot2 * rot2).isEquivalent(mx.Matrix33.IDENTITY, _epsilon))

        # 3D rotation
        rotX = mx.Matrix44.createRotationX(math.pi)
        rotY = mx.Matrix44.createRotationY(math.pi)
        rotZ = mx.Matrix44.createRotationZ(math.pi)
        self.assertTrue((rotX * rotY).isEquivalent(
            mx.Matrix44.createScale(mx.Vector3(-1, -1, 1)), _epsilon))
        self.assertTrue((rotX * rotZ).isEquivalent(
            mx.Matrix44.createScale(mx.Vector3(-1, 1, -1)), _epsilon))
        self.assertTrue((rotY * rotZ).isEquivalent(
            mx.Matrix44.createScale(mx.Vector3(1, -1, -1)), _epsilon))

        # Matrix copy
        trans2 = trans.copy()
        self.assertTrue(trans2 == trans)
        trans2[0, 0] += 1;
        self.assertTrue(trans2 != trans)

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

        # Create a custom nodedef.
        nodeDef = doc.addNodeDef('nodeDef1', 'float', 'turbulence3d')
        nodeDef.setParameterValue('octaves', 3)
        nodeDef.setParameterValue('lacunarity', 2.0)
        nodeDef.setParameterValue('gain', 0.5)

        # Reference the custom nodedef.
        custom = nodeGraph.addNode('turbulence3d', 'turbulence1', 'float')
        self.assertTrue(custom.getParameterValue('octaves') == 3)
        custom.setParameterValue('octaves', 5)
        self.assertTrue(custom.getParameterValue('octaves') == 5)

        # Validate the document.
        self.assertTrue(doc.validate()[0])

        # Test scoped attributes.
        nodeGraph.setFilePrefix('folder/')
        nodeGraph.setColorSpace('lin_rec709')
        self.assertTrue(image.getParameter('file').getResolvedValueString() == 'folder/image1.tif')
        self.assertTrue(constant.getActiveColorSpace() == 'lin_rec709')

        # Create a simple shader interface.
        shaderDef = doc.addNodeDef('shader1', 'surfaceshader', 'simpleSrf')
        diffColor = shaderDef.setInputValue('diffColor', mx.Color3(1.0))
        specColor = shaderDef.setInputValue('specColor', mx.Color3(0.0))
        roughness = shaderDef.setParameterValue('roughness', 0.25)
        texId = shaderDef.setTokenValue('texId', '01')
        self.assertTrue(roughness.getValue() == 0.25)

        # Create a material that instantiates the shader.
        material = doc.addMaterial()
        shaderRef = material.addShaderRef('shaderRef1', 'simpleSrf')
        self.assertTrue(material.getPrimaryShaderName() == 'simpleSrf')
        self.assertTrue(len(material.getPrimaryShaderParameters()) == 1)
        self.assertTrue(len(material.getPrimaryShaderInputs()) == 2)
        self.assertTrue(len(material.getPrimaryShaderTokens()) == 1)
        self.assertTrue(roughness.getBoundValue(material) == 0.25)

        # Bind a shader parameter to a value.
        bindParam = shaderRef.addBindParam('roughness')
        bindParam.setValue(0.5)
        self.assertTrue(roughness.getBoundValue(material) == 0.5)
        self.assertTrue(roughness.getDefaultValue() == 0.25)

        # Bind a shader input to a value.
        bindInput = shaderRef.addBindInput('specColor')
        bindInput.setValue(mx.Color3(0.5))
        self.assertTrue(specColor.getBoundValue(material) == mx.Color3(0.5))
        self.assertTrue(specColor.getDefaultValue() == mx.Color3(0.0))

        # Bind a shader input to a graph output.
        bindInput = shaderRef.addBindInput('diffColor')
        bindInput.setConnectedOutput(output2)
        self.assertTrue(diffColor.getUpstreamElement(material) == output2)
        self.assertTrue(diffColor.getBoundValue(material) is None)
        self.assertTrue(diffColor.getDefaultValue() == mx.Color3(1.0))

        # Bind a shader token to a value.
        bindToken = shaderRef.addBindToken('texId')
        bindToken.setValue('02')
        self.assertTrue(texId.getBoundValue(material) == '02')
        self.assertTrue(texId.getDefaultValue() == '01')

        # Create an inherited material.
        material2 = doc.addMaterial()
        material2.setInheritsFrom(material)
        self.assertTrue(roughness.getBoundValue(material2) == 0.5)
        self.assertTrue(diffColor.getUpstreamElement(material2) == output2)

        # Create a look for the material.
        look = doc.addLook()
        self.assertTrue(len(doc.getLooks()) == 1)

        # Bind the material to a geometry string.
        matAssign1 = look.addMaterialAssign("matAssign1", material.getName())
        matAssign1.setGeom("/robot1")
        self.assertTrue(matAssign1.getReferencedMaterial() == material)
        self.assertTrue(len(material.getGeometryBindings("/robot1")) == 1)
        self.assertTrue(len(material.getGeometryBindings("/robot2")) == 0)

        # Bind the material to a collection.
        matAssign2 = look.addMaterialAssign("matAssign2", material.getName())
        collection = doc.addCollection()
        collection.setIncludeGeom("/robot2")
        collection.setExcludeGeom("/robot2/left_arm")
        matAssign2.setCollection(collection)
        self.assertTrue(len(material.getGeometryBindings("/robot2")) == 1)
        self.assertTrue(len(material.getGeometryBindings("/robot2/right_arm")) == 1)
        self.assertTrue(len(material.getGeometryBindings("/robot2/left_arm")) == 0)

        # Create a property assignment.
        propertyAssign = look.addPropertyAssign("twosided")
        propertyAssign.setGeom("/robot1")
        propertyAssign.setValue(True)
        self.assertTrue(propertyAssign.getGeom() == "/robot1")
        self.assertTrue(propertyAssign.getValue() == True)

        # Create a property set assignment.
        propertySet = doc.addPropertySet()
        propertySet.setPropertyValue('matte', False)
        self.assertTrue(propertySet.getPropertyValue('matte') == False)
        propertySetAssign = look.addPropertySetAssign(propertySet.getName())
        propertySetAssign.setGeom('/robot1')
        self.assertTrue(propertySetAssign.getGeom() == '/robot1')

        # Create a variant set.
        variantSet = doc.addVariantSet()
        original = variantSet.addVariant("original")
        damaged = variantSet.addVariant("damaged")
        self.assertTrue(len(variantSet.getVariants()) == 2)

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
        # Read the standard library.
        libs = []
        for filename in _libraryFilenames:
            lib = mx.createDocument()
            mx.readFromXmlFile(lib, filename, _searchPath)
            self.assertTrue(lib.validate()[0])
            libs.append(lib)

        # Read and validate each example document.
        for filename in _exampleFilenames:
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
                self.assertTrue(material.getPrimaryShaderNodeDef())
                edgeCount = 0
                for param in material.getPrimaryShaderParameters():
                    boundValue = param.getBoundValue(material)
                    self.assertTrue(boundValue is not None)
                    for edge in param.traverseGraph(material):
                        edgeCount += 1
                for input in material.getPrimaryShaderInputs():
                    boundValue = input.getBoundValue(material)
                    upstreamElement = input.getUpstreamElement(material)
                    self.assertTrue(boundValue is not None or upstreamElement is not None)
                    for edge in input.traverseGraph(material):
                        edgeCount += 1
                self.assertTrue(edgeCount > 0)

            # Serialize to XML.
            writeOptions = mx.XmlWriteOptions()
            writeOptions.writeXIncludeEnable = False
            xmlString = mx.writeToXmlString(doc, writeOptions)

            # Verify that the serialized document is identical.
            writtenDoc = mx.createDocument()
            mx.readFromXmlString(writtenDoc, xmlString)
            self.assertTrue(writtenDoc == doc)

            # Combine document with the standard library.
            doc2 = doc.copy()
            for lib in libs:
                doc2.importLibrary(lib)
            self.assertTrue(doc2.validate()[0])

        # Read the same document twice with duplicate elements skipped.
        doc = mx.createDocument()
        readOptions = mx.XmlReadOptions()
        readOptions.skipDuplicateElements = True
        filename = 'PostShaderComposite.mtlx'
        mx.readFromXmlFile(doc, filename, _searchPath, readOptions)
        mx.readFromXmlFile(doc, filename, _searchPath, readOptions)
        self.assertTrue(doc.validate()[0])


#--------------------------------------------------------------------------------
if __name__ == '__main__':
    unittest.main()
