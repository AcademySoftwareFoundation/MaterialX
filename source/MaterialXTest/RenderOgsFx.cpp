//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/RenderUtil.h>

#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>

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
    void loadLibraries(mx::DocumentPtr document,
                       RenderUtil::RenderTestOptions& options) override;

    void registerLights(mx::DocumentPtr document, const RenderUtil::RenderTestOptions &options,
                        mx::GenContext& context) override;

    void createValidator(std::ostream& log) override;

    bool runValidator(const std::string& shaderName,
        mx::TypedElementPtr element,
        mx::GenContext& context,
        mx::DocumentPtr doc,
        std::ostream& log,
        const RenderUtil::RenderTestOptions& testOptions,
        RenderUtil::RenderProfileTimes& profileTimes,
        const mx::FileSearchPath& imageSearchPath,
        const std::string& outputPath = ".") override;

    void getImplementationWhiteList(mx::StringSet& whiteList) override;

    mx::LightHandlerPtr _lightHandler;
};

// In addition to standard texture and shader definition libraries, additional lighting files
// are loaded in. If no files are specifed in the input options, a sample
// compound light type and a set of lights in a "light rig" are loaded in to a given
// document.
void OgsFxShaderRenderTester::loadLibraries(mx::DocumentPtr document,
    RenderUtil::RenderTestOptions& options)
{
    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/Lights");
    for (auto lightFile : options.lightFiles)
    {
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath(lightFile), document);
    }
}

// Create a light handler and populate it based on lights found in a given document
void OgsFxShaderRenderTester::registerLights(mx::DocumentPtr document,
    const RenderUtil::RenderTestOptions &options,
    mx::GenContext& context)
{
    _lightHandler = mx::LightHandler::create();
    RenderUtil::createLightRig(document, *_lightHandler, context,
        options.irradianceIBLPath, options.radianceIBLPath);
}

void OgsFxShaderRenderTester::createValidator(std::ostream& /*log*/)
{
}

bool OgsFxShaderRenderTester::runValidator(const std::string& shaderName,
                                           mx::TypedElementPtr element,
                                           mx::GenContext& context,
                                           mx::DocumentPtr doc,
                                           std::ostream& log,
                                           const RenderUtil::RenderTestOptions& testOptions,
                                           RenderUtil::RenderProfileTimes& profileTimes,
                                           const mx::FileSearchPath& /*imageSearchPath*/,
                                           const std::string& outputPath)
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
                options.hwTransparency = mx::isTransparentSurface(element, shadergen);
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

void OgsFxShaderRenderTester::getImplementationWhiteList(mx::StringSet& whiteList)
{
    whiteList =
    {
        "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
        "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue", "IM_light_genglsl",
        "IM_point_light_genglsl", "IM_spot_light_genglsl", "IM_directional_light_genglsl"
    };
}

TEST_CASE("Render: OgsFx TestSuite", "[renderglsl]")
{
    // Use the Maya version of the OgsFx generator,
    // so we can render the shaders in Maya's viewport.
    OgsFxShaderRenderTester renderTester(mx::MayaGlslPluginShaderGenerator::create());

    mx::FilePathVec testRootPaths;
    mx::FilePath testRoot = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    testRootPaths.push_back(testRoot);
    const mx::FilePath testRoot2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    testRootPaths.push_back(testRoot2);

    mx::FilePath optionsFilePath = testRoot / mx::FilePath("_options.mtlx");

    renderTester.validate(testRootPaths, optionsFilePath);
}
