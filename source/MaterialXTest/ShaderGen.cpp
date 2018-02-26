#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Osl/Arnold/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Osl/OslSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/HwShader.h>

#include <fstream>

namespace mx = MaterialX;

struct GeneratorDescription
{
    mx::ShaderGeneratorPtr shadergen;
    std::string fileExt;
    std::vector<std::string> implementationLibrary;
    size_t outputStage;
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

// Observer to add library markers to all nodes. Use the "library" meta-data attribute to do this,
// until namespace specification is resolved.
static std::string LIBRARY_ATTRIBUTE("library");
class LibraryObserver : public mx::Observer
{
public:
    LibraryObserver() {}

    void onAddElement(mx::ElementPtr /*parent*/, mx::ElementPtr element) override
    {
        if (_libraryRefName.length())
        {
            element->setAttribute(LIBRARY_ATTRIBUTE, _libraryRefName);
        }
    }

    void setLibraryRefName(const std::string& name)
    {
        _libraryRefName = name;
    }

protected:
    // Libraryref name to add
    std::string _libraryRefName;
};

//
// Get source content, source path and resolved paths for
// an implementation
//
bool getShaderSource(mx::ShaderGeneratorPtr generator,
                    const mx::ImplementationPtr implementation,
                    mx::FilePath& sourcePath,
                    mx::FilePath& resolvedPath,
                    std::string& sourceContents) 
{
    if (implementation)
    {
        sourcePath = implementation->getFile();
        resolvedPath = generator->findSourceCode(sourcePath);
        if (mx::readFile(resolvedPath.asString(), sourceContents))
        {
            return true;
        }
    }
    return false;
}

//
// Find the implementation for specific language or target for a given Element.
// If the Element is a NodeGraph then only the target is checked as NodeGraph have
// no language specific implementations.
// Returned is either a reference to a NodeGraph or a Implementation.
//
mx::InterfaceElementPtr findImplementation(mx::DocumentPtr document, const std::string& name,
                                                const std::string& language, const std::string& target)
{
    std::vector<mx::InterfaceElementPtr> impllist = document->getMatchingImplementations(name);
    for (auto element : impllist)
    {
        // Check direct implementation
        mx::ImplementationPtr implementation = element->asA<mx::Implementation>();
        if (implementation)
        {
            // Check for a language match
            const std::string& lang = implementation->getLanguage();
            if (lang.length() && lang == language)
            {
                // Check target. Note that it may be empty.
                const std::string& matchingTarget = implementation->getTarget();
                if (matchingTarget.empty() || matchingTarget == target)
                {
                    return implementation;
                }
            }
        }

        // Check for a nodegraph implementation
        else if (element->isA<mx::NodeGraph>())
        {
            mx::NodeGraphPtr implGraph = element->asA<mx::NodeGraph>();
            const std::string& matchingTarget = implGraph->getTarget();
            if (matchingTarget.empty() || matchingTarget == target)
            {
                return implGraph;
            }
        }
    }
    return nullptr;
}

// Check if a nodedef requires an implementation check
// Untyped nodes do not
bool requiresImplementation(const mx::NodeDefPtr nodeDef) 
{

    if (!nodeDef)
        return false;

    static std::string TYPE_NONE("none");
    const std::string typeAttribute = nodeDef->getAttribute(mx::Element::TYPE_ATTRIBUTE);
    return !typeAttribute.empty() && typeAttribute != TYPE_NONE;
}

// Identifiers for supported light types.
enum LightTypes
{
    DIRECTIONAL_LIGHT,
    POINT_LIGHT
};

// Light shader nodes to bind to each light type.
const std::vector<std::pair<int, std::string>> LIGHT_SHADERS =
{
    { DIRECTIONAL_LIGHT, "ND_directionallight" },
    { POINT_LIGHT, "ND_pointlight" }
};

// Bind the supported light types to corresponding light shaders
// for the given shader generator.
void bindLightShaders(mx::DocumentPtr document, mx::HwShaderGenerator& shadergen)
{
    for (auto lightShader : LIGHT_SHADERS)
    {
        mx::InterfaceElementPtr implElement = findImplementation(document, lightShader.second, shadergen.getLanguage(), shadergen.getTarget());
        REQUIRE(implElement != nullptr);

        mx::SgImplementationPtr impl = shadergen.getImplementation(implElement);
        REQUIRE(impl != nullptr);

        shadergen.bindLightShader(lightShader.first, impl);
    }
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

    mx::DocumentPtr doc = mx::createDocument();
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx"
    };
    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::ArnoldShaderGenerator sg;
    sg.registerSourceCodeSearchPath(searchPath);
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
    mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
    constant1->setParameterValue("value", mx::Color3(1, 2, 3));
    mx::NodePtr swizzle1 = nodeGraph->addNode("swizzle", "swizzle1", "color3");
    swizzle1->setConnectedNode("in", constant1);
    swizzle1->setParameterValue("channels", std::string("rrr"));
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(swizzle1);

