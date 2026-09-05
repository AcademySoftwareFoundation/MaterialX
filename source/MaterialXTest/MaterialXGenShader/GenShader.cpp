//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXGenHw/HwConstants.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/OcioColorManagementSystem.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#include <MaterialXGenShader/Util.h>

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/OslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
#include <MaterialXGenMsl/MslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
#include <MaterialXGenSlang/SlangShaderGenerator.h>
#endif

#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>

namespace mx = MaterialX;

//
// Base tests
//

TEST_CASE("GenShader: Utilities", "[genshader]")
{
    // Test simple text substitution
    std::string test1 = "Look behind you, a $threeheaded $monkey!";
    std::string result1 = "Look behind you, a mighty pirate!";
    mx::StringMap subst1 = { {"$threeheaded","mighty"}, {"$monkey","pirate"} };
    mx::tokenSubstitution(subst1, test1);
    REQUIRE(test1 == result1);

    // Test uniform name substitution
    std::string test2 = "uniform vec3 " + mx::HW::T_ENV_RADIANCE + ";";
    std::string result2 = "uniform vec3 " + mx::HW::ENV_RADIANCE + ";";
    mx::StringMap subst2 = { {mx::HW::T_ENV_RADIANCE, mx::HW::ENV_RADIANCE} };
    mx::tokenSubstitution(subst2, test2);
    REQUIRE(test2 == result2);
}

TEST_CASE("GenShader: Valid Libraries", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "libraries" }, searchPath, doc);

    std::string validationErrors;
    bool valid = doc->validate(&validationErrors);
    if (!valid)
    {
        std::cout << validationErrors << std::endl;
    }
    REQUIRE(valid);
}

