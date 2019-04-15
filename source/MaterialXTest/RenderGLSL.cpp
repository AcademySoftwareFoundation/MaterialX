//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(MATERIALX_TEST_RENDER) && defined(MATERIALX_BUILD_RENDERGLSL)

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXRenderGlsl/GlslValidator.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>

#ifdef MATERIALX_BUILD_CONTRIB
#include <MaterialXContrib/Handlers/TinyEXRImageLoader.h>
#endif
#ifdef MATERIALX_BUILD_OIIO
#include <MaterialXRender/Handlers/OiioImageLoader.h>
#endif
#include <MaterialXRender/Handlers/StbImageLoader.h>

#include <MaterialXRender/Handlers/GeometryHandler.h>
#include <MaterialXRender/Handlers/TinyObjLoader.h>

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/RenderUtil.h>

namespace mx = MaterialX;

//
// Render validation tester for the GLSL shading language
//
class GlslShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    GlslShaderRenderTester() :
        _languageTargetString(mx::GlslShaderGenerator::LANGUAGE + "_" +
        mx::GlslShaderGenerator::TARGET)
    {
    }

  protected:
    const std::string& languageTargetString() override
    {
        return _languageTargetString;
    }
    
    bool runTest(const RenderUtil::RenderTestOptions& testOptions) const override
    {
        return (testOptions.languageAndTargets.count(_languageTargetString) > 0);
    }

    void loadLibraries(mx::DocumentPtr document,
                       RenderUtil::RenderTestOptions& options) override;

    void createShaderGenerator() override
    {
        _shaderGenerator = mx::GlslShaderGenerator::create();
    }

    void registerLights(mx::DocumentPtr document, 
                        const RenderUtil::RenderTestOptions &options, mx::GenContext& context) override;

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

    std::string _languageTargetString;
    mx::GlslValidatorPtr _validator;
    mx::LightHandlerPtr _lightHandler;
};

// In addition to standard texture and shader definition libraries, additional lighting files
// are loaded in. If no files are specifed in the input options, a sample 
// compound light type and a set of lights in a "light rig" are loaded in to a given
// document.
void GlslShaderRenderTester::loadLibraries(mx::DocumentPtr document,
                                           RenderUtil::RenderTestOptions& options)
{
    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/Lights");
    if (options.lightFiles.size() == 0)
    {
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest.mtlx"), document);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest_ng.mtlx"), document);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("light_rig.mtlx"), document);
    }
    else
    {
        for (auto lightFile : options.lightFiles)
        {
            GenShaderUtil::loadLibrary(lightDir / mx::FilePath(lightFile), document);
        }
    }

}

// Create a light handler and populate it based on lights found in a given document
void GlslShaderRenderTester::registerLights(mx::DocumentPtr document,
                                            const RenderUtil::RenderTestOptions &options, 
                                            mx::GenContext& context)
{
    _lightHandler = mx::LightHandler::create();
    RenderUtil::createLightRig(document, *_lightHandler, context,
                               options.radianceIBLPath, options.irradianceIBLPath);
}


//
// Create a validator with the apporpraite image, geometry and light handlers.
// The light handler on the validator is cleared on initialization to indicate no lighting 
// is required. During code generation, if the element to validate requires lighting then
// the handler _lightHandler will be used.
//
void GlslShaderRenderTester::createValidator(std::ostream& log)
{
    bool initialized = false;
    try
    {
        _validator = mx::GlslValidator::create();
        _validator->initialize();

        // Set image handler on validator
        mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
        mx::GLTextureHandlerPtr imageHandler = mx::GLTextureHandler::create(stbLoader);
#ifdef MATERIALX_BUILD_CONTRIB
        mx::TinyEXRImageLoaderPtr exrLoader = mx::TinyEXRImageLoader::create();
        imageHandler->addLoader(exrLoader);
#endif
        _validator->setImageHandler(imageHandler);

        // Set geometry handler on validator
        mx::GeometryHandlerPtr geometryHandler = _validator->getGeometryHandler();
        std::string geometryFile = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/") / mx::FilePath("sphere.obj");
        if (!geometryHandler->hasGeometry(geometryFile))
        {
            geometryHandler->clearGeometry();
            geometryHandler->loadGeometry(geometryFile);
        }

        // Set light handler. 
        _validator->setLightHandler(nullptr);

        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        log << e.what() << std::endl;
    }
    REQUIRE(initialized);
}

