#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>
#include <MaterialXShaderGen/Implementations/Swizzle.h>
#include <MaterialXShaderGen/Util.h>

#include <iostream>
#include <fstream>

namespace mx = MaterialX;

struct GeneratorDescription
{
    mx::ShaderGeneratorPtr shadergen;
    std::string fileExt;
    std::vector<std::string> implementationLibrary;
};

bool isTopologicalOrder(const std::vector<const mx::SgNode*>& nodeOrder)
{
    std::set<const mx::SgNode*> prevNodes;
    for (const mx::SgNode* node : nodeOrder)
    {
        for (auto input : node->getInputs())
        {
            if (input->connection && !prevNodes.count(input->connection->node))
            {
                return false;
            }
        }
        prevNodes.insert(node);
    }
    return true;
}

TEST_CASE("OslSyntax", "[shadergen]")
{
    mx::SyntaxPtr syntax = std::make_shared<mx::OslSyntax>();

    REQUIRE(syntax->getTypeName("float") == "float");
    REQUIRE(syntax->getTypeName("color3") == "color");
    REQUIRE(syntax->getTypeName("vector3") == "vector");

    REQUIRE(syntax->getTypeName("BSDF") == "BSDF");
    REQUIRE(syntax->getOutputTypeName("BSDF") == "output BSDF");

    std::string dv = syntax->getTypeDefault("float");
    REQUIRE(dv == "0.0");
    dv = syntax->getTypeDefault("color3", false);
    REQUIRE(dv == "color(0.0, 0.0, 0.0)");
    dv = syntax->getTypeDefault("color3", true);
    REQUIRE(dv == "color(0.0, 0.0, 0.0)");
}

TEST_CASE("Swizzling", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx"
    };
    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::ArnoldShaderGenerator sg;
    mx::SyntaxPtr syntax = sg.getSyntax();

    // Test swizzle syntax
    std::string var1 = syntax->getSwizzledVariable("foo", "color3", "color3", "bgr");
    REQUIRE(var1 == "color(foo[2], foo[1], foo[0])");
    std::string var2 = syntax->getSwizzledVariable("foo", "color4", "float", "rr01");
    REQUIRE(var2 == "pack(foo, foo, 0, 1)");
    std::string var3 = syntax->getSwizzledVariable("foo", "color2", "vector2", "xy");
    REQUIRE(var3 == "color2(foo.x, foo.y)");

    // Create a simple test graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr constant = nodeGraph->addNode("constant", "constant1", "color3");
    mx::NodePtr swizzle = nodeGraph->addNode("swizzle", "swizzle1", "color3");
    swizzle->setConnectedNode("in", constant);
    swizzle->setParameterValue("channels", std::string("rrr"));
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(swizzle);

    // Create shader gen graph
    mx::SgNodeGraphPtr sgNodeGraph = mx::SgNodeGraph::creator(nodeGraph->getName(), output1, sg);
    mx::SgNode* sgNode = sgNodeGraph->getNode("swizzle1");

    // Test swizzle implementation
    mx::Shader test1("test1");
    test1.addFunctionCall(sgNode, sg);
    REQUIRE(test1.getSourceCode() == "color swizzle1_out = color(constant1_out[0], constant1_out[0], constant1_out[0]);\n");

    // Change swizzle pattern and test again
    sgNode->getInput("channels")->value = mx::Value::createValue(std::string("b0b"));
    mx::Shader test2("test2");
    test2.addFunctionCall(sgNode, sg);
    REQUIRE(test2.getSourceCode() == "color swizzle1_out = color(constant1_out[2], 0, constant1_out[2]);\n");
}

