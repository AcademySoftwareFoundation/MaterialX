#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/HwLightHandler.h>

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OGSFX
#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>
#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/ArnoldShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>
#endif

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>

namespace mx = MaterialX;

void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc)
{
    const std::string MTLX_EXTENSION("mtlx");
    for (const std::string& library : libraryNames)
    {
        mx::FilePath path = searchPath / library;
        mx::StringVec filenames;
        mx::getFilesInDirectory(path.asString(), filenames, MTLX_EXTENSION);

        for (const std::string& filename : filenames)
        {
            mx::FilePath file = path / filename;
            mx::DocumentPtr libDoc = mx::createDocument();
            mx::readFromXmlFile(libDoc, file);
            libDoc->setSourceUri(file);
            mx::CopyOptions copyOptions;
            copyOptions.skipDuplicateElements = true;
            doc->importLibrary(libDoc, &copyOptions);
        }
    }
    REQUIRE(doc->getNodeDefs().size() > 0);
}

void loadExamples(const mx::StringVec& exampleNames, const mx::FilePath& examplesPath, const mx::FilePath searchPath, mx::DocumentPtr doc)
{
    try
    {
        for (const std::string& filename : exampleNames)
        {
            mx::FilePath file = examplesPath / filename;
            mx::readFromXmlFile(doc, file, searchPath);
        }
    }
    catch (mx::Exception e)
    {
    }
}


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

void createLightCompoundExample(mx::DocumentPtr document)
{
    const std::string nodeName = "lightcompound";
    const std::string nodeDefName = "ND_" + nodeName;

    // Make sure it doesn't exists already
    if (!document->getNodeDef(nodeDefName))
    {
        // Create an interface for the light with position, color and intensity
        mx::NodeDefPtr nodeDef = document->addNodeDef(nodeDefName, "lightshader", nodeName);
        nodeDef->addInput("position", "vector3");
        nodeDef->addInput("color", "color3");
        nodeDef->addInput("intensity", "float");

        // Create a graph implementing the light using EDF's
        mx::NodeGraphPtr nodeGraph = document->addNodeGraph("IMP_" + nodeName);
        mx::OutputPtr output = nodeGraph->addOutput("out", "lightshader");

        // Add EDF node and connect the EDF's intensity to the 'color' input
        mx::NodePtr edf = nodeGraph->addNode("uniformedf", "edf1", "EDF");
        mx::InputPtr edf_intensity = edf->addInput("intensity", "color3");
        edf_intensity->setInterfaceName("color");

        // Add the light constructor node connect it's intensity to the 'intensity' input
        mx::NodePtr light = nodeGraph->addNode("light", "light1", "lightshader");
        mx::InputPtr light_intensity = light->addInput("intensity", "float");
        light_intensity->setInterfaceName("intensity");

        // Connect the EDF to the light constructor
        light->setConnectedNode("edf", edf);

        // Connect the light to the graph output
        output->setConnectedNode(light);

        // Make this graph become the implementation of our nodedef
        nodeGraph->setAttribute("nodedef", nodeDef->getName());
    }
}

float cosAngle(float degrees)
{
    static const float PI = 3.14159265f;
    return cos(degrees * PI / 180.0f);
}

// Light type id's for common light shaders
// Using id's matching the OgsFx light sources
// here which simplifies light binding for OGS.
// Note that another target systems could use other ids
// as required by that system.
enum LightType
{
    SPOT = 2,
    POINT = 3,
    DIRECTIONAL = 4,
};

void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen)
{
    // Create a custom light shader by a graph compound
    createLightCompoundExample(doc);

    mx::NodeDefPtr dirLightNodeDef = doc->getNodeDef("ND_directionallight");
    mx::NodeDefPtr pointLightNodeDef = doc->getNodeDef("ND_pointlight");
    mx::NodeDefPtr spotLightNodeDef = doc->getNodeDef("ND_spotlight");
    mx::NodeDefPtr compoundLightNodeDef = doc->getNodeDef("ND_lightcompound");
    REQUIRE(dirLightNodeDef != nullptr);
    REQUIRE(pointLightNodeDef != nullptr);
    REQUIRE(spotLightNodeDef != nullptr);
    REQUIRE(compoundLightNodeDef != nullptr);

    // Add the common light shaders
    lightHandler.addLightShader(LightType::DIRECTIONAL, dirLightNodeDef);
    lightHandler.addLightShader(LightType::POINT, pointLightNodeDef);
    lightHandler.addLightShader(LightType::SPOT, spotLightNodeDef);

    // Add our custom coumpund light shader
    const size_t compoundLightId = 42;
    lightHandler.addLightShader(compoundLightId, compoundLightNodeDef);

    // Create a light rig with one light source for each light shader

    mx::LightSourcePtr dirLight = lightHandler.createLightSource(LightType::DIRECTIONAL);
    dirLight->setParameter("direction", mx::Vector3(0, 0, -1));
    dirLight->setParameter("color", mx::Color3(1, 1, 1));
    dirLight->setParameter("intensity", 0.2f);

    mx::LightSourcePtr pointLight = lightHandler.createLightSource(LightType::POINT);
    pointLight->setParameter("position", mx::Vector3(-2, -2, 2));
    pointLight->setParameter("color", mx::Color3(0, 0.0, 1));
    pointLight->setParameter("intensity", 10.0f);
    pointLight->setParameter("decayRate", 3.0f);

    mx::LightSourcePtr spotLight = lightHandler.createLightSource(LightType::SPOT);
    mx::Vector3 position(3, 3, 3);
    spotLight->setParameter("position", position);
    mx::Vector3 direction = position.getNormalized() * -1;;
    spotLight->setParameter("direction", direction);
    spotLight->setParameter("color", mx::Color3(1, 0, 0));
    spotLight->setParameter("intensity", 1.0f);
    spotLight->setParameter("decayRate", 0.0f);
    spotLight->setParameter("innerConeAngle", cosAngle(5.0f));
    spotLight->setParameter("outerConeAngle", cosAngle(10.0f));

    mx::LightSourcePtr compoundLight = lightHandler.createLightSource(compoundLightId);
    position = { -3, 3, 3 };
    direction = position.getNormalized() * -1;
    compoundLight->setParameter("position", position);
    compoundLight->setParameter("direction", direction);
    compoundLight->setParameter("color", mx::Color3(0, 1, 0));
    compoundLight->setParameter("intensity", 10.0f);

    // Let the shader generator know of these light shaders
    lightHandler.bindLightShaders(shadergen);

    // Set up IBL inputs
    lightHandler.setLightEnvIrradiancePath("documents/TestSuite/Images/san_giuseppe_bridge_diffuse.exr");
    lightHandler.setLightEnvRadiancePath("documents/TestSuite/Images/san_giuseppe_bridge.exr");
}

static std::string RESULT_DIRECTORY("results/");
TEST_CASE("Bootstrap", "[shadergen]")
{
    // This must always come first
    mx::makeDirectory(RESULT_DIRECTORY);
}

TEST_CASE("Valid Libraries", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    std::string validationErrors;
    bool valid = doc->validate(&validationErrors);
    if (!valid)
    {
        std::cout << validationErrors << std::endl;
    }
    REQUIRE(valid);
}