TEST_CASE("GenShader: TypeDesc Check", "[genshader]")
{
    mx::TypeSystemPtr ts = mx::TypeSystem::create();
    mx::GenContext context(mx::GlslShaderGenerator::create(ts));

    // Make sure the standard types are registered
    const mx::TypeDesc floatType = ts->getType("float");
    REQUIRE(floatType != mx::Type::NONE);
    REQUIRE(floatType.getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    const mx::TypeDesc integerType = ts->getType("integer");
    REQUIRE(integerType != mx::Type::NONE);
    REQUIRE(integerType.getBaseType() == mx::TypeDesc::BASETYPE_INTEGER);
    const mx::TypeDesc booleanType = ts->getType("boolean");
    REQUIRE(booleanType != mx::Type::NONE);
    REQUIRE(booleanType.getBaseType() == mx::TypeDesc::BASETYPE_BOOLEAN);
    const mx::TypeDesc color3Type = ts->getType("color3");
    REQUIRE(color3Type != mx::Type::NONE);
    REQUIRE(color3Type.getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color3Type.getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color3Type.isFloat3());
    const mx::TypeDesc color4Type = ts->getType("color4");
    REQUIRE(color4Type != mx::Type::NONE);
    REQUIRE(color4Type.getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color4Type.getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color4Type.isFloat4());

    // Make sure we can register a new custom type
    const std::string fooTypeName = "foo";
    ts->registerType(fooTypeName, mx::TypeDesc::BASETYPE_FLOAT, mx::TypeDesc::SEMANTIC_COLOR, 5);
    mx::TypeDesc fooType = ts->getType(fooTypeName);
    REQUIRE(fooType != mx::Type::NONE);
    REQUIRE(fooType.getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);

    // Make sure we can register a new type replacing an old type
    ts->registerType(fooTypeName, mx::TypeDesc::BASETYPE_INTEGER, mx::TypeDesc::SEMANTIC_VECTOR, 3);
    fooType = ts->getType(fooTypeName);
    REQUIRE(fooType != mx::Type::NONE);
    REQUIRE(fooType.getSemantic() == mx::TypeDesc::SEMANTIC_VECTOR);

    // Make sure we can't request an unknown type
    REQUIRE(ts->getType("bar") == mx::Type::NONE);
}

TEST_CASE("GenShader: Shader Translation", "[translate]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::ShaderTranslatorPtr shaderTranslator = mx::ShaderTranslator::create();

    mx::FilePath testPath = searchPath.find("resources/Materials/Examples/StandardSurface");
    for (mx::FilePath& mtlxFile : testPath.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        mx::DocumentPtr doc = mx::createDocument();
        loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" }, searchPath, doc);

        mx::readFromXmlFile(doc, testPath / mtlxFile, searchPath);
        mtlxFile.removeExtension();

        bool translated = false;
        try
        {
            shaderTranslator->translateAllMaterials(doc, "UsdPreviewSurface");
            translated = true;
        }
        catch (mx::Exception &e)
        {
            std::cout << "Failed translating: " << (testPath / mtlxFile).asString() << ": " << e.what() << std::endl;
        }
        REQUIRE(translated);

        std::string validationErrors;
        bool valid = doc->validate(&validationErrors);
        if (!doc->validate(&validationErrors))
        {
            std::cout << "Shader translation of " << (testPath / mtlxFile).asString() << " failed" << std::endl;
            std::cout << "Validation errors: " << validationErrors << std::endl;
        }
        REQUIRE(valid);
    }
}

TEST_CASE("GenShader: Transparency Regression Check", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    const mx::FilePath resourcePath = searchPath.find("resources");
    mx::StringVec failedTests;
    mx::FilePathVec testFiles = { 
        "Materials/Examples/StandardSurface/standard_surface_default.mtlx", 
        "Materials/Examples/StandardSurface/standard_surface_glass.mtlx",
        "Materials/TestSuite/libraries/metal/brass_wire_mesh.mtlx"
    };
    std::vector<bool> transparencyTest = { false, true, true };
    for (size_t i=0; i<testFiles.size(); i++)
    {
        const mx::FilePath& testFile = resourcePath / testFiles[i];
        bool testValue = transparencyTest[i];

        mx::DocumentPtr testDoc = mx::createDocument();
        testDoc->setDataLibrary(libraries);

        try
        {
            mx::readFromXmlFile(testDoc, testFile, searchPath);
            std::vector<mx::TypedElementPtr> renderables = mx::findRenderableElements(testDoc);
            for (auto renderable : renderables)
            {
                mx::NodePtr node = renderable->asA<mx::Node>();
                if (!node)
                {
                    continue;
                }
                if (testValue != mx::isTransparentSurface(node))
                {
                    failedTests.push_back(std::string("File: ") + testFile.asString() + std::string(". Element: ")
                        + renderable->getNamePath() + std::string(" should be:" + std::to_string(testValue)));
                }
            }
        }
        catch (mx::Exception& e)
        {
            INFO(std::string("Test failed: ") + std::string(e.what()));
        }
    }
    for (auto failedTest : failedTests)
    {
        INFO(failedTest);
    }
    CHECK(failedTests.empty());
}

void testDeterministicGeneration(mx::DocumentPtr libraries, mx::GenContext& context)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    mx::string testElement = "SR_marble1";

    const size_t numRuns = 10;
    mx::vector<mx::DocumentPtr> testDocs(numRuns);
    mx::StringVec sourceCode(numRuns);

    for (size_t i = 0; i < numRuns; ++i)
    {
        mx::DocumentPtr testDoc = mx::createDocument();
        mx::readFromXmlFile(testDoc, testFile);
        testDoc->setDataLibrary(libraries);

        // Keep the document alive to make sure
        // new memory is allocated for each run
        testDocs[i] = testDoc;

        mx::ElementPtr element = testDoc->getChild(testElement);
        CHECK(element);

        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
        sourceCode[i] = shader->getSourceCode();

        if (i > 0)
        {
            // Check if the generated source code is the same
            // for each successive run.
            CHECK(sourceCode[i] == sourceCode[0]);
        }
    }
}

TEST_CASE("GenShader: Deterministic Generation", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
}