    // Test swizzle node implementation
    mx::Shader test1("test1");
    test1.initialize(output1, sg);
    mx::SgNode* sgNode = test1.getNodeGraph()->getNode("swizzle1");
    test1.addFunctionCall(sgNode, sg);
    const std::string test1Result =
        "color swizzle1_in = color(1, 2, 3);\n"
        "color swizzle1_out = color(swizzle1_in[0], swizzle1_in[0], swizzle1_in[0]);\n";
    REQUIRE(test1.getSourceCode() == test1Result);

    // Change swizzle pattern and test again
    swizzle1->setParameterValue("channels", std::string("b0b"));
    mx::Shader test2("test2");
    test2.initialize(output1, sg);
    sgNode = test2.getNodeGraph()->getNode("swizzle1");
    test2.addFunctionCall(sgNode, sg);
    const std::string test2Result =
        "color swizzle1_in = color(1, 2, 3);\n"
        "color swizzle1_out = color(swizzle1_in[2], 0, swizzle1_in[2]);\n";
    REQUIRE(test2.getSourceCode() == test2Result);
}

TEST_CASE("Simple Nodegraph Shader Generation", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx"
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

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Arnold
    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::creator();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = shadergen->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_graphoutput.osl");
        file << shader->getSourceCode();
        file.close();

        // Test shader generation from a node
        shader = shadergen->generate(nodeGraph->getName(), multiply);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_node.osl");
        file << shader->getSourceCode();
        file.close();
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shadergen = mx::OgsFxShaderGenerator::creator();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = shadergen->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_graphoutput.ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        file.close();

        // Test shader generation from a node
        shader = shadergen->generate(nodeGraph->getName(), multiply);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_node.ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        file.close();
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shadergen = mx::GlslShaderGenerator::creator();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = shadergen->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::PIXEL_STAGE).length() > 0);
        file.open(shader->getName() + "_graphoutput.vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shader->getName() + "_graphoutput.frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // Test shader generation from a node
        shader = shadergen->generate(nodeGraph->getName(), multiply);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::PIXEL_STAGE).length() > 0);
        file.open(shader->getName() + "_node.vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shader->getName() + "_node.frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
    }
}

TEST_CASE("Conditional Nodegraph Shader Generation", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a simple node graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("conditional_test1");

    mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
    constant1->setParameterValue("value", mx::Color3(0, 0, 0));
    mx::NodePtr constant2 = nodeGraph->addNode("constant", "constant2", "color3");
    constant2->setParameterValue("value", mx::Color3(1, 1, 1));
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

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getNodeGraph()->getNodes().empty());
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + ".osl");
        file << shader->getSourceCode();
        file.close();
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getNodeGraph()->getNodes().empty());
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode();
        file.close();
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getNodeGraph()->getNodes().empty());
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
    }
}

TEST_CASE("Geometric Nodes", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a nonsensical graph testing some geometric nodes
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("geometric_nodes");

    mx::NodePtr normal1 = nodeGraph->addNode("normal", "normal1", "vector3");
    normal1->setParameterValue("space", std::string("world"));

    mx::NodePtr position1 = nodeGraph->addNode("position", "position1", "vector3");
    position1->setParameterValue("space", std::string("world"));

    mx::NodePtr texcoord1 = nodeGraph->addNode("texcoord", "texcoord1", "vector2");
    texcoord1->setParameterValue("index", 0, "integer");

    mx::NodePtr geomcolor1 = nodeGraph->addNode("geomcolor", "geomcolor1", "color3");
    geomcolor1->setParameterValue("index", 0, "integer");

    mx::NodePtr geomattrvalue1 = nodeGraph->addNode("geomattrvalue", "geomattrvalue1", "float");
    geomattrvalue1->setParameterValue("attrname", std::string("temperature"));

    mx::NodePtr add1 = nodeGraph->addNode("add", "add1", "vector3");
    add1->setConnectedNode("in1", normal1);
    add1->setConnectedNode("in2", position1);

    mx::NodePtr multiply1 = nodeGraph->addNode("multiply", "multiply1", "color3");
    multiply1->setConnectedNode("in1", geomcolor1);
    multiply1->setConnectedNode("in2", geomattrvalue1);

    mx::NodePtr convert1 = nodeGraph->addNode("swizzle", "convert1", "color3");
    convert1->setConnectedNode("in", add1);
    convert1->setParameterValue("channels", std::string("xyz"));

    mx::NodePtr multiply2 = nodeGraph->addNode("multiply", "multiply2", "color3");
    multiply2->setConnectedNode("in1", convert1);
    multiply2->setConnectedNode("in2", multiply1);

    mx::NodePtr time1 = nodeGraph->addNode("time", "time1", "float");
    mx::NodePtr multiply3 = nodeGraph->addNode("multiply", "multiply3", "color3");
    multiply3->setConnectedNode("in1", multiply2);
    multiply3->setConnectedNode("in2", time1);

    mx::NodePtr frame1 = nodeGraph->addNode("frame", "frame1", "float");
    mx::NodePtr multiply4 = nodeGraph->addNode("multiply", "multiply4", "color3");
    multiply4->setConnectedNode("in1", multiply3);
    multiply4->setConnectedNode("in2", frame1);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput(mx::EMPTY_STRING, "color3");
    output1->setConnectedNode(multiply4);

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}