TEST_CASE("Simple Nodegraph Shader Generation", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a simple node graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("simple_test1");
    mx::NodePtr constant = nodeGraph->addNode("constant", "constant1", "color3");
    constant->setParameterValue("value", mx::Color3(1, 0, 0));
    mx::NodePtr image = nodeGraph->addNode("image", "image1", "color3");
    image->setParameterValue("file", std::string("/foo/bar.exr"), "filename");
    mx::NodePtr multiply = nodeGraph->addNode("multiply", "multiply1", "color3");
    multiply->setConnectedNode("in1", constant);
    multiply->setConnectedNode("in2", image);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(multiply);

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = mx::printGraphDot(nodeGraph);
    file.open(nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::ArnoldShaderGenerator::creator(), "osl",  {"documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx"} },
        { mx::OgsFxShaderGenerator::creator(), "ogsfx", {"documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx"} }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = desc.shadergen->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_graphoutput." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();

        // Test shader generation from a node
        shader = desc.shadergen->generate(nodeGraph->getName(), multiply);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_node." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Conditional Nodegraph Shader Generation", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a simple node graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("conditional_test1");

    mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
    constant1->setParameterValue("value", mx::Color3(1, 0, 0));
    mx::NodePtr constant2 = nodeGraph->addNode("constant", "constant2", "color3");
    constant2->setParameterValue("value", mx::Color3(0, 1, 0));
    mx::NodePtr constant3 = nodeGraph->addNode("constant", "constant3", "float");
    constant3->setParameterValue("value", 0.5f);

    mx::NodePtr compare1 = nodeGraph->addNode("compare", "compare1", "color3");
    compare1->setConnectedNode("in1", constant1);
    compare1->setConnectedNode("in2", constant2);
    compare1->setConnectedNode("intest", constant3);

    mx::NodePtr constant4 = nodeGraph->addNode("constant", "constant4", "color3");
    constant4->setParameterValue("value", mx::Color3(1, 0, 0));
    mx::NodePtr constant5 = nodeGraph->addNode("constant", "constant5", "color3");
    constant5->setParameterValue("value", mx::Color3(0, 1, 0));
    mx::NodePtr constant6 = nodeGraph->addNode("constant", "constant6", "color3");
    constant6->setParameterValue("value", mx::Color3(0, 0, 1));

    mx::NodePtr switch1 = nodeGraph->addNode("switch", "switch1", "color3");
    switch1->setConnectedNode("in1", constant4);
    switch1->setConnectedNode("in2", constant5);
    switch1->setConnectedNode("in3", constant6);
    switch1->setConnectedNode("in4", compare1);
    switch1->setParameterValue<float>("which", 3);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(switch1);

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = mx::printGraphDot(nodeGraph);
    file.open(nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::ArnoldShaderGenerator::creator(), "osl",{ "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx" } },
        { mx::OgsFxShaderGenerator::creator(), "ogsfx",{ "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx" } }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        mx::ShaderPtr shader = desc.shadergen->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Subgraph Shader Generation", "[shadergen]")
{
    mx::ShaderGenerator::registerSourceCodeSearchPath(mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries"));

    mx::DocumentPtr doc = mx::createDocument();

    // Load example files
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples;");
    searchPath += mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries/stdlib;");
    searchPath += mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries/sx");
    std::vector<std::string> filenames =
    {
        "SubGraphs.mtlx",
        "BsdfSubGraphs.mtlx"
    };
    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename, searchPath);
    }

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::ArnoldShaderGenerator::creator(), "osl",
        {
            "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx",
            "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_osl.mtlx",
        }
        },
        { mx::OgsFxShaderGenerator::creator(), "ogsfx",
        {
            "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
            "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_glsl.mtlx",
        }
        }
    };

    std::vector<std::string> exampleGraphNames = { "subgraph_ex1" , "subgraph_ex2" };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            // Write out a .dot file for visualization
            std::ofstream file;
            std::string dot = mx::printGraphDot(nodeGraph);
            file.open(nodeGraph->getName() + ".dot");
            file << dot;
            file.close();

            // Test shader generation from the output
            mx::ShaderPtr shader = desc.shadergen->generate(nodeGraph->getName(), output);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            // TODO: Match against blessed versions
            file.open(shader->getName() + "." + desc.fileExt);
            file << shader->getSourceCode();
            file.close();
        }
    }
}