void checkPixelDependencies(mx::DocumentPtr libraries, mx::GenContext& context)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/GltfPbr/gltf_pbr_boombox.mtlx");
    mx::string testElement = "Material_boombox";

    mx::DocumentPtr testDoc = mx::createDocument();
    mx::readFromXmlFile(testDoc, testFile);
    testDoc->setDataLibrary(libraries);

    mx::ElementPtr element = testDoc->getChild(testElement);
    CHECK(element);

    mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    std::set<std::string> dependencies = shader->getStage("pixel").getSourceDependencies();
    for (auto dependency : dependencies) {
        mx::FilePath path(dependency);
        REQUIRE(path.exists() == true);
    }
}

TEST_CASE("GenShader: Track Dependencies", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
}

void variableTracker(mx::ShaderNode* node, mx::GenContext& /*context*/)
{
    static mx::StringMap results;
    results["primvar_one"] = "geompropvalue1/geomprop";
    results["primvar_two"] = "geompropvalue2/geomprop";
    results["0"] = "Tworld";
    results["upstream_primvar"] = "constant/value";

    if (node->hasClassification(mx::ShaderNode::Classification::GEOMETRIC))
    {
        const mx::ShaderInput* geomPropInput = node->getInput("geomprop");
        if (geomPropInput && geomPropInput->getValue())
        {
            std::string prop = geomPropInput->getValue()->getValueString();
            REQUIRE(results.count(prop));
            REQUIRE(results[prop] == geomPropInput->getPath());
        }
        else
        {
            const mx::ShaderInput* indexIput = node->getInput("index");
            if (indexIput && indexIput->getValue())
            {
                std::string prop = indexIput->getValue()->getValueString();
                REQUIRE(results.count(prop));
                REQUIRE(results[prop] == indexIput->getPath());
            }
        }
    }
}

TEST_CASE("GenShader: Track Application Variables", "[genshader]")
{
    std::string testDocumentString = 
    "<?xml version=\"1.0\"?> \
      <materialx version=\"1.38\"> \
      <geompropvalue name=\"geompropvalue\" type=\"color3\" >  \
        <input name=\"geomprop\" type=\"string\" uniform=\"true\" nodename=\"constant\" /> \
      </geompropvalue> \
      <geompropvalue name=\"geompropvalue1\" type=\"color3\" > \
        <input name=\"geomprop\" type=\"string\" uniform=\"true\" value=\"primvar_one\" /> \
      </geompropvalue> \
      <geompropvalue name=\"geompropvalue2\" type=\"color3\" > \
        <input name=\"geomprop\" type=\"string\" uniform=\"true\" value=\"primvar_two\" /> \
      </geompropvalue> \
      <multiply name=\"multiply\" type=\"color3\" > \
        <input name=\"in1\" type=\"color3\" nodename=\"geompropvalue\" /> \
        <input name=\"in2\" type=\"color3\" nodename=\"geompropvalue1\" /> \
      </multiply> \
      <add name=\"add\" type=\"color3\"  > \
        <input name=\"in1\" type=\"color3\" nodename=\"multiply\" /> \
        <input name=\"in2\" type=\"color3\" nodename=\"geompropvalue2\" /> \
      </add> \
      <standard_surface name=\"standard_surface\" type=\"surfaceshader\" > \
        <input name=\"base_color\" type=\"color3\" nodename=\"add\" /> \
      </standard_surface> \
      <constant name=\"constant\" type=\"string\" > \
        <input name=\"value\" type=\"string\" uniform=\"true\" value=\"upstream_primvar\" /> \
      </constant> \
      <surfacematerial name=\"surfacematerial\" type=\"material\" > \
        <input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"standard_surface\" /> \
      </surfacematerial> \
    </materialx>";

    const mx::string testElement = "surfacematerial";

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::DocumentPtr testDoc = mx::createDocument();
    mx::readFromXmlString(testDoc, testDocumentString);
    testDoc->setDataLibrary(libraries);

    mx::ElementPtr element = testDoc->getChild(testElement);
    CHECK(element);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
}

