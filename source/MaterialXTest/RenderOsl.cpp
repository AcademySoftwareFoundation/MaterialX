//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXRenderOsl/OslValidator.h>

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/RenderUtil.h>

namespace mx = MaterialX;

class OslShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    explicit OslShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
        RenderUtil::ShaderRenderTester(shaderGenerator)
    {
    }

  protected:
    void registerSourceCodeSearchPaths(mx::GenContext& context) override
    {
        // Include extra OSL implementation files
        mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
        context.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));
    }

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

    mx::OslValidatorPtr _validator;
};

// Validator setup
void OslShaderRenderTester::createValidator(std::ostream& log)
{
    bool initialized = false;

    _validator = mx::OslValidator::create();

    // Set up additional utilities required to run OSL testing including
    // oslc and testrender paths and OSL include path
    //
    const std::string oslcExecutable(MATERIALX_OSLC_EXECUTABLE);
    _validator->setOslCompilerExecutable(oslcExecutable);
    const std::string testRenderExecutable(MATERIALX_TESTRENDER_EXECUTABLE);
    _validator->setOslTestRenderExecutable(testRenderExecutable);
    _validator->setOslIncludePath(mx::FilePath(MATERIALX_OSL_INCLUDE_PATH));

    try
    {
        _validator->initialize();
        _validator->setImageHandler(nullptr);
        _validator->setLightHandler(nullptr);
        initialized = true;

        // Pre-compile some required shaders for testrender
        if (!oslcExecutable.empty() && !testRenderExecutable.empty())
        {
            mx::FilePath shaderPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/");
            _validator->setOslOutputFilePath(shaderPath);

            const std::string OSL_EXTENSION("osl");
            mx::FilePathVec files = shaderPath.getFilesInDirectory(OSL_EXTENSION);
            for (auto file : files)
            {
                mx::FilePath filePath = shaderPath / file;
                _validator->compileOSL(filePath.asString());
            }

            // Set the search path for these compiled shaders.
            _validator->setOslUtilityOSOPath(shaderPath);
        }
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);
}

