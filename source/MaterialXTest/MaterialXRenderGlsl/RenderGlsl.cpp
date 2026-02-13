//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXRenderGlsl/GlslRenderer.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXTrace/Tracing.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXFormat/Util.h>

#ifdef MATERIALX_BUILD_TRACING
#include <MaterialXRenderGlsl/External/Glad/glad.h>
#include <chrono>
#endif

namespace mx = MaterialX;

#ifdef MATERIALX_BUILD_TRACING
// GPU timing utilities
namespace {

// Get current time in nanoseconds (for async event timestamps)
uint64_t getCurrentTimeNs()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

// GPU timer query helper using GL_TIME_ELAPSED
class GpuTimerQuery
{
  public:
    GpuTimerQuery()
    {
        glGenQueries(1, &_query);
    }

    ~GpuTimerQuery()
    {
        glDeleteQueries(1, &_query);
    }

    void begin()
    {
        glBeginQuery(GL_TIME_ELAPSED, _query);
    }

    void end()
    {
        glEndQuery(GL_TIME_ELAPSED);
    }

    // Returns duration in nanoseconds, blocks until result is available
    uint64_t getDurationNs()
    {
        GLuint64 elapsedTime;
        glGetQueryObjectui64v(_query, GL_QUERY_RESULT, &elapsedTime);
        return elapsedTime;
    }

  private:
    GLuint _query;
};

} // anonymous namespace
#endif // MATERIALX_BUILD_TRACING

//
// Render validation tester for the GLSL shading language
//
class GlslShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    explicit GlslShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
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

    bool saveImage(const mx::FilePath& filePath, mx::ConstImagePtr image, bool verticalFlip) const override;

    mx::GlslRendererPtr _renderer;
    mx::LightHandlerPtr _lightHandler;
};

// In addition to standard texture and shader definition libraries, additional lighting files
// are loaded in. If no files are specified in the input options, a sample
// compound light type and a set of lights in a "light rig" are loaded in to a given
// document.
void GlslShaderRenderTester::loadAdditionalLibraries(mx::DocumentPtr document,
                                                     GenShaderUtil::TestSuiteOptions& options)
{
    mx::FilePath lightDir = mx::getDefaultDataSearchPath().find("resources/Materials/TestSuite/lights");
    for (const auto& lightFile : options.lightFiles)
    {
        loadLibrary(lightDir / mx::FilePath(lightFile), document);
    }
}

// Create a light handler and populate it based on lights found in a given document
void GlslShaderRenderTester::registerLights(mx::DocumentPtr document,
                                            const GenShaderUtil::TestSuiteOptions &options,
                                            mx::GenContext& context)
{
    _lightHandler = mx::LightHandler::create();

    // Scan for lights
    if (options.enableDirectLighting)
    {
        std::vector<mx::NodePtr> lights;
        _lightHandler->findLights(document, lights);
        _lightHandler->registerLights(document, lights, context);

        // Set the list of lights on the with the generator
        _lightHandler->setLightSources(lights);
    }

    // Load environment lights.
    mx::ImagePtr envRadiance = _renderer->getImageHandler()->acquireImage(options.radianceIBLPath);
    mx::ImagePtr envIrradiance = _renderer->getImageHandler()->acquireImage(options.irradianceIBLPath);
    REQUIRE(envRadiance);
    REQUIRE(envIrradiance);

    // Apply light settings for render tests.
    _lightHandler->setEnvRadianceMap(envRadiance);
    _lightHandler->setEnvIrradianceMap(envIrradiance);
    _lightHandler->setEnvSampleCount(options.envSampleCount);
    _lightHandler->setRefractionTwoSided(true);
}