TEST_CASE("GenShader: No-op Color Spaces", "[genshader]")
{
    // DefaultColorManagementSystem treats "none" and "data" as requiring no
    // color transformation, regardless of the source/target working space.
    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create("genglsl");
    CHECK(colorManagementSystem->isNoOpColorSpace("none"));
    CHECK(colorManagementSystem->isNoOpColorSpace("data"));
    CHECK(!colorManagementSystem->isNoOpColorSpace("lin_rec709_scene"));
    // Any unrecognized color space is not considered a NoOp.
    CHECK(!colorManagementSystem->isNoOpColorSpace("Raw"));

#ifdef MATERIALX_BUILD_OCIO
    // OcioColorManagementSystem inherits the "none"/"data" no-op behavior
    // from DefaultColorManagementSystem, and additionally treats any OCIO
    // color space flagged isData() (e.g. "Raw" in the ACES/studio configs)
    // as a no-op, even though that name means nothing to the default system.
    //
    // Unlike the "data"/"bogus_colorspace" shader-generation checks below,
    // there is no equivalent generate()-based integration test for this
    // OCIO-specific behavior: OcioColorManagementSystemImpl::getNodeDef()
    // already substitutes a passthrough <dot> node whenever the underlying
    // OCIO GPU processor itself reports isNoOp() (see OcioColorManagementSystem.cpp),
    // independent of ColorManagementSystem::isNoOpColorSpace(). 
    try
    {
        mx::OcioColorManagementSystemPtr ocioColorManagementSystem =
            mx::OcioColorManagementSystem::createFromBuiltinConfig("ocio://cg-config-latest", "genglsl");
        CHECK(ocioColorManagementSystem->isNoOpColorSpace("none"));
        CHECK(ocioColorManagementSystem->isNoOpColorSpace("data"));
        CHECK(ocioColorManagementSystem->isNoOpColorSpace("Raw"));
        CHECK(!ocioColorManagementSystem->isNoOpColorSpace("lin_rec709_scene"));
        CHECK(!ocioColorManagementSystem->isNoOpColorSpace("ACEScg"));
    }
    catch (const std::exception& e)
    {
        WARN(std::string("Could not create OcioColorManagementSystem from builtin config: ") + e.what());
    }
#endif

#ifdef MATERIALX_BUILD_GEN_GLSL
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    colorManagementSystem = mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    colorManagementSystem->loadLibrary(libraries);
    context.getShaderGenerator().setColorManagementSystem(colorManagementSystem);

    // A color3 tagged "data" is not the working color space, and has no
    // corresponding transform nodedef (e.g. no "data_to_lin_rec709_scene"
    // node exists). If isNoOpColorSpace() were not consulted before looking
    // up a transform, ShaderGraph::populateColorTransformMap would throw
    // ExceptionShaderGenError("Unsupported color space transform ...").
    std::string noOpDocString =
    "<?xml version=\"1.0\"?> \
      <materialx version=\"1.39\" colorspace=\"lin_rec709_scene\"> \
        <constant name=\"data_constant\" type=\"color3\"> \
          <input name=\"value\" type=\"color3\" value=\"0.5, 0.5, 0.5\" colorspace=\"data\" /> \
        </constant> \
        <surface_unlit name=\"data_surface\" type=\"surfaceshader\"> \
          <input name=\"emission_color\" type=\"color3\" nodename=\"data_constant\" /> \
        </surface_unlit> \
      </materialx>";
    mx::DocumentPtr noOpDoc = mx::createDocument();
    mx::readFromXmlString(noOpDoc, noOpDocString);
    noOpDoc->setDataLibrary(libraries);
    mx::ElementPtr noOpElement = noOpDoc->getChild("data_surface");
    REQUIRE(noOpElement);
    REQUIRE_NOTHROW(context.getShaderGenerator().generate("data_surface", noOpElement, context));

    // Sanity check that the harness above is capable of catching a real
    // mismatch: an unrecognized, non-no-op color space with no transform
    // nodedef must still throw.
    std::string unsupportedDocString =
    "<?xml version=\"1.0\"?> \
      <materialx version=\"1.39\" colorspace=\"lin_rec709_scene\"> \
        <constant name=\"bogus_constant\" type=\"color3\"> \
          <input name=\"value\" type=\"color3\" value=\"0.5, 0.5, 0.5\" colorspace=\"bogus_colorspace\" /> \
        </constant> \
        <surface_unlit name=\"bogus_surface\" type=\"surfaceshader\"> \
          <input name=\"emission_color\" type=\"color3\" nodename=\"bogus_constant\" /> \
        </surface_unlit> \
      </materialx>";
    mx::DocumentPtr unsupportedDoc = mx::createDocument();
    mx::readFromXmlString(unsupportedDoc, unsupportedDocString);
    unsupportedDoc->setDataLibrary(libraries);
    mx::ElementPtr unsupportedElement = unsupportedDoc->getChild("bogus_surface");
    REQUIRE(unsupportedElement);
    REQUIRE_THROWS_AS(context.getShaderGenerator().generate("bogus_surface", unsupportedElement, context),
                      mx::ExceptionShaderGenError);
#endif
}

