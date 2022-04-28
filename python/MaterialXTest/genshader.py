#!/usr/bin/env python
'''
Unit tests for shader generation in MaterialX Python.
'''

import os, unittest

import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader
import MaterialX.PyMaterialXGenOsl as mx_gen_osl

class TestGenShader(unittest.TestCase):
    def test_ShaderInterface(self):
        doc = mx.createDocument()

        filePath = os.path.dirname(os.path.abspath(__file__))
        searchPath = os.path.join(filePath, "..", "..")
        mx.loadLibraries(["libraries"], searchPath, doc)

        exampleName = u"shader_interface"

        # Create a nodedef taking three color3 and producing another color3
        nodeDef = doc.addNodeDef("ND_foo", "color3", "foo")
        fooInputA = nodeDef.addInput("a", "color3")
        fooInputB = nodeDef.addInput("b", "color3")
        fooOutput = nodeDef.getOutput("out")
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

        doc.addNode("foo", "foo1", "color3")
        output = doc.addOutput("foo_test", "color3");
        output.setNodeName("foo1");
        output.setAttribute("output", "o");

        shadergen = mx_gen_osl.OslShaderGenerator.create()
        context = mx_gen_shader.GenContext(shadergen)
        context.registerSourceCodeSearchPath(searchPath)

        # Test complete mode
        context.getOptions().shaderInterfaceType = int(mx_gen_shader.ShaderInterfaceType.SHADER_INTERFACE_COMPLETE);
        shader = shadergen.generate(exampleName, output, context);
        self.assertTrue(shader)
        self.assertTrue(len(shader.getSourceCode(mx_gen_shader.PIXEL_STAGE)) > 0)

        ps = shader.getStage(mx_gen_shader.PIXEL_STAGE);
        uniforms = ps.getUniformBlock(mx_gen_osl.OSL_UNIFORMS)
        self.assertTrue(uniforms.size() == 2)

        outputs = ps.getOutputBlock(mx_gen_osl.OSL_OUTPUTS)
        self.assertTrue(outputs.size() == 1)
        self.assertTrue(outputs[0].getName() == output.getName())

        file = open(shader.getName() + "_complete.osl", "w+")
        file.write(shader.getSourceCode(mx_gen_shader.PIXEL_STAGE))
        file.close()
        os.remove(shader.getName() + "_complete.osl");

        context.getOptions().shaderInterfaceType = int(mx_gen_shader.ShaderInterfaceType.SHADER_INTERFACE_REDUCED);
        shader = shadergen.generate(exampleName, output, context);
        self.assertTrue(shader)
        self.assertTrue(len(shader.getSourceCode(mx_gen_shader.PIXEL_STAGE)) > 0)

        ps = shader.getStage(mx_gen_shader.PIXEL_STAGE);
        uniforms = ps.getUniformBlock(mx_gen_osl.OSL_UNIFORMS)
        self.assertTrue(uniforms.size() == 0)

        outputs = ps.getOutputBlock(mx_gen_osl.OSL_OUTPUTS)
        self.assertTrue(outputs.size() == 1)
        self.assertTrue(outputs[0].getName() == output.getName())

        file = open(shader.getName() + "_reduced.osl", "w+")
        file.write(shader.getSourceCode(mx_gen_shader.PIXEL_STAGE))
        file.close()
        os.remove(shader.getName() + "_reduced.osl");

if __name__ == '__main__':
    unittest.main()
