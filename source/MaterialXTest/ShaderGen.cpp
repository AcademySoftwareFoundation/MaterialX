#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/Registry.h>
#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
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
        "documents/Libraries/mx_stdlib_defs.mtlx",
        "documents/Libraries/mx_stdlib_impl_shadergen_osl.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a simple node graph
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_testgraph", "color3");
    mx::ParameterPtr param1 = nodeDef->addParameter("color1", "color3");
    mx::ParameterPtr param2 = nodeDef->addParameter("color2", "color3");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("testgraph");
    nodeGraph->setNodeDef(nodeDef->getName());

    mx::NodePtr add = nodeGraph->addNode("add", "add1", "color3");
    mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
    mx::NodePtr image1 = nodeGraph->addNode("image", "image1", "color3");
    constant1->setParameterValue("value", mx::Color3(1, 0, 0));
    image1->setParameterValue("file", std::string("/foo/bar.exr"));
    add->setConnectedNode("in1", constant1);
    add->setConnectedNode("in2", image1);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(add);
    output1->setChannels("rrr");

    // Generate an OSL shader from the output
    mx::ArnoldShaderGenerator sg;
    mx::ShaderPtr shader = sg.generate(output1->getConnectedNode(), output1);

    REQUIRE(shader != nullptr);
    REQUIRE(shader->getSourceCode().length() > 0);

    std::ofstream stream;
    stream.open(nodeGraph->getName() + ".osl");
    stream << shader->getSourceCode();
    stream.close();

    std::string dot = mx::printGraphDot(nodeGraph);
    stream.open(nodeGraph->getName() + ".dot");
    stream << dot;
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

    mx::Shader test1;
    swizzleImpl.emitCode(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test1);
    REQUIRE(test1.getSourceCode() == "color foo_swizzle = color(foo_bar[0], foo_bar[0], foo_bar[0]);\n");

    swizzle->setParameterValue("channels", std::string("b0b"));

    mx::Shader test2;
    swizzleImpl.emitCode(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test2);
    REQUIRE(test2.getSourceCode() == "color foo_swizzle = color(foo_bar[2], 0, foo_bar[2]);\n");
}