TEST_CASE("GenShader: User-Facing Color Space Names", "[genshader]")
{
    // DefaultColorManagementSystem translates both color interop forum IDs and their
    // deprecated legacy equivalents to the same user-facing display name.
    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create("genglsl");
    CHECK(colorManagementSystem->getUserFacingName("lin_rec709_scene") == "Linear Rec.709 (sRGB)");
    CHECK(colorManagementSystem->getUserFacingName("lin_rec709") == "Linear Rec.709 (sRGB)");
    CHECK(colorManagementSystem->getUserFacingName("lin_ap1_scene") == "ACEScg");
    CHECK(colorManagementSystem->getUserFacingName("acescg") == "ACEScg");
    CHECK(colorManagementSystem->getUserFacingName("lin_ap0_scene") == "ACES2065-1");
    CHECK(colorManagementSystem->getUserFacingName("none") == "Data");
    CHECK(colorManagementSystem->getUserFacingName("data") == "Data");
    // An unrecognized color space is returned unchanged.
    CHECK(colorManagementSystem->getUserFacingName("bogus_colorspace") == "bogus_colorspace");

#ifdef MATERIALX_BUILD_OCIO
    try
    {
        mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
        mx::FilePath minimalConfigFile = searchPath.find(
            "resources/Materials/TestSuite/stdlib/color_management/minimal_config.ocio");
        REQUIRE(minimalConfigFile.exists());

        mx::OcioColorManagementSystemPtr ocioColorManagementSystem =
            mx::OcioColorManagementSystem::createFromFile(minimalConfigFile.asString(), "genglsl");

        // The OcioColorManagementSystem checks its own config first. This color space is defined
        // there but not in the built-in cg-config or the DefaultColorManagementSystem.
        CHECK(ocioColorManagementSystem->getUserFacingName("ocio:arrilogc4_awg4_scene") == "ARRI camera LogC4");

        // This color space is known to both its own config and the DefaultColorManagementSystem
        // but the config author's name gets first preference.
        CHECK(ocioColorManagementSystem->getUserFacingName("srgb_rec709_scene") == "sRGB (Scene-referred)");

        // This color space is not in its config, so the fallback will come from the DefaultColorManagementSystem.
        CHECK(ocioColorManagementSystem->getUserFacingName("lin_rec709_scene") == "Linear Rec.709 (sRGB)");

        // In this case, the color space string is not in its own config or the DefaultColorManagementSystem,
        // so the IdentifyBuiltinColorSpace fallback in getSupportedColorSpaceName is invoked. This will find
        // a color space for the string in the built-in cg-config. The transform math is then compared against
        // the color spaces in the minimal_config.ocio, where it finds a match.
        CHECK(ocioColorManagementSystem->getUserFacingName("srgb_encoded_ap1_tx") == "sRGB AP1");

        // A totally unrecognized color space name is returned unmodified.
        CHECK(ocioColorManagementSystem->getUserFacingName("bogus_colorspace") == "bogus_colorspace");
    }
    catch (const std::exception& e)
    {
        WARN(std::string("Could not create OcioColorManagementSystem from minimal_config.ocio: ") + e.what());
    }
#endif
}
