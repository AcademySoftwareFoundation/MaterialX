//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXFormat/Util.h>

#include <MaterialXGenShader/Shader.h>

#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>

namespace mx = MaterialX;

//
// Render validation tester for the GLSL shading language
//
class OgsFxShaderRenderTester : public RenderUtil::ShaderRenderTester
{
public:
    explicit OgsFxShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
        RenderUtil::ShaderRenderTester(shaderGenerator)
    {
    }

protected:
    void loadAdditionalLibraries(mx::DocumentPtr document,
                                 GenShaderUtil::TestSuiteOptions& options) override;

    void registerLights(mx::DocumentPtr document, const GenShaderUtil::TestSuiteOptions &options,
                        mx::GenContext& context) override;

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

    mx::LightHandlerPtr _lightHandler;
};

// In addition to standard texture and shader definition libraries, additional lighting files
// are loaded in. If no files are specifed in the input options, a sample
// compound light type and a set of lights in a "light rig" are loaded in to a given
// document.
void OgsFxShaderRenderTester::loadAdditionalLibraries(mx::DocumentPtr document,
    GenShaderUtil::TestSuiteOptions& options)
{
    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/Lights");
    for (const auto& lightFile : options.lightFiles)
    {
        loadLibrary(lightDir / mx::FilePath(lightFile), document);
    }
}

// Create a light handler and populate it based on lights found in a given document
void OgsFxShaderRenderTester::registerLights(mx::DocumentPtr document,
    const GenShaderUtil::TestSuiteOptions &/*options*/,
    mx::GenContext& context)
{
    _lightHandler = mx::LightHandler::create();

    // Scan for lights
    std::vector<mx::NodePtr> lights;
    _lightHandler->findLights(document, lights);
    _lightHandler->registerLights(document, lights, context);

    // Set the list of lights on the with the generator
    _lightHandler->setLightSources(lights);
}

void OgsFxShaderRenderTester::createRenderer(std::ostream& /*log*/)
{
}

bool OgsFxShaderRenderTester::runRenderer(const std::string& shaderName,
                                           mx::TypedElementPtr element,
                                           mx::GenContext& context,
                                           mx::DocumentPtr doc,
                                           std::ostream& log,
                                           const GenShaderUtil::TestSuiteOptions& testOptions,
                                           RenderUtil::RenderProfileTimes& profileTimes,
                                           const mx::FileSearchPath& /*imageSearchPath*/,
                                           const std::string& outputPath,
                                           mx::ImageVec* /*imageVec*/)
{
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.languageTimes.totalTime, "OgsFx total time");

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
        log << "------------ Run GLSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.languageTimes.ioTime, "OgsFx dir time");
                outputFilePath.createDirectory();
            }

            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer transpTimer(profileTimes.languageTimes.transparencyTime, "OgsFx transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shadergen.getTarget());
                transpTimer.endTimer();

                RenderUtil::AdditiveScopedTimer generationTimer(profileTimes.languageTimes.generationTime, "OgsFx generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.targetColorSpaceOverride = "lin_rec709";
                contextOptions.fileTextureVerticalFlip = true;
                contextOptions.hwSpecularEnvironmentMethod = testOptions.specularEnvironmentMethod;
                shader = shadergen.generate(shaderName, element, context);
                generationTimer.endTimer();
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
            const std::string& fxSourceCode = shader->getSourceCode(mx::Stage::EFFECT);
            CHECK(fxSourceCode.length() > 0);

            if (testOptions.dumpGeneratedCode)
            {
                RenderUtil::AdditiveScopedTimer dumpTimer(profileTimes.languageTimes.ioTime, "OgsFx I/O time");
                std::ofstream file;
                file.open(shaderPath + ".ogsfx");
                file << fxSourceCode;
                file.close();
            }

            // TODO: Validate compilation.
            // TODO: Validate rendering.
        }
    }

    return true;
}

TEST_CASE("Render: OgsFx TestSuite", "[renderogsfx]")
{
    // Use the Maya version of the OgsFx generator,
    // so we can render the shaders in Maya's viewport.
    OgsFxShaderRenderTester renderTester(mx::MayaGlslPluginShaderGenerator::create());

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
