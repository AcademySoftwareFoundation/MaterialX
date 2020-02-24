//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/Shader.h>

#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenOgsXml/OgsFragment.h>

#include <MaterialXTest/GenShaderUtil.h>
#include <MaterialXTest/GenGlsl.h>

#ifdef MATERIALX_BUILD_CROSS
#include <MaterialXCross/Cross.h>
#endif

namespace mx = MaterialX;

class OgsXmlShaderGeneratorTester : public GlslShaderGeneratorTester
{
public:
    OgsXmlShaderGeneratorTester(
        const mx::FilePathVec& testRootPaths,
        const mx::FilePath& libSearchPath,
        const mx::FileSearchPath& srcSearchPath,
        const mx::FilePath& logFilePath
    )
        : GlslShaderGeneratorTester(
            mx::GlslFragmentGenerator::create(),
            testRootPaths,
            libSearchPath,
            srcSearchPath,
            logFilePath
        )
    {
        dumpMaterials = { "Tiled_Brass", "Brass_Wire_Mesh" };
    }

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::PIXEL);
    }

    // Ignore light shaders in the document for GLSL fragments since MaterialX
    // lights and not supported in OGS/VP2.
    void findLights(mx::DocumentPtr /*doc*/, std::vector<mx::NodePtr>& lights) override
    {
        lights.clear();
    }

    // Generate source code for a given element and check that code was produced.
    bool generateCode(
        mx::GenContext& context, const std::string& /*shaderName*/, mx::TypedElementPtr element,
        std::ostream& log, mx::StringVec /*testStages*/, mx::StringVec& /*sourceCode*/
    ) override
    {
        std::unique_ptr<MaterialXMaya::OgsFragment> fragment;
        try
        {
            fragment.reset(new MaterialXMaya::OgsFragment(element, context));
        }
        catch (mx::Exception& e)
        {
            log << ">> Fragment Code generation failure: " << e.what() << "\n";
        }
        CHECK(fragment);
        if (!fragment)
        {
            log << ">> Failed to generate fragment for element: " << element->getNamePath() << std::endl;
            return false;
        }

        if (mx::ShaderRefPtr shaderRef = element->asA<mx::ShaderRef>())
        {
            mx::MaterialPtr material = shaderRef->getParent()->asA<mx::Material>();
            if (material && dumpMaterials.count(material->getName()))
            {
                std::ofstream file(material->getName() + ".xml");
                file << fragment->getFragmentSource();
                file.close();
            }
        }

        return true;
    }

protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geompropvalue", "IM_light_genglsl",
            "IM_point_light_genglsl", "IM_spot_light_genglsl", "IM_directional_light_genglsl", "IM_angle", "material", "ND_material"
        };
    }

    mx::StringSet dumpMaterials;
};

static void generateXmlCode()
{
    const mx::FilePathVec testRootPaths{
        mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite"),
        mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface"),
        mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface"),
        mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/Units")
    };

    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    const mx::FilePath logPath("genogsxml_generate_test.txt");

    OgsXmlShaderGeneratorTester tester(testRootPaths, libSearchPath, srcSearchPath, logPath);

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = testRootPaths.front() / mx::FilePath("_options.mtlx");

#ifdef MATERIALX_BUILD_CROSS
    mx::Cross::initialize();
#endif
    tester.validate(genOptions, optionsFilePath);
#ifdef MATERIALX_BUILD_CROSS
    mx::Cross::finalize();
#endif
}

TEST_CASE("GenShader: OGS XML Shader Generation", "[genogsxml]")
{
    generateXmlCode();
}