TEST_CASE("Syntax", "[shadergen]")
{
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::SyntaxPtr syntax = mx::OslSyntax::create();

        REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "color");
        REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vector");
        REQUIRE(syntax->getTypeName(mx::Type::FLOATARRAY) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::INTEGERARRAY) == "int");
        REQUIRE(mx::Type::FLOATARRAY->isArray());
        REQUIRE(mx::Type::INTEGERARRAY->isArray());

        REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
        REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "output BSDF");

        // Set fixed precision with one digit
        mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

        std::string value;
        value = syntax->getDefaultValue(mx::Type::FLOAT);
        REQUIRE(value == "0.0");
        value = syntax->getDefaultValue(mx::Type::COLOR3);
        REQUIRE(value == "color(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR3, true);
        REQUIRE(value == "color(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4);
        REQUIRE(value == "color4(color(0.0), 0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4, true);
        REQUIRE(value == "{color(0.0), 0.0}");
        value = syntax->getDefaultValue(mx::Type::FLOATARRAY, true);
        REQUIRE(value.empty());
        value = syntax->getDefaultValue(mx::Type::INTEGERARRAY, true);
        REQUIRE(value.empty());

        mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
        value = syntax->getValue(mx::Type::FLOAT, *floatValue);
        REQUIRE(value == "42.0");
        value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
        REQUIRE(value == "42.0");

        mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
        value = syntax->getValue(mx::Type::COLOR3, *color3Value);
        REQUIRE(value == "color(1.0, 2.0, 3.0)");
        value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
        REQUIRE(value == "color(1.0, 2.0, 3.0)");

        mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
        value = syntax->getValue(mx::Type::COLOR4, *color4Value);
        REQUIRE(value == "color4(color(1.0, 2.0, 3.0), 4.0)");
        value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
        REQUIRE(value == "{color(1.0, 2.0, 3.0), 4.0}");

        std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
        mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
        value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
        REQUIRE(value == "{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7}");

        std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
        mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
        value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
        REQUIRE(value == "{1, 2, 3, 4, 5, 6, 7}");
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::SyntaxPtr syntax = mx::GlslSyntax::create();

        REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
        REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

        REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
        REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

        // Set fixed precision with one digit
        mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

        std::string value;
        value = syntax->getDefaultValue(mx::Type::FLOAT);
        REQUIRE(value == "0.0");
        value = syntax->getDefaultValue(mx::Type::COLOR3);
        REQUIRE(value == "vec3(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR3, true);
        REQUIRE(value == "vec3(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4);
        REQUIRE(value == "vec4(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4, true);
        REQUIRE(value == "vec4(0.0)");
        value = syntax->getDefaultValue(mx::Type::FLOATARRAY, true);
        REQUIRE(value.empty());
        value = syntax->getDefaultValue(mx::Type::INTEGERARRAY, true);
        REQUIRE(value.empty());

        mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
        value = syntax->getValue(mx::Type::FLOAT, *floatValue);
        REQUIRE(value == "42.0");
        value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
        REQUIRE(value == "42.0");

        mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
        value = syntax->getValue(mx::Type::COLOR3, *color3Value);
        REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
        value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
        REQUIRE(value == "vec3(1.0, 2.0, 3.0)");

        mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
        value = syntax->getValue(mx::Type::COLOR4, *color4Value);
        REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
        value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
        REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");

        std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
        mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
        value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
        REQUIRE(value == "float[7](0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)");

        std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
        mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
        value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
        REQUIRE(value == "int[7](1, 2, 3, 4, 5, 6, 7)");
    }
#endif // MATERIALX_BUILD_GEN_GLSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::SyntaxPtr syntax = mx::OgsFxSyntax::create();

        REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
        REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

        REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
        REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

        // Set fixed precision with one digit
        mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

        std::string value;
        value = syntax->getDefaultValue(mx::Type::FLOAT);
        REQUIRE(value == "0.0");
        value = syntax->getDefaultValue(mx::Type::COLOR3);
        REQUIRE(value == "vec3(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR3, true);
        REQUIRE(value == "{0.0, 0.0, 0.0}");
        value = syntax->getDefaultValue(mx::Type::COLOR4);
        REQUIRE(value == "vec4(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4, true);
        REQUIRE(value == "{0.0, 0.0, 0.0, 0.0}");

        mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
        value = syntax->getValue(mx::Type::FLOAT, *floatValue);
        REQUIRE(value == "42.0");
        value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
        REQUIRE(value == "42.0");

        mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
        value = syntax->getValue(mx::Type::COLOR3, *color3Value);
        REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
        value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
        REQUIRE(value == "{1.0, 2.0, 3.0}");

        mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
        value = syntax->getValue(mx::Type::COLOR4, *color4Value);
        REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
        value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
        REQUIRE(value == "{1.0, 2.0, 3.0, 4.0}");
    }
#endif // MATERIALX_BUILD_GEN_OGSFX
}

TEST_CASE("TypeDesc", "[shadergen]")
{
    // Make sure the standard types are registered
    const mx::TypeDesc* floatType = mx::TypeDesc::get("float");
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    const mx::TypeDesc* integerType = mx::TypeDesc::get("integer");
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::TypeDesc::BASETYPE_INTEGER);
    const mx::TypeDesc* booleanType = mx::TypeDesc::get("boolean");
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::TypeDesc::BASETYPE_BOOLEAN);
    const mx::TypeDesc* color2Type = mx::TypeDesc::get("color2");
    REQUIRE(color2Type != nullptr);
    REQUIRE(color2Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color2Type->getSemantic() == mx::TypeDesc::SEMATIC_COLOR);
    REQUIRE(color2Type->isFloat2());
    const mx::TypeDesc* color3Type = mx::TypeDesc::get("color3");
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::TypeDesc::SEMATIC_COLOR);
    REQUIRE(color3Type->isFloat3());
    const mx::TypeDesc* color4Type = mx::TypeDesc::get("color4");
    REQUIRE(color4Type != nullptr);
    REQUIRE(color4Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color4Type->getSemantic() == mx::TypeDesc::SEMATIC_COLOR);
    REQUIRE(color4Type->isFloat4());

    // Make sure we can register a new sutom type
    const mx::TypeDesc* fooType = mx::TypeDesc::registerType("foo", mx::TypeDesc::BASETYPE_FLOAT, mx::TypeDesc::SEMATIC_COLOR, 5);
    REQUIRE(fooType != nullptr);

    // Make sure we can't use a name already take
    REQUIRE_THROWS(mx::TypeDesc::registerType("color3", mx::TypeDesc::BASETYPE_FLOAT));

    // Make sure we can't request an unknown type
    REQUIRE_THROWS(mx::TypeDesc::get("bar"));
}

TEST_CASE("Reference Implementation Validity", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    // Set source code search path
    mx::FileSearchPath sourceCodeSearchPath;
    sourceCodeSearchPath.append(searchPath);

    std::filebuf implDumpBuffer;
    std::string fileName = "reference_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Scanning language: osl. Target: reference" << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    const std::string language("osl");
    const std::string target("");

    std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
    implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    for (auto impl : impls)
    {
        if (language == impl->getLanguage() && impl->getTarget().empty())
        {
            std::string msg("Impl: ");
            msg += impl->getName();

            mx::NodeDefPtr nodedef = impl->getNodeDef();
            if (!nodedef)
            {
                std::string nodedefName = impl->getNodeDefString();
                msg += ". Does NOT have a nodedef with name: " + nodedefName;
            }
            implDumpStream << msg << std::endl;
        }
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
        if (!mx::requiresImplementation(nodeDef))
        {
            found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
            continue;
        }

        mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
        if (!inter)
        {
            missing++;
            missing_str += "Missing nodeDef implemenation: " + nodeDefName + ", Node: " + nodeName + ".\n";
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
                        + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                }
                else
                {
                    found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + impl->getName() + ".\n";
                }
            }
            else
            {
                mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Impl: " + graph->getName() + ".\n";
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

TEST_CASE("ShaderX Implementation Validity", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    std::vector<mx::ShaderGeneratorPtr> shaderGenerators =
    {
#ifdef MATERIALX_BUILD_GEN_OSL
        mx::ArnoldShaderGenerator::create(),
#endif
#ifdef MATERIALX_BUILD_GEN_GLSL
        mx::GlslShaderGenerator::create(),
#endif
#ifdef MATERIALX_BUILD_GEN_OGSFX
        mx::OgsFxShaderGenerator::create()
#endif
    };

    std::filebuf implDumpBuffer;
    std::string fileName = "shadgen_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    for (auto generator : shaderGenerators)
    {
        generator->registerSourceCodeSearchPath(searchPath);

        const std::string& language = generator->getLanguage();
        const std::string& target = generator->getTarget();

        // Node types to explicitly skip temporarily.
        std::set<std::string> skipNodeTypes =
        {
            "ambientocclusion",
            "arrayappend",
            "curveadjust",
        };

#ifdef MATERIALX_BUILD_GEN_OSL
        // Skip light types in OSL for now
        if (language == mx::OslShaderGenerator::LANGUAGE)
        {
            skipNodeTypes.insert("light");
            skipNodeTypes.insert("pointlight");
            skipNodeTypes.insert("directionallight");
            skipNodeTypes.insert("spotlight");
        }
#endif // MATERIALX_BUILD_GEN_OSL

        // Explicit set of node defs to skip temporarily
        std::set<std::string> skipNodeDefs =
        {
            "ND_add_displacementshader",
            "ND_add_volumeshader",
            "ND_multiply_displacementshaderF",
            "ND_multiply_displacementshaderV",
            "ND_multiply_volumeshaderF",
            "ND_multiply_volumeshaderC",
            "ND_mix_displacementshader",
            "ND_mix_volumeshader"
        };

#ifdef MATERIALX_BUILD_GEN_GLSL
        // Skip some shader math in GLSL for now
        if (language == mx::GlslShaderGenerator::LANGUAGE)
        {
            skipNodeDefs.insert("ND_add_surfaceshader");
            skipNodeDefs.insert("ND_multiply_surfaceshaderF");
            skipNodeDefs.insert("ND_multiply_surfaceshaderC");
            skipNodeDefs.insert("ND_mix_surfaceshader");
        }
#endif // MATERIALX_BUILD_GEN_GLSL

        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        implDumpStream << "Scanning language: " << language << ". Target: " << target << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;

        std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
        implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        for (auto impl : impls)
        {
            if (language == impl->getLanguage())
            {
                std::string msg("Impl: ");
                msg += impl->getName();
                std::string targetName = impl->getTarget();
                if (targetName.size())
                {
                    msg += ", target: " + targetName;
                }
                else
                {
                    msg += ", target: NONE ";
                }
                mx::NodeDefPtr nodedef = impl->getNodeDef();
                if (!nodedef)
                {
                    std::string nodedefName = impl->getNodeDefString();
                    msg += ". Does NOT have a nodedef with name: " + nodedefName;
                }
                implDumpStream << msg << std::endl;
            }
        }

        std::string nodeDefNode;
        std::string nodeDefType;
        unsigned int count = 0;
        unsigned int missing = 0;
        unsigned int skipped = 0;
        std::string missing_str;
        std::string found_str;

        // Scan through every nodedef defined
        for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
        {
            count++;

            const std::string& nodeDefName = nodeDef->getName();
            const std::string& nodeName = nodeDef->getNodeString();

            if (skipNodeTypes.count(nodeName))
            {
                found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
                skipped++;
                continue;
            }
            if (skipNodeDefs.count(nodeDefName))
            {
                found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
                skipped++;
                continue;
            }

            if (!requiresImplementation(nodeDef))
            {
                found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
                continue;
            }

            mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
            if (!inter)
            {
                missing++;
                missing_str += "Missing nodeDef implementation: " + nodeDefName + ", Node: " + nodeName + ".\n";

                std::vector<mx::InterfaceElementPtr> inters = doc->getMatchingImplementations(nodeDefName);
                for (auto inter2 : inters)
                {
                    mx::ImplementationPtr impl = inter2->asA<mx::Implementation>();
                    if (impl)
                    {
                        std::string msg("\t Cached Impl: ");
                        msg += impl->getName();
                        msg += ", nodedef: " + impl->getNodeDefString();
                        msg += ", target: " + impl->getTarget();
                        msg += ", language: " + impl->getLanguage();
                        missing_str += msg + ".\n";
                    }
                }

                for (auto childImpl : impls)
                {
                    if (childImpl->getNodeDefString() == nodeDefName)
                    {
                        std::string msg("\t Doc Impl: ");
                        msg += childImpl->getName();
                        msg += ", nodedef: " + childImpl->getNodeDefString();
                        msg += ", target: " + childImpl->getTarget();
                        msg += ", language: " + childImpl->getLanguage();
                        missing_str += msg + ".\n";
                    }
                }

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
                            + nodeDefName + ". Impl: " + impl->getName() + ".\n";
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
                                + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                        }
                        else
                        {
                            found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                                + nodeName + +". Impl: " + impl->getName() + "Path: " + resolvedPath.asString() + ".\n";
                        }
                    }
                }
                else
                {
                    mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                    found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + graph->getName() + ".\n";
                }
            }
        }

        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs. Skipped: " << skipped << std::endl;
        implDumpStream << missing_str << std::endl;
        implDumpStream << found_str << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;

        // Should have 0 missing including skipped
        REQUIRE(missing == 0);
        REQUIRE(skipped == 38);
    }

    implDumpBuffer.close();
}

TEST_CASE("Swizzling", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({"stdlib"}, searchPath, doc);

    mx::GenOptions options;
    mx::GenContext context(mx::ShaderGenerator::CONTEXT_DEFAULT);

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ArnoldShaderGenerator sg;
        sg.registerSourceCodeSearchPath(searchPath);
        const mx::Syntax* syntax = sg.getSyntax();

        // Test swizzle syntax
        std::string var1 = syntax->getSwizzledVariable("foo", mx::Type::COLOR3, "bgr", mx::Type::COLOR3);
        REQUIRE(var1 == "color(foo[2], foo[1], foo[0])");
        std::string var2 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR2, "xy", mx::Type::COLOR2);
        REQUIRE(var2 == "color2(foo.x, foo.y)");
        std::string var3 = syntax->getSwizzledVariable("foo", mx::Type::FLOAT, "rr01", mx::Type::COLOR4);
        REQUIRE(var3 == "color4(color(foo, foo, 0), 1)");
        std::string var4 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR3, "zyx", mx::Type::VECTOR3);
        REQUIRE(var4 == "vector(foo[2], foo[1], foo[0])");
        std::string var5 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR4, "yy", mx::Type::VECTOR2);
        REQUIRE(var5 == "vector2(foo.y, foo.y)");
        std::string var6 = syntax->getSwizzledVariable("foo", mx::Type::COLOR2, "rraa", mx::Type::VECTOR4);
        REQUIRE(var6 == "vector4(foo.r, foo.r, foo.a, foo.a)");

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
        test1.initialize(output1, sg, options);
        mx::ShaderNode* sgNode = test1.getGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test1.addFunctionCall(sgNode, context, sg);
        const std::string test1Result = "color swizzle1_out = color(swizzle1_in[0], swizzle1_in[0], swizzle1_in[0]);\n";
        REQUIRE(test1.getSourceCode() == test1Result);

        // Change swizzle pattern and test again
        swizzle1->setParameterValue("channels", std::string("b0b"));
        mx::Shader test2("test2");
        test2.initialize(output1, sg, options);
        sgNode = test2.getGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test2.addFunctionCall(sgNode, context, sg);
        const std::string test2Result = "color swizzle1_out = color(swizzle1_in[2], 0, swizzle1_in[2]);\n";
        REQUIRE(test2.getSourceCode() == test2Result);
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GlslShaderGenerator sg;
        sg.registerSourceCodeSearchPath(searchPath);
        const mx::Syntax* syntax = sg.getSyntax();

        // Test swizzle syntax
        std::string var1 = syntax->getSwizzledVariable("foo", mx::Type::COLOR3, "bgr", mx::Type::COLOR3);
        REQUIRE(var1 == "vec3(foo.z, foo.y, foo.x)");
        std::string var2 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR2, "xy", mx::Type::COLOR2);
        REQUIRE(var2 == "vec2(foo.x, foo.y)");
        std::string var3 = syntax->getSwizzledVariable("foo", mx::Type::FLOAT, "rr01", mx::Type::COLOR4);
        REQUIRE(var3 == "vec4(foo, foo, 0, 1)");
        std::string var4 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR3, "zyx", mx::Type::VECTOR3);
        REQUIRE(var4 == "vec3(foo.z, foo.y, foo.x)");
        std::string var5 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR4, "yy", mx::Type::VECTOR2);
        REQUIRE(var5 == "vec2(foo.y, foo.y)");
        std::string var6 = syntax->getSwizzledVariable("foo", mx::Type::COLOR2, "rraa", mx::Type::VECTOR4);
        REQUIRE(var6 == "vec4(foo.x, foo.x, foo.y, foo.y)");

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
        test1.initialize(output1, sg, options);
        mx::ShaderNode* sgNode = test1.getGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test1.addFunctionCall(sgNode, context, sg);
        const std::string test1Result = "vec3 swizzle1_out = vec3(swizzle1_in.x, swizzle1_in.x, swizzle1_in.x);\n";
        REQUIRE(test1.getSourceCode() == test1Result);

        // Change swizzle pattern and test again
        swizzle1->setParameterValue("channels", std::string("b0b"));
        mx::Shader test2("test2");
        test2.initialize(output1, sg, options);
        sgNode = test2.getGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test2.addFunctionCall(sgNode, context, sg);
        const std::string test2Result = "vec3 swizzle1_out = vec3(swizzle1_in.z, 0, swizzle1_in.z);\n";
        REQUIRE(test2.getSourceCode() == test2Result);
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