//
// Create a renderer with the appropriate image, geometry and light handlers.
// The light handler on the renderer is cleared on initialization to indicate no lighting
// is required. During code generation, if the element to validate requires lighting then
// the handler _lightHandler will be used.
//
void GlslShaderRenderTester::createRenderer(std::ostream& log)
{
    bool initialized = false;
    try
    {
        _renderer = mx::GlslRenderer::create();
        _renderer->initialize();

        // Set image handler on renderer
        mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
        mx::ImageHandlerPtr imageHandler = _renderer->createImageHandler(stbLoader);
        imageHandler->setSearchPath(mx::getDefaultDataSearchPath());
#if defined(MATERIALX_BUILD_OIIO)
        mx::OiioImageLoaderPtr oiioLoader = mx::OiioImageLoader::create();
        imageHandler->addLoader(oiioLoader);
#endif
        _renderer->setImageHandler(imageHandler);

        // Set light handler.
        _renderer->setLightHandler(nullptr);

        initialized = true;
    }
    catch (mx::ExceptionRenderError& e)
    {
        for (const auto& error : e.errorLog())
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

bool GlslShaderRenderTester::saveImage(const mx::FilePath& filePath, mx::ConstImagePtr image, bool verticalFlip) const
{
    return _renderer->getImageHandler()->saveImage(filePath, image, verticalFlip);
}

bool GlslShaderRenderTester::runRenderer(const std::string& shaderName,
                                          mx::TypedElementPtr element,
                                          mx::GenContext& context,
                                          mx::DocumentPtr doc,
                                          std::ostream& log,
                                          const GenShaderUtil::TestSuiteOptions& testOptions,
                                          RenderUtil::RenderProfileTimes& profileTimes,
                                          const mx::FileSearchPath& imageSearchPath,
                                          const std::string& outputPath,
                                          mx::ImageVec* imageVec)
{
    MX_TRACE_FUNCTION(mx::Tracing::Category::Render);
    MX_TRACE_SCOPE(mx::Tracing::Category::Material, shaderName.c_str());
    std::cout << "Validating GLSL rendering for: " << doc->getSourceUri() << std::endl;

    mx::ScopedTimer totalGLSLTime(&profileTimes.languageTimes.totalTime);

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run GLSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::FilePath outputFilePath = outputPath;
            
            // Note: mkdir will fail if the directory already exists which is ok.
            {
                mx::ScopedTimer ioDir(&profileTimes.languageTimes.ioTime);
                outputFilePath.createDirectory();
                
                // Use separate directory for reduced output
                if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
                {
                    outputFilePath = outputFilePath / mx::FilePath("reduced");
                    outputFilePath.createDirectory();
                }
            }

            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                mx::ScopedTimer transpTimer(&profileTimes.languageTimes.transparencyTime);
                options.hwTransparency = mx::isTransparentSurface(element, shadergen.getTarget());
                transpTimer.endTimer();

                mx::ScopedTimer generationTimer(&profileTimes.languageTimes.generationTime);
                MX_TRACE_SCOPE(mx::Tracing::Category::ShaderGen, "GenerateShader");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.targetColorSpaceOverride = "lin_rec709";
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
                MX_TRACE_SCOPE(mx::Tracing::Category::Render, "DumpGeneratedCode");
                mx::ScopedTimer dumpTimer(&profileTimes.languageTimes.ioTime);
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << vertexSourceCode;
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << pixelSourceCode;
                file.close();
            }

            // Validate
            MaterialX::GlslProgramPtr program = _renderer->getProgram();
            bool validated = false;
            try
            {
                // Set geometry
                mx::GeometryHandlerPtr geomHandler = _renderer->getGeometryHandler();
                mx::FilePath geomPath;
                if (!testOptions.renderGeometry.isEmpty())
                {
                    if (!testOptions.renderGeometry.isAbsolute())
                    {
                        geomPath = searchPath.find("resources/Geometry") / testOptions.renderGeometry;
                    }
                    else
                    {
                        geomPath = testOptions.renderGeometry;
                    }
                }
                else
                {
                    geomPath = searchPath.find("resources/Geometry/sphere.obj");
                }

                if (!geomHandler->hasGeometry(geomPath))
                {
                    // For test sphere and plane geometry perform a V-flip of texture coordinates.
                    const std::string baseName = geomPath.getBaseName();
                    bool texcoordVerticalFlip = baseName == "sphere.obj" || baseName == "plane.obj";
                    geomHandler->clearGeometry();
                    geomHandler->loadGeometry(geomPath, texcoordVerticalFlip);
                    for (mx::MeshPtr mesh : geomHandler->getMeshes())
                    {
                        addAdditionalTestStreams(mesh);
                    }
                }

                bool isShader = mx::elementRequiresShading(element);
                _renderer->setLightHandler(isShader ? _lightHandler : nullptr);

                {
                    MX_TRACE_SCOPE(mx::Tracing::Category::Render, "CompileShader");
                    mx::ScopedTimer compileTimer(&profileTimes.languageTimes.compileTime);
                    _renderer->createProgram(shader);
                    _renderer->validateInputs();
                }

                if (testOptions.dumpUniformsAndAttributes)
                {
                    MX_TRACE_SCOPE(mx::Tracing::Category::Render, "DumpUniformsAndAttributes");
                    mx::ScopedTimer printTimer(&profileTimes.languageTimes.ioTime);
                    log << "* Uniform:" << std::endl;
                    program->printUniforms(log);
                    log << "* Attributes:" << std::endl;
                    program->printAttributes(log);

                    log << "* Uniform UI Properties:" << std::endl;
                    const std::string& target = shadergen.getTarget();
                    const MaterialX::GlslProgram::InputMap& uniforms = program->getUniformsList();
                    for (const auto& uniform : uniforms)
                    {
                        const std::string& path = uniform.second->path;
                        if (path.empty())
                        {
                            continue;
                        }

                        mx::UIProperties uiProperties;
                        mx::ElementPtr pathElement = doc->getDescendant(path);
                        mx::InputPtr input = pathElement ? pathElement->asA<mx::Input>() : nullptr;
                        if (getUIProperties(input, target, uiProperties) > 0)
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
                            if (uiProperties.uiSoftMin)
                                log << ". UI Soft Min: " << uiProperties.uiSoftMin->getValueString();
                            if (uiProperties.uiSoftMax)
                                log << ". UI Soft Max: " << uiProperties.uiSoftMax->getValueString();
                            if (uiProperties.uiStep)
                                log << ". UI Step: " << uiProperties.uiStep->getValueString();
                            log << std::endl;
                        }
                    }
                }

                int supersampleFactor = testOptions.enableReferenceQuality ? 8 : 1;

                {
                    MX_TRACE_SCOPE(mx::Tracing::Category::Render, "RenderMaterial");
                    mx::ScopedTimer renderTimer(&profileTimes.languageTimes.renderTime);
                    _renderer->getImageHandler()->setSearchPath(imageSearchPath);
                    unsigned int width = (unsigned int) testOptions.renderSize[0] * supersampleFactor;
                    unsigned int height = (unsigned int) testOptions.renderSize[1] * supersampleFactor;
                    _renderer->setSize(width, height);

                    // Render multiple frames for statistical validity (configurable via framesPerMaterial)
                    // Frame 0 often includes driver shader compilation; analyze in Python to discard warmup
                    for (unsigned int frameIdx = 0; frameIdx < testOptions.framesPerMaterial; ++frameIdx)
                    {
#ifdef MATERIALX_BUILD_TRACING
                        // GPU timing with timer queries
                        uint64_t cpuStartNs = getCurrentTimeNs();
                        GpuTimerQuery gpuTimer;
                        gpuTimer.begin();
#endif
                        _renderer->render();

#ifdef MATERIALX_BUILD_TRACING
                        gpuTimer.end();
                        
                        // glFinish ensures GPU is done, making CPU trace scope accurate
                        glFinish();
                        
                        // Get GPU duration (query result blocks until available)
                        uint64_t gpuDurationNs = gpuTimer.getDurationNs();
                        
                        // Emit async event on GPU track showing actual GPU work duration
                        MX_TRACE_ASYNC(mx::Tracing::AsyncTrack::GPU, mx::Tracing::Category::Render, shaderName.c_str(), cpuStartNs, gpuDurationNs);
#endif
                    }
                }

                {
                    MX_TRACE_SCOPE(mx::Tracing::Category::Render, "CaptureAndSaveImage");
                    mx::ScopedTimer ioTimer(&profileTimes.languageTimes.imageSaveTime);
                    std::string fileName = shaderPath + "_glsl.png";
                    mx::ImagePtr image = _renderer->captureImage();
                    if (image)
                    {
                        if (supersampleFactor > 1)
                        {
                            image = image->applyBoxDownsample(supersampleFactor);
                        }
                        _renderer->getImageHandler()->saveImage(fileName, image, true);
                        if (imageVec)
                        {
                            imageVec->push_back(image);
                        }
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionRenderError& e)
            {
                // Always dump shader stages on error
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << shader->getSourceCode(mx::Stage::VERTEX);
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << shader->getSourceCode(mx::Stage::PIXEL);
                file.close();

                for (const auto& error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump files: " << shaderPath << "_ps.glsl and _vs.glsl files" << std::endl;
                WARN(std::string(e.what()) + " in " + shaderPath);
            }
            catch (mx::Exception& e)
            {
                log << e.what() << std::endl;
                WARN(std::string(e.what()) + " in " + shaderPath);
            }
            CHECK(validated);
        }
    }
    return true;
}

TEST_CASE("Render: GLSL TestSuite", "[renderglsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");

    GlslShaderRenderTester renderTester(mx::GlslShaderGenerator::create());
    renderTester.validate(optionsFilePath);
}
