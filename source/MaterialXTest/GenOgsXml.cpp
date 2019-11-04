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

#include <MaterialXTest/GenShaderUtil.h>
#include <MaterialXTest/GenGlsl.h>

namespace mx = MaterialX;

class OgsXmlShaderGeneratorTester : public GlslShaderGeneratorTester
{
public:
    OgsXmlShaderGeneratorTester(const mx::FilePathVec& testRootPaths, const mx::FilePath& libSearchPath,
        const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath) :
        GlslShaderGeneratorTester(mx::GlslFragmentGenerator::create(), testRootPaths, libSearchPath, srcSearchPath, logFilePath)
    {
        dumpMaterials = { "Tiled_Brass", "Brass_Wire_Mesh" };
    }

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::PIXEL);
    }

    // Generate source code for a given element and check that code was produced.
    bool generateCode(mx::GenContext& context, const std::string& shaderName, mx::TypedElementPtr element,
        std::ostream& log, mx::StringVec testStages, mx::StringVec&) override
    {
        mx::ShaderPtr shader = nullptr;
        try
        {
            shader = context.getShaderGenerator().generate(shaderName, element, context);
        }
        catch (mx::Exception& e)
        {
            log << ">> GLSL Code generation failure: " << e.what() << "\n";
            shader = nullptr;
        }
        CHECK(shader);
        if (!shader)
        {
            log << ">> Failed to generate GLSL shader for element: " << element->getNamePath() << std::endl;
            return false;
        }

        std::string cleanShaderName = mx::createValidName(shaderName);
        std::ostringstream sourceStream;

        constexpr bool hwTransparency = false;
        xmlGenerator.generate(cleanShaderName, shader.get(), nullptr, hwTransparency, sourceStream);
        std::string fragmentSource = sourceStream.str();
        if (fragmentSource.empty())
        {
            log << ">> Failed to generate XML code." << std::endl;
            return false;
        }

        mx::ShaderRefPtr shaderRef = element->asA<mx::ShaderRef>();
        if (shaderRef)
        {
            mx::MaterialPtr material = shaderRef->getParent()->asA<mx::Material>();
            if (material && dumpMaterials.count(material->getName()))
            {
                std::ofstream file(material->getName() + ".xml");
                file << fragmentSource;
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
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue", "IM_light_genglsl",
            "IM_point_light_genglsl", "IM_spot_light_genglsl", "IM_directional_light_genglsl", "IM_angle"
        };
    }

    mx::OgsXmlGenerator xmlGenerator;
    mx::StringSet dumpMaterials;
};

static void generateXmlCode()
{
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    const mx::FilePath testRootPath3 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface");
    const mx::FilePath testRootPath4 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/Units");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    testRootPaths.push_back(testRootPath2);
    testRootPaths.push_back(testRootPath3);
    testRootPaths.push_back(testRootPath4);
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    const mx::FilePath logPath("genogsxml_generate_test.txt");

    OgsXmlShaderGeneratorTester tester(testRootPaths, libSearchPath, srcSearchPath, logPath);

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}

TEST_CASE("GenShader: OGS XML Shader Generation", "[genogsxml]")
{
    generateXmlCode();
}