#ifdef MATERIALX_BUILD_GEN_OSL
//
// Utility to call validate OSL.
// For now only call into oslc to compile an OSL file and get the results.
//
static void validateOSL(const std::string oslFileName, std::string& errorResult)
{
    errorResult.clear();

    // Use the user specified build options for oslc exe, and include path
    const std::string oslcCommand(MATERIALX_OSLC_EXECUTABLE);
    const std::string oslIncludePath(MATERIALX_OSL_INCLUDE_PATH);
    if (oslcCommand.empty() || oslIncludePath.empty())
    {
        return;
    }

    // Set oso output name
    std::string osoFileName = mx::removeExtension(oslFileName);
    osoFileName += ".oso";

    // Use a known error file name to check
    std::string errorFile(oslFileName + "_errors.txt");
    const std::string redirectString(" 2>&1");

    // Run the command and get back the result. If non-empty string throw exception with error
    std::string command = oslcCommand + " -o " + osoFileName + " -q -I\"" + oslIncludePath + "\" " + oslFileName + " > " +
        errorFile + redirectString;

    int returnValue = std::system(command.c_str());

    std::ifstream errorStream(errorFile);
    errorResult.assign(std::istreambuf_iterator<char>(errorStream),
        std::istreambuf_iterator<char>());

    if (errorResult.length())
    {
        errorResult = "Command return code: " + std::to_string(returnValue) + "\n" +
            errorResult;
        std::cout << "OSLC failed to compile: " << oslFileName << ":\n"
            << errorResult << std::endl;
    }
}
#endif // MATERIALX_BUILD_GEN_OSL