TEST_CASE("Subgraph Shader Generation", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Load example files
    std::string searchPath1 = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples;");
    searchPath1 += mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries/stdlib;");
    searchPath1 += mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries/sx");
    std::vector<std::string> filenames =
    {
        "SubGraphs.mtlx",
        "BsdfSubGraphs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
        "documents/Libraries/sx/impl/shadergen/osl/impl.mtlx",
        "documents/Libraries/sx/impl/shadergen/glsl/impl.mtlx"
    };
    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename, searchPath1);
    }

    mx::FilePath searchPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    std::vector<std::string> exampleGraphNames = { "subgraph_ex1" , "subgraph_ex2" };

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath2);

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            // TODO: Match against blessed versions
            std::ofstream file;
            file.open(shader->getName() + ".osl");
            file << shader->getSourceCode();
        }
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath2);

        mx::HwShaderGenerator* hwShaderGenerator = static_cast<mx::HwShaderGenerator*>(shaderGenerator.get());
        bindLightShaders(doc, *hwShaderGenerator);

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Match against blessed versions
            std::ofstream file;
            file.open(shader->getName() + ".ogsfx");
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        }
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::creator();
        shaderGenerator->registerSourceCodeSearchPath(searchPath2);

        mx::HwShaderGenerator* hwShaderGenerator = static_cast<mx::HwShaderGenerator*>(shaderGenerator.get());
        bindLightShaders(doc, *hwShaderGenerator);

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output);
            REQUIRE(shader != nullptr);

            REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
            REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Match against blessed versions
            std::ofstream file;
            file.open(shader->getName() + ".frag");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();
            file.open(shader->getName() + ".vert");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        }
    }
}

TEST_CASE("Material Shader Generation", "[shadergen]")
{
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
    // Set opacity for transparency check
    mx::BindInputPtr baseOpacity = shaderRef->addBindInput("opacity", "color3");
    baseOpacity->setValueString("0.8, 0.8, 0.8");

    mx::ShaderGeneratorPtr arnoldShaderGenerator = mx::ArnoldShaderGenerator::creator();
    mx::ShaderGeneratorPtr ogsfxShaderGenerator = mx::OgsFxShaderGenerator::creator();
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    arnoldShaderGenerator->registerSourceCodeSearchPath(searchPath);
    ogsfxShaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { arnoldShaderGenerator, "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/osl/impl.mtlx"
            },
            mx::Shader::PIXEL_STAGE
        },
        { ogsfxShaderGenerator, "ogsfx",
            { 
                "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/glsl/impl.mtlx"
            },
            mx::OgsFxShader::FINAL_FX_STAGE
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
        // For now only ogsfx has transparency detection set up
        if (desc.fileExt == "ogsfx")
        {
            REQUIRE(shader->isTransparent());
        }

        // Write out a .dot file for visualization
        std::ofstream file;
        std::string dot = mx::printGraphDot(*shader->getNodeGraph());
        file.open(shader->getNodeGraph()->getName() + ".dot");
        file << dot;
        file.close();

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode(desc.outputStage);
        file.close();
    }
}