// Validator execution
bool OslShaderRenderTester::runValidator(const std::string& shaderName,
                                         mx::TypedElementPtr element,
                                         mx::GenContext& context,
                                         mx::DocumentPtr doc,
                                         std::ostream& log,
                                         const RenderUtil::RenderTestOptions& testOptions,
                                         RenderUtil::RenderProfileTimes& profileTimes,
                                         const mx::FileSearchPath& imageSearchPath,
                                         const std::string& outputPath)
{
    RenderUtil::AdditiveScopedTimer totalOSLTime(profileTimes.languageTimes.totalTime, "OSL total time");

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

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer genTimer(profileTimes.languageTimes.generationTime, "OSL generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
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
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.languageTimes.ioTime, "OSL dir time");
                outputFilePath.createDirectory();
            }

            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            // Write out osl file
            if (testOptions.dumpGeneratedCode)
            {
                RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.languageTimes.ioTime, "OSL I/O time");
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();
            }

            if (!testOptions.compileCode)
            {
                return false;
            }

            // Validate
            bool validated = false;
            try
            {
                // Set output path and shader name
                _validator->setOslOutputFilePath(outputFilePath);
                _validator->setOslShaderName(shaderName);

                // Validate compilation
                {
                    RenderUtil::AdditiveScopedTimer compileTimer(profileTimes.languageTimes.compileTime, "OSL compile time");
                    _validator->validateCreation(shader);
                }

                if (testOptions.renderImages)
                {
                    const mx::ShaderStage& stage = shader->getStage(mx::Stage::PIXEL);

                    // Look for textures and build parameter override string for each image
                    // files if a relative path maps to an absolute path
                    const mx::VariableBlock& uniforms = stage.getUniformBlock(mx::OSL::UNIFORMS);

                    mx::StringVec overrides;
                    mx::StringVec envOverrides;
                    mx::StringMap separatorMapper;
                    separatorMapper["\\\\"] = "/";
                    separatorMapper["\\"] = "/";
                    for (size_t i = 0; i<uniforms.size(); ++i)
                    {
                        const mx::ShaderPort* uniform = uniforms[i];

                        // Bind input images
                        if (uniform->getType() != MaterialX::Type::FILENAME)
                        {
                            continue;
                        }
                        if (uniform->getValue())
                        {
                            const std::string& uniformName = uniform->getName();
                            mx::FilePath filename;
                            mx::FilePath origFilename(uniform->getValue()->getValueString());
                            if (!origFilename.isAbsolute())
                            {
                                filename = imageSearchPath.find(origFilename);
                                if (filename != origFilename)
                                {
                                    std::string overrideString("string " + uniformName + " \"" + filename.asString() + "\";\n");
                                    overrideString = mx::replaceSubstrings(overrideString, separatorMapper);
                                    overrides.push_back(overrideString);
                                }
                            }
                        }
                    }
                    // Bind IBL image name overrides.
                    std::string envmap_filename("string envmap_filename \"resources/Images/san_giuseppe_bridge.hdr\";\n");
                    envOverrides.push_back(envmap_filename);

                    _validator->setShaderParameterOverrides(overrides);
                    _validator->setEnvShaderParameterOverrides(envOverrides);

                    const mx::VariableBlock& outputs = stage.getOutputBlock(mx::OSL::OUTPUTS);
                    if (outputs.size() > 0)
                    {
                        const mx::ShaderPort* output = outputs[0];
                        const mx::TypeSyntax& typeSyntax = shadergen.getSyntax().getTypeSyntax(output->getType());

                        const std::string& outputName = output->getName();
                        const std::string& outputType = typeSyntax.getTypeAlias().empty() ? typeSyntax.getName() : typeSyntax.getTypeAlias();

                        static const std::string SHADING_SCENE_FILE = "closure_color_scene.xml";
                        static const std::string NON_SHADING_SCENE_FILE = "constant_color_scene.xml";
                        const std::string& sceneTemplateFile = mx::elementRequiresShading(element) ? SHADING_SCENE_FILE : NON_SHADING_SCENE_FILE;

                        // Set shader output name and type to use
                        _validator->setOslShaderOutput(outputName, outputType);

                        // Set scene template file. For now we only have the constant color scene file
                        mx::FilePath sceneTemplatePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/");
                        sceneTemplatePath = sceneTemplatePath / sceneTemplateFile;
                        _validator->setOslTestRenderSceneTemplateFile(sceneTemplatePath.asString());

                        // Validate rendering
                        {
                            RenderUtil::AdditiveScopedTimer renderTimer(profileTimes.languageTimes.renderTime, "OSL render time");
                            _validator->validateRender();
                        }
                    }
                    else
                    {
                        CHECK(false);
                        log << ">> Shader has no output to render from\n";
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError& e)
            {
                // Always dump shader on error
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump file: " << shaderPath << ".osl file" << std::endl;
            }
            catch (mx::Exception& e)
            {
                log << e.what();
            }
            CHECK(validated);
        }
    }

    return true;
}

void OslShaderRenderTester::getImplementationWhiteList(mx::StringSet& whiteList)
{
    whiteList =
    {
        "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
        "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue"
    };
}


TEST_CASE("Render: OSL TestSuite", "[renderosl]")
{
    OslShaderRenderTester renderTester(mx::OslShaderGenerator::create());

    mx::FilePathVec testRootPaths;
    mx::FilePath testRoot = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    testRootPaths.push_back(testRoot);
    const mx::FilePath testRoot2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    testRootPaths.push_back(testRoot2);

    mx::FilePath optionsFilePath = testRoot / mx::FilePath("_options.mtlx");

    renderTester.validate(testRootPaths, optionsFilePath);
}