TEST_CASE("Hello World", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "hello_world";

    // Create a nodedef taking two color3 and producing another color3
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "color3", exampleName);
    mx::InputPtr inputA = nodeDef->addInput("a", "color3");
    mx::InputPtr inputB = nodeDef->addInput("b", "color3");
    inputA->setValue(mx::Color3(1.0f, 1.0f, 0.0f));
    inputB->setValue(mx::Color3(0.8f, 0.1f, 0.1f));

    // Create an implementation graph for the nodedef performing
    // a multiplication of the two colors.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");
    mx::NodePtr mult1 = nodeGraph->addNode("multiply", "mult1", "color3");
    mx::InputPtr in1 = mult1->addInput("in1", "color3");
    in1->setInterfaceName(inputA->getName());
    mx::InputPtr in2 = mult1->addInput("in2", "color3");
    in2->setInterfaceName(inputB->getName());
    output1->setConnectedNode(mult1);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shadergen->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Test shader generation from nodegraph
        mx::ShaderPtr shader = shadergen->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        std::ofstream file;
        std::string fileName(RESULT_DIRECTORY + shader->getName() + "_graph.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        // For now only use externally specified oslc to check code.
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);

        // Test shader generation from shaderref
        shader = shadergen->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        fileName.assign(RESULT_DIRECTORY + shader->getName() + "_shaderref.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        // For now only use externally specified oslc to check code.
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
#endif // MATERIALX_BUILD_GEN_GLSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shadergen = mx::OgsFxShaderGenerator::create();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph
        mx::ShaderPtr shader = shadergen->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + "_graph.ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        file.close();

        // Test shader generation from shaderref
        shader = shadergen->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        file.open(RESULT_DIRECTORY + shader->getName() + "_shaderref.ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        file.close();
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shadergen = mx::GlslShaderGenerator::create();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph
        mx::ShaderPtr shader = shadergen->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + "_graph_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_graph_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // Test shader generation from shaderref
        shader = shadergen->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        file.open(RESULT_DIRECTORY + shader->getName() + "_shaderref_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_shaderref_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Conditionals", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "conditionals";

    // Create a simple node graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph(exampleName + "_graph");

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
    switch1->setParameterValue<int>("which", 3);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(switch1);

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = nodeGraph->asStringDot();
    file.open(RESULT_DIRECTORY + nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getGraph()->getNodes().empty());
        REQUIRE(shader->getGraph()->getOutputSocket()->value != nullptr);
        REQUIRE(shader->getGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());

        // Write out to file for inspection
        const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
        file << shader->getSourceCode();
        file.close();

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getGraph()->getNodes().empty());
        REQUIRE(shader->getGraph()->getOutputSocket()->value != nullptr);
        REQUIRE(shader->getGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getGraph()->getNodes().empty());
        REQUIRE(shader->getGraph()->getOutputSocket()->value != nullptr);
        REQUIRE(shader->getGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Geometric Nodes", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "geometric_nodes";

    // Create a nonsensical graph testing some geometric nodes
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);

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

    // Create a nodedef and make its implementation be the graph above
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "color3", exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        std::ofstream file;
        const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Noise", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "noise_test";

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");

    std::vector<mx::NodePtr> noiseNodes;
    mx::NodePtr noise2d = nodeGraph->addNode("noise2d", "noise2d", "float");
    noiseNodes.push_back(noise2d);
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d", "noise3d", "float");
    noiseNodes.push_back(noise3d);
    mx::NodePtr cellnoise2d = nodeGraph->addNode("cellnoise2d", "cellnoise2d", "float");
    noiseNodes.push_back(cellnoise2d);
    mx::NodePtr cellnoise3d = nodeGraph->addNode("cellnoise3d", "cellnoise3d", "float");
    noiseNodes.push_back(cellnoise3d);
    mx::NodePtr fractal3d = nodeGraph->addNode("fractal3d", "fractal3d", "float");
    noiseNodes.push_back(fractal3d);

    noise2d->setParameterValue("amplitude", 1.0);
    noise2d->setParameterValue("pivot", 0.0f);
    noise3d->setParameterValue("amplitude", 1.0);
    noise3d->setParameterValue("pivot", 0.0f);
    fractal3d->setParameterValue("amplitude", 1.0f);

    // Multiplier to scale noise input uv's
    mx::NodePtr uv1 = nodeGraph->addNode("texcoord", "uv1", "vector2");
    mx::NodePtr uvmult1 = nodeGraph->addNode("multiply", "uvmult1", "vector2");
    uvmult1->setConnectedNode("in1", uv1);
    uvmult1->setInputValue("in2", mx::Vector2(16, 16));

    // Multiplier to scale noise input position
    mx::NodePtr pos1 = nodeGraph->addNode("position", "pos1", "vector3");
    noise3d->setConnectedNode("position", pos1);
    mx::NodePtr posmult1 = nodeGraph->addNode("multiply", "posmult1", "vector3");
    posmult1->setConnectedNode("in1", pos1);
    posmult1->setInputValue("in2", mx::Vector3(16, 16, 16));

    noise2d->setConnectedNode("texcoord", uvmult1);
    noise3d->setConnectedNode("position", posmult1);
    cellnoise2d->setConnectedNode("texcoord", uvmult1);
    cellnoise3d->setConnectedNode("position", posmult1);
    fractal3d->setConnectedNode("position", posmult1);

    // Create a noise selector switch
    mx::NodePtr switch1 = nodeGraph->addNode("switch", "switch1", "float");
    switch1->setConnectedNode("in1", noise2d);
    switch1->setConnectedNode("in2", noise3d);
    switch1->setConnectedNode("in3", cellnoise2d);
    switch1->setConnectedNode("in4", cellnoise3d);
    switch1->setConnectedNode("in5", fractal3d);

    // Remap the noise to [0,1]
    mx::NodePtr add1 = nodeGraph->addNode("add", "add1", "float");
    mx::NodePtr multiply1 = nodeGraph->addNode("multiply", "multiply1", "float");
    add1->setConnectedNode("in1", switch1);
    add1->setInputValue("in2", 1.0f);
    multiply1->setConnectedNode("in1", add1);
    multiply1->setInputValue("in2", 0.5f);

    // Blend some colors using the noise
    mx::NodePtr mixer = nodeGraph->addNode("mix", "mixer", "color3");
    mixer->setInputValue("fg", mx::Color3(1, 0, 0));
    mixer->setInputValue("bg", mx::Color3(1, 1, 0));
    mixer->setConnectedNode("mix", multiply1);

    output1->setConnectedNode(mixer);

    mx::GenOptions options;

    const size_t numNoiseType = noiseNodes.size();
    for (size_t noiseType = 0; noiseType < numNoiseType; ++noiseType)
    {
        const std::string shaderName = "test_" + noiseNodes[noiseType]->getName();

        // Select the noise type
        switch1->setParameterValue("which", float(noiseType));

#ifdef MATERIALX_BUILD_GEN_OSL
        {
            mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
            // Add path to find all source code snippets
            shadergen->registerSourceCodeSearchPath(searchPath);
            // Add path to find OSL include files
            shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

            // Test shader generation from nodegraph
            mx::ShaderPtr shader = shadergen->generate(shaderName, output1, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            std::ofstream file;
            const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
            file.open(fileName);
            file << shader->getSourceCode();
            file.close();

            // TODO: Use validation in MaterialXRender library
            std::string errorResult;
            validateOSL(fileName, errorResult);
            REQUIRE(errorResult.size() == 0);
        }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
        {
            mx::ShaderGeneratorPtr shadergen = mx::OgsFxShaderGenerator::create();
            shadergen->registerSourceCodeSearchPath(searchPath);

            // Test shader generation from nodegraph
            mx::ShaderPtr shader = shadergen->generate(shaderName, output1, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
            file.close();
        }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
        {
            mx::ShaderGeneratorPtr shadergen = mx::GlslShaderGenerator::create();
            shadergen->registerSourceCodeSearchPath(searchPath);

            // Test shader generation from nodegraph
            mx::ShaderPtr shader = shadergen->generate(shaderName, output1, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);
            REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
            file.close();
            file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();
        }
#endif // MATERIALX_BUILD_GEN_GLSL
    }
}


TEST_CASE("Unique Names", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "unique_names";

    // Generate a shade with an internal node having the same name as the shader,
    // which will result in a name conflict between the shader output and the
    // internal node output
    const std::string shaderName = "unique_names";
    const std::string nodeName = shaderName;

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");
    mx::NodePtr node1 = nodeGraph->addNode("noise2d", nodeName, "color3");

    output1->setConnectedNode(node1);

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Set the output to a restricted name for OSL
        output1->setName("output");

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Make sure the output and internal node output has been renamed
        mx::ShaderGraphOutputSocket* sgOutputSocket = shader->getGraph()->getOutputSocket();
        REQUIRE(sgOutputSocket->name != "output");
        mx::ShaderNode* sgNode1 = shader->getGraph()->getNode(node1->getName());
        REQUIRE(sgNode1->getOutput()->name == "unique_names_out");

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        const std::string fileName(RESULT_DIRECTORY + exampleName + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Set the output to a restricted name for OgsFx
        output1->setName("out");

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Make sure the output and internal node output has been renamed
        mx::ShaderGraphOutputSocket* sgOutputSocket = shader->getGraph()->getOutputSocket();
        REQUIRE(sgOutputSocket->name != "out");
        mx::ShaderNode* sgNode1 = shader->getGraph()->getNode(node1->getName());
        REQUIRE(sgNode1->getOutput()->name == "unique_names_out");

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + exampleName + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Set the output to a restricted name for GLSL
        output1->setName("vec3");

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Make sure the output and internal node output has been renamed
        mx::ShaderGraphOutputSocket* sgOutputSocket = shader->getGraph()->getOutputSocket();
        REQUIRE(sgOutputSocket->name != "vec3");
        mx::ShaderNode* sgNode1 = shader->getGraph()->getNode(node1->getName());
        REQUIRE(sgNode1->getOutput()->name == "unique_names_out");

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + exampleName + "_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + exampleName + "_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Subgraphs", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::FilePath examplesSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples");
    loadExamples({ "SubGraphs.mtlx"}, examplesSearchPath, searchPath,  doc);

    std::vector<std::string> exampleGraphNames = { "subgraph_ex1" , "subgraph_ex2" };

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(graphName, output, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
            file.open(fileName);
            file << shader->getSourceCode();
            file.close();

            // TODO: Use validation in MaterialXRender library
            std::string errorResult;
            validateOSL(fileName, errorResult);
            REQUIRE(errorResult.size() == 0);
        }
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(graphName, output, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        }
    }
#endif // MATERIALX_BUILD_OGSX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(graphName, output, options);
            REQUIRE(shader != nullptr);

            REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
            REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();
            file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        }
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Materials", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::FilePath materialsFile = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/sxpbrlib/materials/surfaceshader.mtlx");
    mx::readFromXmlFile(doc, materialsFile.asString());

    // Get all materials
    std::vector<mx::MaterialPtr> materials = doc->getMaterials();

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        for (const mx::MaterialPtr& material : doc->getMaterials())
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode().length() > 0);

                // Write out to file for inspection
                std::ofstream file;
                const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
                file.open(fileName);
                file << shader->getSourceCode();
                file.close();

                // TODO: Use validation in MaterialXRender library
                std::string errorResult;
                validateOSL(fileName, errorResult);
                REQUIRE(errorResult.size() == 0);
            }
        }
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode().length() > 0);

                // Write out to file for inspection
                // TODO: Use validation in MaterialXRender library
                std::ofstream file;
                file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
                file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
            }
        }
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
                REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

                // Write out to file for inspection
                // TODO: Use validation in MaterialXRender library
                std::ofstream file;
                file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
                file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
                file.close();
                file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
                file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
            }
        }
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Color Spaces", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::StringVec libraryNames;
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::MaterialPtr material = doc->addMaterial("color_spaces");
    mx::ShaderRefPtr shaderRef = material->addShaderRef("color_spaces_surface", "standard_surface");

    // Bind an image texture to the base_color input, with sRGB color space
    mx::NodePtr baseColorTex = doc->addNode("image", "base_color_tex", "color3");
    mx::ParameterPtr baseColorTexFileParam = baseColorTex->setParameterValue("file", std::string("image1.png"), "filename");
    baseColorTexFileParam->setAttribute("colorspace", "sRGB");
    mx::OutputPtr baseColorOutput = doc->addOutput("baseColorOutput", "color3");
    baseColorOutput->setConnectedNode(baseColorTex);
    mx::BindInputPtr baseColorBind = shaderRef->addBindInput("base_color", "color3");
    baseColorBind->setConnectedOutput(baseColorOutput);

    // Bind an image texture to the specular_roughness input, with sRGB color space
    // This color spaces transform should be ignored since it's a float data type
    mx::NodePtr rougnessTex = doc->addNode("image", "specular_roughness_tex", "float");
    mx::ParameterPtr rougnessTexFileParam = rougnessTex->setParameterValue("file", std::string("image2.png"), "filename");
    rougnessTexFileParam->setAttribute("colorspace", "sRGB");
    mx::OutputPtr roughnessOutput = doc->addOutput("roughnessOutput", "float");
    roughnessOutput->setConnectedNode(rougnessTex);
    mx::BindInputPtr rougnessBind = shaderRef->addBindInput("specular_roughness", "float");
    rougnessBind->setConnectedOutput(roughnessOutput);

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(material->getName(), shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        std::ofstream file;
        const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(material->getName(), shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(material->getName(), shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("BSDF Layering", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    const std::string exampleName = "layered_bsdf";

    // Create a nodedef interface for the surface shader
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "surfaceshader", exampleName);
    nodeDef->addInput("diffuse_color", "color3");
    nodeDef->addInput("sss_color", "color3");
    nodeDef->addInput("sss_weight", "float");
    nodeDef->addInput("coating_tint", "color3");
    nodeDef->addInput("coating_roughness", "float");
    nodeDef->addInput("coating_ior", "float");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    // Diffuse component
    mx::NodePtr diffuse = nodeGraph->addNode("diffusebrdf", "diffuse", "BSDF");
    mx::InputPtr diffuse_color = diffuse->addInput("color", "color3");
    diffuse_color->setInterfaceName("diffuse_color");

    // Translucent (thin walled SSS) component
    mx::NodePtr sss = nodeGraph->addNode("diffusebtdf", "sss", "BSDF");
    mx::InputPtr sss_color = sss->addInput("color", "color3");
    sss_color->setInterfaceName("sss_color");

    // Mix diffuse over sss
    mx::NodePtr substrate = nodeGraph->addNode("mixbsdf", "substrate", "BSDF");
    mx::NodePtr substrate_weight_inv = nodeGraph->addNode("invert", "substrate_weight_inv", "float");
    substrate->setConnectedNode("in1", diffuse);
    substrate->setConnectedNode("in2", sss);
    substrate->setConnectedNode("weight", substrate_weight_inv);
    mx::InputPtr sss_weight = substrate_weight_inv->addInput("in", "float");
    sss_weight->setInterfaceName("sss_weight");

    // Add a coating specular component on top
    mx::NodePtr coating = nodeGraph->addNode("dielectricbrdf", "coating", "BSDF");
    coating->setConnectedNode("base", substrate);
    mx::InputPtr coating_tint = coating->addInput("tint", "color3");
    coating_tint->setInterfaceName("coating_tint");
    mx::NodePtr coating_roughness = nodeGraph->addNode("roughness", "coating_roughness", "roughnessinfo");
    mx::InputPtr roughness = coating_roughness->addInput("roughness", "float");
    roughness->setInterfaceName("coating_roughness");
    coating->setConnectedNode("roughness", coating_roughness);
    mx::InputPtr coating_ior = coating->addInput("ior", "float");
    coating_ior->setInterfaceName("coating_ior");

    // Create a surface shader
    mx::NodePtr surface = nodeGraph->addNode("surface", "surface1", "surfaceshader");
    surface->setConnectedNode("bsdf", coating);

    // Connect to graph output
    mx::OutputPtr output = nodeGraph->addOutput("output", "surfaceshader");
    output->setConnectedNode(surface);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef("shaderref", exampleName);

    // Bind shader parameter values
    mx::BindInputPtr diffuse_color_input = shaderRef->addBindInput("diffuse_color", "color3");
    diffuse_color_input->setValue(mx::Color3(0.8f, 0.2f, 0.8f));
    mx::BindInputPtr sss_color_input = shaderRef->addBindInput("sss_color", "color3");
    sss_color_input->setValue(mx::Color3(1.0f, 0.0f, 0.0f));
    mx::BindInputPtr sss_weight_input = shaderRef->addBindInput("sss_weight", "float");
    sss_weight_input->setValue(0.45f);
    mx::BindInputPtr coating_color_input = shaderRef->addBindInput("coating_color", "color3");
    coating_color_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    mx::BindInputPtr coating_roughness_input = shaderRef->addBindInput("coating_roughness", "float");
    coating_roughness_input->setValue(0.2f);
    mx::BindInputPtr coating_ior_input = shaderRef->addBindInput("coating_ior", "float");
    coating_ior_input->setValue(1.52f);

    mx::GenOptions options;

    // Test generation from both graph ouput and shaderref
    std::vector<mx::ElementPtr> elements = { output, shaderRef };
    for (mx::ElementPtr elem : elements)
    {
        const std::string shaderName = exampleName + "_" + elem->getName();

#ifdef MATERIALX_BUILD_GEN_OSL
        {
            mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
            // Add path to find all source code snippets
            shaderGenerator->registerSourceCodeSearchPath(searchPath);
            // Add path to find OSL include files
            shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

            mx::ShaderPtr shader = shaderGenerator->generate(shaderName, elem, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
            file.open(fileName);
            file << shader->getSourceCode();
            file.close();

            // TODO: Use validation in MaterialXRender library
            std::string errorResult;
            validateOSL(fileName, errorResult);
            REQUIRE(errorResult.size() == 0);
        }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
        {
            mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
            shaderGenerator->registerSourceCodeSearchPath(searchPath);

            // Setup lighting
            mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
            createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

            mx::ShaderPtr shader = shaderGenerator->generate(shaderName, elem, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
        {
            mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
            shaderGenerator->registerSourceCodeSearchPath(searchPath);

            // Setup lighting
            mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
            createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

            mx::ShaderPtr shader = shaderGenerator->generate(shaderName, elem, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
            REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXRender library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();
            file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        }
#endif // MATERIALX_BUILD_GEN_GLSL
    }
}

TEST_CASE("Transparency", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    const std::string exampleName = "transparent_surface";

    // Create a nodedef interface for the surface shader
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "surfaceshader", exampleName);
    mx::InputPtr in = nodeDef->addInput("reflection", "float");
    in->setValue(1.0f);
    in = nodeDef->addInput("reflection_color", "color3");
    in->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    in = nodeDef->addInput("transmission", "float");
    in->setValue(1.0f);
    in = nodeDef->addInput("transmission_tint", "color3");
    in->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    in = nodeDef->addInput("roughness", "vector2");
    in->setValue(mx::Vector2(0.0f, 0.0f));
    in = nodeDef->addInput("ior", "float");
    in->setValue(1.5f);
    in = nodeDef->addInput("opacity", "float");
    in->setValue(1.0f);

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    mx::NodePtr worldNormal = nodeGraph->addNode("normal", "worldNormal", "vector3");
    worldNormal->setParameterValue<std::string>("space", "world");
    mx::NodePtr worldTangent = nodeGraph->addNode("tangent", "worldTangent", "vector3");
    worldTangent->setParameterValue<std::string>("space", "world");

    mx::NodePtr transmission = nodeGraph->addNode("dielectricbtdf", "transmission", "BSDF");
    transmission->setConnectedNode("normal", worldNormal);
    transmission->setConnectedNode("tangent", worldTangent);
    mx::InputPtr transmission_tint = transmission->addInput("tint", "color3");
    transmission_tint->setInterfaceName("transmission_tint");
    mx::InputPtr transmission_weight = transmission->addInput("weight", "float");
    transmission_weight->setInterfaceName("transmission");
    mx::InputPtr transmission_roughness = transmission->addInput("roughness", "vector2");
    transmission_roughness->setInterfaceName("roughness");
    mx::InputPtr transmission_ior = transmission->addInput("ior", "float");
    transmission_ior->setInterfaceName("ior");

    mx::NodePtr reflection = nodeGraph->addNode("dielectricbrdf", "reflection", "BSDF");
    reflection->setConnectedNode("normal", worldNormal);
    reflection->setConnectedNode("tangent", worldTangent);
    reflection->setConnectedNode("base", transmission);
    mx::InputPtr reflection_color = reflection->addInput("tint", "color3");
    reflection_color->setInterfaceName("reflection_color");
    mx::InputPtr reflection_weight = reflection->addInput("weight", "float");
    reflection_weight->setInterfaceName("reflection");
    mx::InputPtr reflection_roughness = reflection->addInput("roughness", "vector2");
    reflection_roughness->setInterfaceName("roughness");
    mx::InputPtr reflection_ior = reflection->addInput("ior", "float");
    reflection_ior->setInterfaceName("ior");

    mx::NodePtr surface = nodeGraph->addNode("surface", "surface1", "surfaceshader");
    surface->setConnectedNode("bsdf", reflection);

    mx::InputPtr opacity = surface->addInput("opacity", "float");
    opacity->setInterfaceName("opacity");

    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(surface);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", "standard_surface");

    // Bind shader parameter values
    mx::BindInputPtr reflection_input = shaderRef->addBindInput("reflection", "float");
    reflection_input->setValue(1.0f);
    mx::BindInputPtr transmission_input = shaderRef->addBindInput("transmission", "float");
    transmission_input->setValue(1.0f);
    mx::BindInputPtr ior_input = shaderRef->addBindInput("ior", "float");
    ior_input->setValue(1.50f);
    mx::BindInputPtr opacity_input = shaderRef->addBindInput("opacity", "color3");
    opacity_input->setValue(mx::Color3(1.0f, 1, 1));

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        const std::string fileName(RESULT_DIRECTORY + shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
#endif // MATERIALX_BUILD_GEN_OSL

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        std::unordered_map<std::string, mx::ShaderGeneratorPtr> shaderGenerators =
        {
            {".ogsfx", mx::OgsFxShaderGenerator::create()},
            {".glslplugin.ogsfx", mx::MayaGlslPluginShaderGenerator::create()}
        };

        for (auto it : shaderGenerators)
        {
            mx::ShaderGeneratorPtr shaderGenerator = it.second;
            shaderGenerator->registerSourceCodeSearchPath(searchPath);

            // Setup lighting
            mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
            createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

            // Test the transparency tracking
            transmission_input->setValue(0.0f);
            options.hwTransparency = isTransparentSurface(shaderRef, *shaderGenerator);
            REQUIRE(!options.hwTransparency);
            transmission_input->setValue(1.0f);
            options.hwTransparency = isTransparentSurface(shaderRef, *shaderGenerator);
            REQUIRE(options.hwTransparency);

            mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXView library
            std::ofstream file;
            file.open(RESULT_DIRECTORY + shader->getName() + it.first);
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        }
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        // Specify if this shader needs to handle transparency
        options.hwTransparency = isTransparentSurface(shaderRef, *shaderGenerator);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

TEST_CASE("Surface Layering", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    const std::string exampleName = "layered_surface";

    // Create a nodedef interface for the surface shader
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "surfaceshader", exampleName);
    nodeDef->addInput("layer1_diffuse", "color3");
    nodeDef->addInput("layer1_specular", "color3");
    nodeDef->addInput("layer2_diffuse", "color3");
    nodeDef->addInput("layer2_specular", "color3");
    nodeDef->addInput("mix_weight", "float");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    // Create first surface layer from a surface with two BSDF's
    mx::NodePtr layer1_diffuse = nodeGraph->addNode("diffusebrdf", "layer1_diffuse", "BSDF");
    mx::InputPtr layer1_diffuse_color = layer1_diffuse->addInput("color", "color3");
    layer1_diffuse_color->setInterfaceName("layer1_diffuse");
    mx::NodePtr layer1_specular = nodeGraph->addNode("dielectricbrdf", "layer1_specular", "BSDF");
    layer1_specular->setConnectedNode("base", layer1_diffuse);
    mx::InputPtr layer1_specular_color = layer1_specular->addInput("tint", "color3");
    layer1_specular_color->setInterfaceName("layer1_specular");
    mx::NodePtr layer1 = nodeGraph->addNode("surface", "layer1", "surfaceshader");
    layer1->setConnectedNode("bsdf", layer1_specular);

    // Create second surface layer from a standard uber shader
    mx::NodePtr layer2 = nodeGraph->addNode("standard_surface", "layer2", "surfaceshader");
    mx::InputPtr layer2_diffuse_color = layer2->addInput("base_color", "color3");
    layer2_diffuse_color->setInterfaceName("layer2_diffuse");
    mx::InputPtr layer2_specular_color = layer2->addInput("specular_color", "color3");
    layer2_specular_color->setInterfaceName("layer2_specular");

    // Create layer mixer
    mx::NodePtr mixer = nodeGraph->addNode("mixsurface", "mixer", "surfaceshader");
    mixer->setConnectedNode("in1", layer1);
    mixer->setConnectedNode("in2", layer2);
    mx::InputPtr mix_weight = mixer->addInput("weight", "float");
    mix_weight->setInterfaceName("mix_weight");

    // Connect to graph output
    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(mixer);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    // Bind shader parameter values
    mx::BindInputPtr layer1_diffuse_input = shaderRef->addBindInput("layer1_diffuse", "color3");
    layer1_diffuse_input->setValue(mx::Color3(0.2f, 0.8f, 0.2f));
    mx::BindInputPtr layer1_specular_input = shaderRef->addBindInput("layer1_specular", "color3");
    layer1_specular_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    mx::BindInputPtr layer2_diffuse_input = shaderRef->addBindInput("layer2_diffuse", "color3");
    layer2_diffuse_input->setValue(mx::Color3(0.8f, 0.2f, 0.8f));
    mx::BindInputPtr layer2_specular_input = shaderRef->addBindInput("layer2_specular", "color3");
    layer2_specular_input->setValue(mx::Color3(1.0f, 0.0f, 0.0f));
    mx::BindInputPtr mix_weight_input = shaderRef->addBindInput("mix_weight", "float");
    mix_weight_input->setValue(0.5f);

    mx::GenOptions options;

#ifdef MATERIALX_BUILD_GEN_OGSFX
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        // Specify if this shader needs to handle transparency
        options.hwTransparency = isTransparentSurface(shaderRef, *shaderGenerator);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_OGSFX

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        // Specify if this shader needs to handle transparency
        options.hwTransparency = isTransparentSurface(shaderRef, *shaderGenerator);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXRender library
        std::ofstream file;
        file.open(RESULT_DIRECTORY + shader->getName() + "_ps.glsl");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(RESULT_DIRECTORY + shader->getName() + "_vs.glsl");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
#endif // MATERIALX_BUILD_GEN_GLSL
}

#ifdef MATERIALX_BUILD_GEN_OSL
TEST_CASE("Osl Output Types", "[shadergen]")
{
    // OSL doesn't support having color2/color4 as shader output types.
    // The color2/color4 types are custom struct types added by MaterialX.
    // It's actually crashing the OSL compiler right now.
    // TODO: Report this problem to the OSL team.
    //
    // This test makes sure that color2/color4/vector2/vector4 gets converted
    // to color/vector when used as shader outputs.

    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "osl_output";

    mx::NodeGraphPtr nodeGraph1 = doc->addNodeGraph();
    mx::OutputPtr output1 = nodeGraph1->addOutput(mx::EMPTY_STRING, "color2");
    mx::NodePtr node1 = nodeGraph1->addNode("remap", mx::EMPTY_STRING, "color2");
    output1->setConnectedNode(node1);
    mx::NodeDefPtr nodeDef1 = doc->addNodeDef(mx::EMPTY_STRING, "color2", exampleName + "_color2");
    nodeGraph1->setAttribute("nodedef", nodeDef1->getName());

    mx::NodeGraphPtr nodeGraph2 = doc->addNodeGraph();
    mx::OutputPtr output2 = nodeGraph2->addOutput(mx::EMPTY_STRING, "color4");
    mx::NodePtr node2 = nodeGraph2->addNode("remap", mx::EMPTY_STRING, "color4");
    output2->setConnectedNode(node2);
    mx::NodeDefPtr nodeDef2 = doc->addNodeDef(mx::EMPTY_STRING, "color4", exampleName + "_color4");
    nodeGraph2->setAttribute("nodedef", nodeDef2->getName());

    mx::GenOptions options;

    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shadergen->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Test shader generation from color2 type graph
        mx::ShaderPtr shader = shadergen->generate(exampleName + "_color2", output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        std::ofstream file;
        std::string fileName(RESULT_DIRECTORY + exampleName + "_color2.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);

        // Test shader generation from color4 type graph
        shader = shadergen->generate(exampleName + "_color4", output2, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        fileName.assign(RESULT_DIRECTORY + exampleName + "_color4.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // Change to vector2/vector4 types
    output1->setType("vector2");
    node1->setType("vector2");
    nodeDef1->setType("vector2");
    output2->setType("vector4");
    node2->setType("vector4");
    nodeDef2->setType("vector4");

    // Add swizzling to make sure type remapping works with swizzling
    //output1->setChannels("yx");
    //output2->setChannels("wzyx");

    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shadergen->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Test shader generation from color2 type graph
        mx::ShaderPtr shader = shadergen->generate(exampleName + "_vector2", output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        std::ofstream file;
        std::string fileName(RESULT_DIRECTORY + exampleName + "_vector2.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);

        // Test shader generation from color4 type graph
        shader = shadergen->generate(exampleName + "_vector4", output2, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        fileName.assign(RESULT_DIRECTORY + exampleName + "_vector4.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXRender library
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }
}
#endif // MATERIALX_BUILD_GEN_OSL