TEST_CASE("Material Shader Generation", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/adsk/adsk_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a node graph with the following structure:
    //
    // [image1] [constant]     
    //        \ /                    
    //    [multiply]          [image2]         [adskCellNoise2d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image");
    image1->addParameter("file", "filename");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    image2->addParameter("file", "filename");
    mx::NodePtr noise = nodeGraph->addNode("adskCellNoise2d", mx::EMPTY_STRING, "float");
    mx::NodePtr constant = nodeGraph->addNode("constant");
    mx::ParameterPtr v = constant->addParameter("value", "color3");
    v->setValueString("1.0, 1.0, 1.0");
    mx::NodePtr multiply = nodeGraph->addNode("multiply");
    mx::NodePtr mix = nodeGraph->addNode("mix");
    mx::OutputPtr output = nodeGraph->addOutput();
    multiply->setConnectedNode("in1", image1);
    multiply->setConnectedNode("in2", constant);
    mix->setConnectedNode("fg", multiply);
    mix->setConnectedNode("bg", image2);
    mix->setConnectedNode("mask", noise);
    output->setConnectedNode(mix);

    // Create a material with a shader ref connecting to the graph
    mx::MaterialPtr material = doc->addMaterial("adskSurfaceMtrl");
    mx::ShaderRefPtr shaderRef = material->addShaderRef("adskSurface1", "adskSurface");
    mx::BindInputPtr base = shaderRef->addBindInput("base", "float");
    mx::BindInputPtr baseColor = shaderRef->addBindInput("base_color", "color3");
    base->setValue(0.6f);
    baseColor->setConnectedOutput(output);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::ArnoldShaderGenerator::creator(), "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_osl.mtlx"
            }
        },
        { mx::OgsFxShaderGenerator::creator(), "ogsfx",
            { 
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_glsl.mtlx"
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Test shader generation from shader ref
        mx::ShaderPtr shader = desc.shadergen->generate(material->getName(), shaderRef);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out a .dot file for visualization
        std::ofstream file;
        std::string dot = mx::printGraphDot(*shader->getNodeGraph());
        file.open(shader->getNodeGraph()->getName() + ".dot");
        file << dot;
        file.close();

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("BSDF Layering", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("BsdfLayering");

    // Diffuse component
    mx::NodePtr diffuse = nodeGraph->addNode("diffusebsdf", "diffuse", "BSDF");
    mx::InputPtr diffuse_color = diffuse->addInput("reflectance", "color3");
    diffuse_color->setPublicName("diffuse_color");
    diffuse_color->setValueString("0.9, 0.1, 0.1");

    // Translucent (thin walled SSS) component
    mx::NodePtr sss = nodeGraph->addNode("translucentbsdf", "sss", "BSDF");
    mx::InputPtr sss_color = sss->addInput("transmittance", "color3");
    sss_color->setPublicName("sss_color");
    sss_color->setValueString("0.1, 0.1, 0.8");

    // Layer diffuse over sss
    mx::NodePtr substrate = nodeGraph->addNode("layeredbsdf", "substrate", "BSDF");
    mx::NodePtr substrate_weight_inv = nodeGraph->addNode("invert", "substrate_weight_inv", "float");
    substrate->setConnectedNode("top", diffuse);
    substrate->setConnectedNode("base", sss);
    substrate->setConnectedNode("weight", substrate_weight_inv);
    mx::InputPtr sss_weight = substrate_weight_inv->addInput("in", "float");
    sss_weight->setPublicName("sss_weight");
    sss_weight->setValueString("0.5");

    // Add a coating specular component on top
    mx::NodePtr coating = nodeGraph->addNode("coatingbsdf", "coating", "BSDF");
    coating->setConnectedNode("base", substrate);
    mx::InputPtr coating_color = coating->addInput("reflectance", "color3");
    coating_color->setPublicName("coating_color");
    coating_color->setValueString("1.0, 1.0, 1.0");
    mx::InputPtr coating_roughness = coating->addInput("roughness", "float");
    coating_roughness->setPublicName("coating_roughness");
    coating_roughness->setValueString("0.2");
    mx::InputPtr coating_ior = coating->addInput("ior", "float");
    coating_ior->setPublicName("coating_ior");
    coating_ior->setValueString("1.52");

    // Create a surface shader
    mx::NodePtr surface = nodeGraph->addNode("surface", "surface1", "surfaceshader");
    surface->setConnectedNode("bsdf", coating);

    // Connect to graph output
    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(surface);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::ArnoldShaderGenerator::creator(), "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_osl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_osl.mtlx",
            }
        },
        { mx::OgsFxShaderGenerator::creator(), "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_glsl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_glsl.mtlx",
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = desc.shadergen->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Transparency", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("Transparency");

    mx::NodePtr refraction = nodeGraph->addNode("refractionbsdf", "refraction", "BSDF");
    mx::InputPtr transmittance = refraction->addInput("transmittance", "color3");
    transmittance->setPublicName("transmittance");
    transmittance->setValueString("0.0, 0.0, 0.0");

    mx::NodePtr coating = nodeGraph->addNode("coatingbsdf", "coating", "BSDF");
    coating->setConnectedNode("base", refraction);
    mx::InputPtr coating_color = coating->addInput("reflectance", "color3");
    coating_color->setPublicName("coating_color");
    coating_color->setValueString("1.0, 1.0, 1.0");

    mx::NodePtr ior_common = nodeGraph->addNode("constant", "ior_common", "float");
    mx::ParameterPtr ior = ior_common->addParameter("value", "float");
    ior->setPublicName("ior");
    ior->setValueString("1.52");
    coating->setConnectedNode("ior", ior_common);
    refraction->setConnectedNode("ior", ior_common);

    mx::NodePtr roughness_common = nodeGraph->addNode("constant", "roughness_common", "float");
    mx::ParameterPtr roughness = roughness_common->addParameter("value", "float");
    roughness->setPublicName("roughness");
    roughness->setValueString("0.2");
    coating->setConnectedNode("roughness", roughness_common);
    refraction->setConnectedNode("roughness", roughness_common);

    mx::NodePtr surface = nodeGraph->addNode("surface", "surface1", "surfaceshader");
    surface->setConnectedNode("bsdf", coating);
    mx::InputPtr opacity = surface->addInput("opacity", "float");
    opacity->setPublicName("opacity");
    opacity->setValueString("1.0");

    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(surface);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::ArnoldShaderGenerator::creator(), "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_osl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_osl.mtlx",
            }
        },
        { mx::OgsFxShaderGenerator::creator(), "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_glsl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_glsl.mtlx",
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = desc.shadergen->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("LayeredSurface", "[shadergen]")
{
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx",
        "documents/Libraries/adsk/adsk_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("LayeredSurface");

    // Create first surface layer from a surface with two BSDF's
    mx::NodePtr layer1_diffuse  = nodeGraph->addNode("diffusebsdf", "layer1_diffuse", "BSDF");
    mx::InputPtr layer1_diffuse_color = layer1_diffuse->addInput("reflectance", "color3");
    layer1_diffuse_color->setPublicName("layer1_diffuse");
    layer1_diffuse_color->setValueString("0.2, 0.9, 0.2");
    mx::NodePtr layer1_specular = nodeGraph->addNode("coatingbsdf", "layer1_specular", "BSDF");
    layer1_specular->setConnectedNode("base", layer1_diffuse);
    mx::InputPtr layer1_specular_color = layer1_specular->addInput("reflectance", "color3");
    layer1_specular_color->setPublicName("layer1_specular");
    layer1_specular_color->setValueString("1.0, 1.0, 1.0");
    mx::NodePtr layer1 = nodeGraph->addNode("surface", "layer1", "surfaceshader");
    layer1->setConnectedNode("bsdf", layer1_specular);

    // Create second surface layer from an uber shader
    mx::NodePtr layer2 = nodeGraph->addNode("adskSurface", "layer2", "surfaceshader");
    mx::InputPtr layer2_diffuse_color = layer2->addInput("base_color", "color3");
    layer2_diffuse_color->setPublicName("layer2_diffuse");
    layer2_diffuse_color->setValueString("0.9, 0.1, 0.2");
    mx::InputPtr layer2_specular_color = layer2->addInput("specular_color", "color3");
    layer2_specular_color->setPublicName("layer2_specular");
    layer2_specular_color->setValueString("1.0, 1.0, 1.0");

    // Create layer mixer
    mx::NodePtr mixer = nodeGraph->addNode("layeredsurface", "mixer", "surfaceshader");
    mixer->setConnectedNode("top", layer2);
    mixer->setConnectedNode("base", layer1);
    mx::InputPtr mix_weight = mixer->addInput("weight", "float");
    mix_weight->setPublicName("mix_weight");
    mix_weight->setValueString("1.0");

    // Connect to graph output
    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(mixer);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { mx::OgsFxShaderGenerator::creator(), "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_glsl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_glsl.mtlx",
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = desc.shadergen->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}