bool GlslShaderRenderTester::runValidator(const std::string& shaderName,
                                          mx::TypedElementPtr element,
                                          mx::GenContext& context,
                                          mx::DocumentPtr doc,
                                          std::ostream& log,
                                          const RenderUtil::RenderTestOptions& testOptions,
                                          RenderUtil::RenderProfileTimes& profileTimes,
                                          const mx::FileSearchPath& imageSearchPath,
                                          const std::string& outputPath)
{
    RenderUtil::AdditiveScopedTimer totalGLSLTime(profileTimes.languageTimes.totalTime, "GLSL total time");

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
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.languageTimes.ioTime, "GLSL dir time");
                outputFilePath.createDirectory();
            }

            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer transpTimer(profileTimes.languageTimes.transparencyTime, "GLSL transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shadergen);
                transpTimer.endTimer();

                RenderUtil::AdditiveScopedTimer generationTimer(profileTimes.languageTimes.generationTime, "GLSL generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.fileTextureVerticalFlip = true;
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
            const std::string& vertexSourceCode = shader->getSourceCode(mx::Stage::VERTEX);
            const std::string& pixelSourceCode = shader->getSourceCode(mx::Stage::PIXEL);
            CHECK(vertexSourceCode.length() > 0);
            CHECK(pixelSourceCode.length() > 0);

            if (testOptions.dumpGeneratedCode)
            {
                RenderUtil::AdditiveScopedTimer dumpTimer(profileTimes.languageTimes.ioTime, "GLSL I/O time");
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << vertexSourceCode;
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << pixelSourceCode;
                file.close();
            }

            if (!testOptions.compileCode)
            {
                return false;
            }

            // Validate
            MaterialX::GlslProgramPtr program = _validator->program();
            bool validated = false;
            try
            {
                mx::GeometryHandlerPtr geomHandler = _validator->getGeometryHandler();

                bool isShader = mx::elementRequiresShading(element);
                if (isShader)
                {
                    // Set shaded element geometry
                    mx::FilePath geomPath;
                    if (!testOptions.shadedGeometry.isEmpty())
                    {
                        if (!testOptions.shadedGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.shadedGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.shadedGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/shaderball.obj");
                    }
                    if (!geomHandler->hasGeometry(geomPath))
                    {
                        geomHandler->clearGeometry();
                        geomHandler->loadGeometry(geomPath);
                    }

                    // Set shaded element lights
                    _validator->setLightHandler(_lightHandler);
                }
                else
                {
                    // Set unshaded element geometry
                    mx::FilePath geomPath;
                    if (!testOptions.unShadedGeometry.isEmpty())
                    {
                        if (!testOptions.unShadedGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.unShadedGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.unShadedGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/sphere.obj");
                    }
                    if (!geomHandler->hasGeometry(geomPath))
                    {
                        geomHandler->clearGeometry();
                        geomHandler->loadGeometry(geomPath);
                    }

                    // Clear lights for unshaded element
                    _validator->setLightHandler(nullptr);
                }

                {
                    RenderUtil::AdditiveScopedTimer compileTimer(profileTimes.languageTimes.compileTime, "GLSL compile time");
                    _validator->validateCreation(shader);
                    _validator->validateInputs();
                }

                if (testOptions.dumpUniformsAndAttributes)
                {
                    RenderUtil::AdditiveScopedTimer printTimer(profileTimes.languageTimes.ioTime, "GLSL io time");
                    log << "* Uniform:" << std::endl;
                    program->printUniforms(log);
                    log << "* Attributes:" << std::endl;
                    program->printAttributes(log);

                    log << "* Uniform UI Properties:" << std::endl;
                    const std::string& target = shadergen.getTarget();
                    const MaterialX::GlslProgram::InputMap& uniforms = program->getUniformsList();
                    for (auto uniform : uniforms)
                    {
                        const std::string& path = uniform.second->path;
                        if (path.empty())
                        {
                            continue;
                        }

                        mx::UIProperties uiProperties;
                        if (getUIProperties(path, doc, target, uiProperties) > 0)
                        {
                            log << "Program Uniform: " << uniform.first << ". Path: " << path;
                            if (!uiProperties.uiName.empty())
                                log << ". UI Name: \"" << uiProperties.uiName << "\"";
                            if (!uiProperties.uiFolder.empty())
                                log << ". UI Folder: \"" << uiProperties.uiFolder << "\"";
                            if (!uiProperties.enumeration.empty())
                            {
                                log << ". Enumeration: {";
                                for (size_t i = 0; i < uiProperties.enumeration.size(); i++)
                                    log << uiProperties.enumeration[i] << " ";
                                log << "}";
                            }
                            if (!uiProperties.enumerationValues.empty())
                            {
                                log << ". Enum Values: {";
                                for (size_t i = 0; i < uiProperties.enumerationValues.size(); i++)
                                    log << uiProperties.enumerationValues[i]->getValueString() << "; ";
                                log << "}";
                            }
                            if (uiProperties.uiMin)
                                log << ". UI Min: " << uiProperties.uiMin->getValueString();
                            if (uiProperties.uiMax)
                                log << ". UI Max: " << uiProperties.uiMax->getValueString();
                            log << std::endl;
                        }
                    }
                }

                if (testOptions.renderImages)
                {
                    {
                        RenderUtil::AdditiveScopedTimer renderTimer(profileTimes.languageTimes.renderTime, "GLSL render time");
                        _validator->getImageHandler()->setSearchPath(imageSearchPath);
                        _validator->validateRender(!isShader);
                    }

                    if (testOptions.saveImages)
                    {
                        RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.languageTimes.imageSaveTime, "GLSL image save time");
                        std::string fileName = shaderPath + "_glsl.png";
                        _validator->save(fileName, false);
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError& e)
            {
                // Always dump shader stages on error
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << shader->getSourceCode(mx::Stage::VERTEX);
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << shader->getSourceCode(mx::Stage::PIXEL);
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump files: " << shaderPath << "_ps.glsl and _vs.glsl files" << std::endl;
            }
            catch (mx::Exception& e)
            {
                log << e.what() << std::endl;
            }
            CHECK(validated);
        }
    }
    return true;
}

void GlslShaderRenderTester::getImplementationWhiteList(mx::StringSet& whiteList)
{
    whiteList =
    {
        "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
        "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue"
    };
}

TEST_CASE("Render: GLSL TestSuite", "[renderglsl]")
{
    GlslShaderRenderTester renderTester;
    renderTester.validate();
}

#endif