TEST_CASE("BSDF Layering", "[shadergen]")
{
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

    mx::ShaderGeneratorPtr arnoldShaderGenerator = mx::ArnoldShaderGenerator::creator();
    mx::ShaderGeneratorPtr ogsfxShaderGenerator = mx::OgsFxShaderGenerator::creator();
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    arnoldShaderGenerator->registerSourceCodeSearchPath(searchPath);
    ogsfxShaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { arnoldShaderGenerator, "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/osl/impl.mtlx",
                "documents/Libraries/sx/impl/shadergen/osl/impl.mtlx",
            },
            mx::Shader::PIXEL_STAGE
        },
        { ogsfxShaderGenerator, "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/sx/impl/shadergen/glsl/impl.mtlx",
            },
            mx::OgsFxShader::FINAL_FX_STAGE
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
        file << shader->getSourceCode(desc.outputStage);
        file.close();
    }
}

TEST_CASE("Transparency", "[shadergen]")
{
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
    opacity->setValueString("0.8");

    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(surface);

    mx::ShaderGeneratorPtr arnoldShaderGenerator = mx::ArnoldShaderGenerator::creator();
    mx::ShaderGeneratorPtr ogsfxShaderGenerator = mx::OgsFxShaderGenerator::creator();
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    arnoldShaderGenerator->registerSourceCodeSearchPath(searchPath);
    ogsfxShaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { arnoldShaderGenerator, "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/osl/impl.mtlx",
                "documents/Libraries/sx/impl/shadergen/osl/impl.mtlx",
            },
            mx::Shader::PIXEL_STAGE
        },
        { ogsfxShaderGenerator, "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/sx/impl/shadergen/glsl/impl.mtlx",
            },
            mx::OgsFxShader::FINAL_FX_STAGE
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
        // For now only ogsfx has transparency detection set up
        if (desc.fileExt == "ogsfx")
        {
            REQUIRE(shader->isTransparent());
        }

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "." + desc.fileExt);
        file << shader->getSourceCode(desc.outputStage);
        file.close();
    }
}

TEST_CASE("LayeredSurface", "[shadergen]")
{
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

    mx::ShaderGeneratorPtr ogsfxShaderGenerator = mx::OgsFxShaderGenerator::creator();
    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    ogsfxShaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { ogsfxShaderGenerator, "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/glsl/impl.mtlx",
                "documents/Libraries/sx/impl/shadergen/glsl/impl.mtlx",
            },
            mx::OgsFxShader::FINAL_FX_STAGE
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
        file << shader->getSourceCode(desc.outputStage);
        file.close();
    }
}

TEST_CASE("Reference implementation validity", "[shadergen]")
{
    std::filebuf implDumpBuffer;
    std::string fileName = "reference_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Scanning language: osl. Target: reference" << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    mx::ObservedDocumentPtr doc = mx::Document::createDocument<mx::ObservedDocument>();

    std::shared_ptr<LibraryObserver> observer = std::make_shared<LibraryObserver>();
    observer->setLibraryRefName("stdlib");
    doc->addObserver("libraryObserver", observer);

    // Load standard libraries implemented by reference implementation
    // Note that if there are other language implementations this list should be appended to
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/reference/osl/impl.mtlx",
    };
    for (const std::string& filename : filenames)
    {
        implDumpStream << "* Read in implementation file: " << filename << std::endl;
        mx::readFromXmlFile(doc, filename);
    }

    // Set source code search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::FileSearchPath sourceCodeSearchPath; 
    sourceCodeSearchPath.append(searchPath);

    std::string nodeDefNode;
    std::string nodeDefType;
    unsigned int count = 0;
    unsigned int missing = 0;
    std::string missing_str;
    std::string found_str;

    // Scan through every nodedef defined
    for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
    {
        count++;

        const std::string language("osl");
        const std::string target("");

        std::string nodeDefName = nodeDef->getName();
        std::string nodeName = nodeDef->getNodeString();
        if (!requiresImplementation(nodeDef))
        {
            found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: "
                + nodeName + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
            continue;
        }

        mx::InterfaceElementPtr inter =
            findImplementation(doc, nodeDefName, language, target);
        if (!inter)
        {
            missing++;
            missing_str += "Missing nodeDef implemenation: " + nodeDefName + ", Node: " + nodeName
                + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
        }
        else
        {
            mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
            if (impl)
            {
                // Scan for file and see if we can read in the contents
                std::string sourceContents;
                mx::FilePath sourcePath = impl->getFile();
                mx::FilePath resolvedPath = sourceCodeSearchPath.find(sourcePath);
                bool found = mx::readFile(resolvedPath.asString(), sourceContents);
                if (!found)
                {
                    missing++;
                    missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                        + nodeDefName + ". Impl: " + impl->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                }
                else
                {
                    found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + impl->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                }
            }
            else
            {
                mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Impl: " + graph->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
            }
        }
    }

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs\n";
    implDumpStream << missing_str << std::endl;
    implDumpStream << found_str << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    implDumpBuffer.close();

    // To enable once this is true
    //REQUIRE(missing == 0);
}

