#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenRegistry.h>
#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>
#include <MaterialXShaderGen/NodeImplementations/VDirection.h>
#include <MaterialXShaderGen/NodeImplementations/Swizzle.h>

#include <iostream>
#include <fstream>

namespace mx = MaterialX;

struct GeneratorDescription
{
    std::string _language;
    std::string _target;
    std::string _fileExt;
    std::vector<std::string> _implementationLibrary;
};

TEST_CASE("Registry", "[shadergen]")
{
    mx::ShaderGenRegistry::registerBuiltIn();

    mx::ShaderGeneratorPtr sg1 = mx::ShaderGenRegistry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage,
        mx::ArnoldShaderGenerator::kTarget);
    REQUIRE(sg1->getTarget() == mx::ArnoldShaderGenerator::kTarget);
    
    mx::ShaderGeneratorPtr sg2 = mx::ShaderGenRegistry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage);
    REQUIRE(sg2 == nullptr);

    mx::ShaderGenRegistry::unregisterShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage, 
        mx::ArnoldShaderGenerator::kTarget);

    sg1 = mx::ShaderGenRegistry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage, 
        mx::ArnoldShaderGenerator::kTarget);
    REQUIRE(sg1 == nullptr);

    mx::NodeImplementationPtr impl1 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipOsl::kNode,
        mx::VDirectionFlipOsl::kLanguage,
        mx::VDirectionFlipOsl::kTarget);
    REQUIRE(impl1 != nullptr);

    mx::NodeImplementationPtr impl2 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipOsl::kNode,
        mx::VDirectionFlipOsl::kLanguage);
    REQUIRE(impl2 != nullptr);
    REQUIRE(impl2 == impl1);

    mx::NodeImplementationPtr impl3 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipOsl::kNode);
    REQUIRE(impl3 == nullptr);

    mx::NodeImplementationPtr impl4 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipGlsl::kNode,
        mx::VDirectionFlipGlsl::kLanguage,
        mx::VDirectionFlipGlsl::kTarget);
    REQUIRE(impl4 != nullptr);
    REQUIRE(impl4 != impl2);

    mx::ShaderGenRegistry::unregisterBuiltIn();

    impl4 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipGlsl::kNode,
        mx::VDirectionFlipGlsl::kLanguage,
        mx::VDirectionFlipGlsl::kTarget);
    REQUIRE(impl4 == nullptr);
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
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

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
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("foo");
    mx::NodePtr add = nodeGraph->addNode("add", "", "color3");
    mx::NodePtr constant = nodeGraph->addNode("constant", "bar", "color3");
    mx::NodePtr swizzle = nodeGraph->addNode("swizzle", "swizzle", "color3");
    swizzle->setConnectedNode("in", constant);
    swizzle->setParameterValue("channels", std::string("rrr"));
    add->setConnectedNode("in1", swizzle);

    // Test swizzle node custom implementation
    mx::Swizzle swizzleNode;

    mx::Shader test1("test1");
    swizzleNode.emitCode(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test1);
    REQUIRE(test1.getSourceCode() == "color foo_swizzle = color(foo_bar[0], foo_bar[0], foo_bar[0]);\n");

    swizzle->setParameterValue("channels", std::string("b0b"));

    mx::Shader test2("test2");
    swizzleNode.emitCode(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test2);
    REQUIRE(test2.getSourceCode() == "color foo_swizzle = color(foo_bar[2], 0, foo_bar[2]);\n");
}

TEST_CASE("Simple Nodegraph Shader Generation", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

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
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("test1");
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
        // language,  target,    file ext,  implementation library
        { "osl",      "arnold",  "osl",     {"documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx"} },
        { "glsl",     "ogsfx",   "ogsfx",   {"documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx"} }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_graphoutput." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();

        // Test shader generation from a node
        shader = sg->generate(nodeGraph->getName(), multiply);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_node." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Subgraph Shader Generation", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries/stdlib");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load example file
    searchPath += ";";
    searchPath += mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples");
    std::vector<std::string> filenames =
    {
        "SubGraphs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename, searchPath, true);
    }

    // Get a node graph from the example file
    mx::NodeGraphPtr nodeGraph = doc->getNodeGraph("subgraph_ex1");
    mx::OutputPtr output = nodeGraph->getOutput("out");

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = mx::printGraphDot(nodeGraph);
    file.open(nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        // language,  target,    file ext,  implementation library
        { "osl",      "arnold",  "osl",     { "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx" } },
        { "glsl",     "ogsfx",   "ogsfx",   { "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx" } }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Material Shader Generation", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

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
    mx::MaterialPtr material = doc->addMaterial();
    mx::ShaderRefPtr shaderRef = material->addShaderRef("adskSurface1", "adskSurface");
    mx::BindInputPtr base = shaderRef->addBindInput("base", "float");
    mx::BindInputPtr baseColor = shaderRef->addBindInput("base_color", "color3");
    base->setValue(0.8f);
    baseColor->setConnectedOutput(output);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { "osl", "arnold", "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_osl.mtlx"
            }
        },
        { "glsl", "ogsfx", "ogsfx",
            { 
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_glsl.mtlx"
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }
        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(shaderRef->getName(), shaderRef);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "_shaderref." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

