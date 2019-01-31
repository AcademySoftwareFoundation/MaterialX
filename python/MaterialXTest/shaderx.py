import math
import os
import unittest

import MaterialX as mx

_fileDir = os.path.dirname(os.path.abspath(__file__))

def _getSubDirectories(libraryPath):
    return [name for name in os.listdir(libraryPath)
            if os.path.isdir(os.path.join(libraryPath, name))]

def _getMTLXFilesInDirectory(path):
    for file in os.listdir(path):
        if file.endswith(".mtlx"):
            yield file

def _loadLibrary(file, doc):
    libDoc = mx.createDocument()
    mx.readFromXmlFile(libDoc, file)
    libDoc.setSourceUri(file)
    copyOptions = mx.CopyOptions()
    copyOptions.skipDuplicateElements = True;
    doc.importLibrary(libDoc, copyOptions)

def _loadLibraries(doc, searchPath, libraryPath):
    librarySubPaths = _getSubDirectories(libraryPath)
    librarySubPaths.append(libraryPath)
    for path in librarySubPaths:
        filenames = _getMTLXFilesInDirectory(os.path.join(libraryPath, path))
        for filename in filenames:
            filePath = os.path.join(libraryPath, os.path.join(path, filename))
            _loadLibrary(filePath, doc)

"""
Unit tests for ShaderX Python.
"""
class TestShaderX(unittest.TestCase):

    def test_ShaderInterface(self):
        doc = mx.createDocument()

        searchPath = os.path.join(_fileDir, "../../documents/Libraries")
        libraryPath = os.path.join(searchPath, "stdlib")
        _loadLibraries(doc, searchPath, libraryPath)

        exampleName = u"shader_interface"

        # Create a nodedef taking three color3 and producing another color3
        nodeDef = doc.addNodeDef("ND_foo", "color3", "foo")
        fooInputA = nodeDef.addInput("a", "color3")
        fooInputB = nodeDef.addInput("b", "color3")
        fooOutput = nodeDef.addOutput("o", "color3")
        fooInputA.setValue(mx.Color3(1.0, 1.0, 0.0))
        fooInputB.setValue(mx.Color3(0.8, 0.1, 0.1))

        # Create an implementation graph for the nodedef performing
        # a multiplication of the three colors.
        nodeGraph = doc.addNodeGraph("IMP_foo")
        nodeGraph.setAttribute("nodedef", nodeDef.getName())

        output = nodeGraph.addOutput(fooOutput.getName(), "color3")
        mult1 = nodeGraph.addNode("multiply", "mult1", "color3")
        in1 = mult1.addInput("in1", "color3")
        in1.setInterfaceName(fooInputA.getName())
        in2 = mult1.addInput("in2", "color3")
        in2.setInterfaceName(fooInputB.getName())
        output.setConnectedNode(mult1)

        foo = doc.addNode("foo", "foo1", "color3")
        output = doc.addOutput("foo_test", "color3");
        output.setNodeName("foo1");
        output.setAttribute("output", "o");

        options = mx.GenOptions()

        shadergen = mx.ArnoldShaderGenerator.create()
        # Add path to find all source code snippets
        shadergen.registerSourceCodeSearchPath(mx.FilePath(searchPath))
        # Add path to find OSL include files
        shadergen.registerSourceCodeSearchPath(mx.FilePath(os.path.join(searchPath, "stdlib/osl")))

        # Test complete mode
        options.shaderInterfaceType = mx.ShaderInterfaceType.SHADER_INTERFACE_COMPLETE;
        shader = shadergen.generate(exampleName, output, options);
        self.assertTrue(shader)
        self.assertTrue(len(shader.getSourceCode(mx.Shader.PIXEL_STAGE)) > 0)

        uniforms = shader.getUniformBlock(mx.Shader.PIXEL_STAGE, mx.Shader.PUBLIC_UNIFORMS)
        self.assertTrue(uniforms.size() == 2)

        outputs = shader.getOutputBlock()
        self.assertTrue(outputs.size() == 1)
        self.assertTrue(outputs[0].name == output.getName())

        file = open(shader.getName() + "_complete.osl", "w+")
        file.write(shader.getSourceCode(mx.Shader.PIXEL_STAGE))
        file.close()

        # Test reduced mode
        options.shaderInterfaceType = mx.ShaderInterfaceType.SHADER_INTERFACE_REDUCED;
        shader = shadergen.generate(exampleName, output, options);
        self.assertTrue(shader)
        self.assertTrue(len(shader.getSourceCode(mx.Shader.PIXEL_STAGE)) > 0)

        uniforms = shader.getUniformBlock(mx.Shader.PIXEL_STAGE, mx.Shader.PUBLIC_UNIFORMS)
        self.assertTrue(uniforms.size() == 0)

        outputs = shader.getOutputBlock()
        self.assertTrue(outputs.size() == 1)
        self.assertTrue(outputs[0].name == output.getName())

        file = open(shader.getName() + "_reduced.osl", "w+")
        file.write(shader.getSourceCode(mx.Shader.PIXEL_STAGE))
        file.close()

if __name__ == '__main__':
    unittest.main()
