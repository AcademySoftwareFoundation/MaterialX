//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

namespace mx = MaterialX;

class ArnoldShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    ArnoldShaderRenderTester() :
        RenderUtil::ShaderRenderTester(mx::ArnoldShaderGenerator::create())
    {
    }

  protected:
    void registerSourceCodeSearchPaths(mx::GenContext& context) override
    {
        // Include extra OSL implementation files
        mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
        context.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Include current path to find resources.
        context.registerSourceCodeSearchPath(mx::FilePath::getCurrentPath());
    }

    void createRenderer(std::ostream& log) override;

    bool runRenderer(const std::string& shaderName,
                      mx::TypedElementPtr element,
                      mx::GenContext& context,
                      mx::DocumentPtr doc,
                      std::ostream& log,
                      const GenShaderUtil::TestSuiteOptions& testOptions,
                      RenderUtil::RenderProfileTimes& profileTimes,
                      const mx::FileSearchPath& imageSearchPath,
                      const std::string& outputPath = ".",
                      mx::ImageVec* imageVec = nullptr) override;
};

void ArnoldShaderRenderTester::createRenderer(std::ostream& /*log*/)
{
}

bool ArnoldShaderRenderTester::runRenderer(const std::string& shaderName,
                                            mx::TypedElementPtr element,
                                            mx::GenContext& context,
                                            mx::DocumentPtr doc,
                                            std::ostream& log,
                                            const GenShaderUtil::TestSuiteOptions& testOptions,
                                            RenderUtil::RenderProfileTimes& profileTimes,
                                            const mx::FileSearchPath& /*imageSearchPath*/,
                                            const std::string& outputPath,
                                            mx::ImageVec* /*returnImage*/)
{
    RenderUtil::AdditiveScopedTimer totalArnoldTime(profileTimes.languageTimes.totalTime, "Arnold total time");

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    // Perform validation if requested
    if (testOptions.validateElementToRender)
    {
        std::string message;
        if (!element->validate(&message))
        {
            log << "Element is invalid: " << message << std::endl;
            return false;
        }
    }

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run OSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (const auto& options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer genTimer(profileTimes.languageTimes.generationTime, "Arnold generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.targetColorSpaceOverride = "lin_rec709";
                shader = shadergen.generate(shaderName, element, context);
            }
            catch (mx::Exception& e)
            {
                log << ">> " << e.what() << "\n";
                shader = nullptr;
            }
            CHECK(shader != nullptr);
            if (shader == nullptr)
            {
                log << ">> Failed to generate shader\n";
                return false;
            }
            CHECK(shader->getSourceCode().length() > 0);

            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.languageTimes.ioTime, "Arnold dir time");
                outputFilePath.createDirectory();
            }

            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName + "_arnold");

            // Write out osl file
            if (testOptions.dumpGeneratedCode)
            {
                RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.languageTimes.ioTime, "Arnold I/O time");
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();
            }

            // TODO: Validate compilation using Arnold's OSL compiler (oslc).
            // TODO: Validate rendering using Arnold's command line renderer (kick).
        }
    }

    return true;
}

TEST_CASE("Render: Arnold TestSuite", "[renderosl]")
{
    ArnoldShaderRenderTester renderTester;

    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    const mx::FilePath testRootPath3 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    testRootPaths.push_back(testRootPath2);
    testRootPaths.push_back(testRootPath3);

    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");

    renderTester.validate(testRootPaths, optionsFilePath);
}