TEST_CASE("Shadergen implementation validity", "[shadergen]")
{
    mx::ObservedDocumentPtr doc = mx::Document::createDocument<mx::ObservedDocument>();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx",
        "documents/Libraries/adsk/adsk_defs.mtlx"
    };
    std::vector<std::string> libraryNames =
    {
        "stdlib",
        "sx",
        "adsk"
    };
    std::shared_ptr<LibraryObserver> observer = std::make_shared<LibraryObserver>();
    doc->addObserver("libraryObserver", observer);
    for (size_t i=0; i<filenames.size(); i++)
    {
        observer->setLibraryRefName(libraryNames[i]);
        mx::readFromXmlFile(doc, filenames[i]);
    }

    // Register search paths
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    mx::ShaderGeneratorPtr arnoldShaderGenerator = mx::ArnoldShaderGenerator::creator();
    arnoldShaderGenerator->registerSourceCodeSearchPath(searchPath);

    mx::ShaderGeneratorPtr ogsfxShaderGenerator = mx::OgsFxShaderGenerator::creator();
    ogsfxShaderGenerator->registerSourceCodeSearchPath(searchPath);

    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { arnoldShaderGenerator, "osl",
        {
            "documents/Libraries/stdlib/impl/shadergen/osl/impl.mtlx",
            "documents/Libraries/adsk/impl/shadergen/osl/impl.mtlx",
            "documents/Libraries/sx/impl/shadergen/osl/impl.mtlx",
        },
        mx::Shader::PIXEL_STAGE
        },
        { ogsfxShaderGenerator, "ogsfx",
        {
            "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
            "documents/Libraries/adsk/impl/shadergen/glsl/impl.mtlx",
            "documents/Libraries/sx/impl/shadergen/glsl/impl.mtlx",
        },
        mx::OgsFxShader::FINAL_FX_STAGE
        }
    };


    std::filebuf implDumpBuffer;
    std::string fileName = "shadgen_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    for (auto desc : generatorDescriptions)
    {
        mx::ShaderGeneratorPtr generator = desc.shadergen;
        const std::string& language = generator->getLanguage();
        const std::string& target = generator->getTarget();

        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        implDumpStream << "Scanning language: " << language << ". Target: " << target << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;

        // Load in the implementation libraries
        for (const std::string& libfile : desc.implementationLibrary)
        {
            implDumpStream << "* Read in implementation file: " << libfile << std::endl;
            mx::readFromXmlFile(doc, libfile);
        }

        std::string nodeDefNode;
        std::string nodeDefType;
        unsigned int count = 0;
        unsigned int missing = 0;
        std::string missing_str;
        std::string found_str;

        // Scan through every nodedef defined
        for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
        {
            count++;

            std::string nodeDefName = nodeDef->getName();
            std::string nodeName = nodeDef->getNodeString();
            if (!requiresImplementation(nodeDef))
            {
                found_str += "No implementation requried for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                continue;
            }

            mx::InterfaceElementPtr inter = findImplementation(doc, nodeDefName, language, target);
            if (!inter)
            {
                missing++;
                missing_str += "Missing nodeDef implementation: " + nodeDefName + ", Node: " + nodeName
                    + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
            }
            else
            {
                mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
                if (impl)
                {
                    // Test if the generator has an interal implementation first
                    if (generator->implementationRegistered(impl->getName()))
                    {
                        found_str += "Found generator impl for nodedef: " + nodeDefName + ", Node: "
                            + nodeDefName + ". Impl: " + impl->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                    }

                    // Check for an implementation explicitly stored
                    else
                    {
                        mx::FilePath sourcePath, resolvedPath;
                        std::string contents;
                        if (!getShaderSource(generator, impl, sourcePath, resolvedPath, contents))
                        {
                            missing++;
                            missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                                + nodeDefName + ". Impl: " + impl->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                        }
                        else
                        {
                            found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                                + nodeName + +". Impl: " + impl->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                        }
                    }
                }
                else
                {
                    mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                    found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + graph->getName() + ". Library: " + nodeDef->getAttribute(LIBRARY_ATTRIBUTE) + "\n";
                }
            }
        }

        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs\n";
        implDumpStream << missing_str << std::endl;
        implDumpStream << found_str << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;

        // To enable once this is true
        //REQUIRE(missing == 0);
    }

    implDumpBuffer.close();
}
