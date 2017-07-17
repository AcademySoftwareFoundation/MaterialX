#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/Registry.h>
#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>
#include <MaterialXShaderGen/CustomImpls/VDirectionImpl.h>
#include <MaterialXShaderGen/CustomImpls/SwizzleImpl.h>

#include <iostream>
#include <fstream>

namespace mx = MaterialX;

TEST_CASE("Registry", "[ShaderGen]")
{
    mx::Registry::registerBuiltIn();

    mx::ShaderGeneratorPtr sg1 = mx::Registry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage,
        mx::ArnoldShaderGenerator::kTarget);
    REQUIRE(sg1->getTarget() == mx::ArnoldShaderGenerator::kTarget);
    
    mx::ShaderGeneratorPtr sg2 = mx::Registry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage);
    REQUIRE(sg2 == nullptr);

    mx::Registry::unregisterShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage, 
        mx::ArnoldShaderGenerator::kTarget);

    sg1 = mx::Registry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage, 
        mx::ArnoldShaderGenerator::kTarget);
    REQUIRE(sg1 == nullptr);

    mx::CustomImplPtr impl1 = mx::Registry::findImplementation(
        mx::VDirectionImplFlipOsl::kNode,
        mx::VDirectionImplFlipOsl::kLanguage,
        mx::VDirectionImplFlipOsl::kTarget);
    REQUIRE(impl1 != nullptr);

    mx::CustomImplPtr impl2 = mx::Registry::findImplementation(
        mx::VDirectionImplFlipOsl::kNode,
        mx::VDirectionImplFlipOsl::kLanguage);
    REQUIRE(impl2 != nullptr);
    REQUIRE(impl2 == impl1);

    mx::CustomImplPtr impl3 = mx::Registry::findImplementation(
        mx::VDirectionImplFlipOsl::kNode);
    REQUIRE(impl3 == nullptr);

    mx::CustomImplPtr impl4 = mx::Registry::findImplementation(
        mx::VDirectionImplFlipGlsl::kNode,
        mx::VDirectionImplFlipGlsl::kLanguage,
        mx::VDirectionImplFlipGlsl::kTarget);
    REQUIRE(impl4 != nullptr);
    REQUIRE(impl4 != impl2);

    mx::Registry::unregisterBuiltIn();

    impl4 = mx::Registry::findImplementation(
        mx::VDirectionImplFlipGlsl::kNode,
        mx::VDirectionImplFlipGlsl::kLanguage,
        mx::VDirectionImplFlipGlsl::kTarget);
    REQUIRE(impl4 == nullptr);
}

TEST_CASE("OslSyntax", "[ShaderGen]")
{
    mx::SyntaxPtr syntax = std::make_shared<mx::OslSyntax>();

    REQUIRE(syntax->getTypeName("float") == "float");
    REQUIRE(syntax->getTypeName("color3") == "color");
    REQUIRE(syntax->getTypeName("vector3") == "vector");

    REQUIRE(syntax->getTypeName("BSDF") == "BSDF");
    REQUIRE(syntax->getOutputTypeName("BSDF") == "output closure color");

    std::string dv = syntax->getTypeDefault("float");
    REQUIRE(dv == "0.0");
    dv = syntax->getTypeDefault("color3", false);
    REQUIRE(dv == "color(0.0, 0.0, 0.0)");
    dv = syntax->getTypeDefault("color3", true);
    REQUIRE(dv == "color(0.0, 0.0, 0.0)");
}

TEST_CASE("Simple Shader Generation", "[ShaderGen]")
{
    mx::ScopedRegistryInit registryInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::addSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/mx_stdlib_defs.mtlx"
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

    // Setup descriptions of the shader generators
    using GeneratorDescription = std::tuple<std::string, std::string, std::string, std::string>;
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        // language,  target,    file ext,  implementation library
        { "osl",      "arnold",  "osl",     "documents/Libraries/mx_stdlib_impl_shadergen_osl.mtlx"  },
        { "glsl",     "ogsfx",   "ogsfx",   "documents/Libraries/mx_stdlib_impl_shadergen_glsl.mtlx" }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the library of implementations
        mx::readFromXmlFile(doc, std::get<3>(desc));

        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::Registry::findShaderGenerator(std::get<0>(desc), std::get<1>(desc));
        REQUIRE(sg != nullptr);

        // Generate the shader
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output1->getConnectedNode(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        file.open(shader->getName() + "." + std::get<2>(desc));
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Swizzling", "[ShaderGen]")
{
    mx::ScopedRegistryInit registryInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenerator::addSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();
    std::vector<std::string> filenames =
    {
        "documents/Libraries/mx_stdlib_defs.mtlx",
        "documents/Libraries/mx_stdlib_impl_shadergen_osl.mtlx"
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
    REQUIRE(var2 == "_color4(foo, foo, 0, 1)");
    std::string var3 = syntax->getSwizzledVariable("foo", "color2", "vector2", "xy");
    REQUIRE(var3 == "color(foo[0], foo[1], 0)");

    // Create a simple test graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("foo");
    mx::NodePtr add = nodeGraph->addNode("add", "", "color3");
    mx::NodePtr constant = nodeGraph->addNode("constant", "bar", "color3");
    mx::NodePtr swizzle = nodeGraph->addNode("swizzle", "swizzle", "color3");
    swizzle->setConnectedNode("in", constant);
    swizzle->setParameterValue("channels", std::string("rrr"));
    add->setConnectedNode("in1", swizzle);

    // Test swizzle node custom implementation
    mx::SwizzleImpl swizzleImpl;

    mx::Shader test1("test1");
    swizzleImpl.emitCode(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test1);
    REQUIRE(test1.getSourceCode() == "color foo_swizzle = color(foo_bar[0], foo_bar[0], foo_bar[0]);\n");

    swizzle->setParameterValue("channels", std::string("b0b"));

    mx::Shader test2("test2");
    swizzleImpl.emitCode(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test2);
    REQUIRE(test2.getSourceCode() == "color foo_swizzle = color(foo_bar[2], 0, foo_bar[2]);\n");
}

