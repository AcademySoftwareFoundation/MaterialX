//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

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

    bool canBake() const override
    {
        return true;
    }

    void runBake(mx::DocumentPtr doc, const mx::FileSearchPath& imageSearchPath, const mx::FilePath& outputFilename,
                 const GenShaderUtil::TestSuiteOptions::BakeSetting& bakeOptions, std::ostream& log) override;

    mx::GlslRendererPtr _renderer;
    mx::LightHandlerPtr _lightHandler;
};

// In addition to standard texture and shader definition libraries, additional lighting files
// are loaded in. If no files are specifed in the input options, a sample
// compound light type and a set of lights in a "light rig" are loaded in to a given
// document.
void GlslShaderRenderTester::loadAdditionalLibraries(mx::DocumentPtr document,
                                                     GenShaderUtil::TestSuiteOptions& options)
{
    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/lights");
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
    std::vector<mx::NodePtr> lights;
    _lightHandler->findLights(document, lights);
    _lightHandler->registerLights(document, lights, context);

    // Set the list of lights on the with the generator
    _lightHandler->setLightSources(lights);

    // Load environment lights.
    mx::ImagePtr envRadiance = _renderer->getImageHandler()->acquireImage(options.radianceIBLPath);
    mx::ImagePtr envIrradiance = _renderer->getImageHandler()->acquireImage(options.irradianceIBLPath);
    REQUIRE(envRadiance);
    REQUIRE(envIrradiance);

    // Apply light settings for render tests.
    _lightHandler->setEnvRadianceMap(envRadiance);
    _lightHandler->setEnvIrradianceMap(envIrradiance);
    _lightHandler->setEnvSampleCount(1024);
    _lightHandler->setRefractionEnv(false);
    _lightHandler->setRefractionColor(_renderer->getScreenColor());
}

//
// Create a renderer with the apporpraite image, geometry and light handlers.
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
        mx::ImageHandlerPtr imageHandler = mx::GLTextureHandler::create(stbLoader);
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

// If these streams don't exist add them for testing purposes
//
void addAdditionalTestStreams(mx::MeshPtr mesh)
{
    size_t vertexCount = mesh->getVertexCount();
    if (vertexCount < 1)
    {
        return;
    }

    const std::string TEXCOORD_STREAM0_NAME("i_" + mx::MeshStream::TEXCOORD_ATTRIBUTE + "_0");
    mx::MeshStreamPtr texCoordStream1 = mesh->getStream(TEXCOORD_STREAM0_NAME);
    mx::MeshFloatBuffer uv = texCoordStream1->getData();

    const std::string TEXCOORD_STREAM1_NAME("i_" + mx::MeshStream::TEXCOORD_ATTRIBUTE + "_1");
    mx::MeshFloatBuffer* texCoordData2 = nullptr;
    if (!mesh->getStream(TEXCOORD_STREAM1_NAME))
    {
        mx::MeshStreamPtr texCoordStream2 = mx::MeshStream::create(TEXCOORD_STREAM1_NAME, mx::MeshStream::TEXCOORD_ATTRIBUTE, 1);
        texCoordStream2->setStride(2);
        texCoordData2 = &(texCoordStream2->getData());
        texCoordData2->resize(vertexCount * 2);
        mesh->addStream(texCoordStream2);
    }

    const std::string COLOR_STREAM0_NAME("i_" + mx::MeshStream::COLOR_ATTRIBUTE + "_0");
    mx::MeshFloatBuffer* colorData1 = nullptr;
    if (!mesh->getStream(COLOR_STREAM0_NAME))
    {
        mx::MeshStreamPtr colorStream1 = mx::MeshStream::create(COLOR_STREAM0_NAME, mx::MeshStream::COLOR_ATTRIBUTE, 0);
        colorData1 = &(colorStream1->getData());
        colorStream1->setStride(4);
        colorData1->resize(vertexCount * 4);
        mesh->addStream(colorStream1);
    }

    const std::string COLOR_STREAM1_NAME("i_" + mx::MeshStream::COLOR_ATTRIBUTE + "_1");
    mx::MeshFloatBuffer* colorData2 = nullptr;
    if (!mesh->getStream(COLOR_STREAM1_NAME))
    {
        mx::MeshStreamPtr colorStream2 = mx::MeshStream::create(COLOR_STREAM1_NAME, mx::MeshStream::COLOR_ATTRIBUTE, 1);
        colorData2 = &(colorStream2->getData());
        colorStream2->setStride(4);
        colorData2->resize(vertexCount * 4);
        mesh->addStream(colorStream2);
    }

    const std::string GEOM_INT_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_integer");
    int32_t* geomIntData = nullptr;
    if (!mesh->getStream(GEOM_INT_STREAM_NAME))
    {
        mx::MeshStreamPtr geomIntStream = mx::MeshStream::create(GEOM_INT_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 0);
        geomIntStream->setStride(1);
        geomIntStream->getData().resize(vertexCount);
        mesh->addStream(geomIntStream);
        // Float and int32 have same size.
        geomIntData = reinterpret_cast<int32_t*>(geomIntStream->getData().data());
    }

    const std::string GEOM_FLOAT_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_float");
    mx::MeshFloatBuffer* geomFloatData = nullptr;
    if (!mesh->getStream(GEOM_FLOAT_STREAM_NAME))
    {
        mx::MeshStreamPtr geomFloatStream = mx::MeshStream::create(GEOM_FLOAT_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomFloatData = &(geomFloatStream->getData());
        geomFloatStream->setStride(1);
        geomFloatData->resize(vertexCount);
        mesh->addStream(geomFloatStream);
    }

    const std::string GEOM_VECTOR2_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_vector2");
    mx::MeshFloatBuffer* geomVector2Data = nullptr;
    if (!mesh->getStream(GEOM_VECTOR2_STREAM_NAME))
    {
        mx::MeshStreamPtr geomVector2Stream = mx::MeshStream::create(GEOM_VECTOR2_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomVector2Data = &(geomVector2Stream->getData());
        geomVector2Stream->setStride(2);
        geomVector2Data->resize(vertexCount * 2);
        mesh->addStream(geomVector2Stream);
    }

    const std::string GEOM_VECTOR3_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_vector3");
    mx::MeshFloatBuffer* geomVector3Data = nullptr;
    if (!mesh->getStream(GEOM_VECTOR3_STREAM_NAME))
    {
        mx::MeshStreamPtr geomVector3Stream = mx::MeshStream::create(GEOM_VECTOR3_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomVector3Data = &(geomVector3Stream->getData());
        geomVector3Stream->setStride(3);
        geomVector3Data->resize(vertexCount * 3);
        mesh->addStream(geomVector3Stream);
    }

    const std::string GEOM_VECTOR4_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_vector4");
    mx::MeshFloatBuffer* geomVector4Data = nullptr;
    if (!mesh->getStream(GEOM_VECTOR4_STREAM_NAME))
    {
        mx::MeshStreamPtr geomVector4Stream = mx::MeshStream::create(GEOM_VECTOR4_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomVector4Data = &(geomVector4Stream->getData());
        geomVector4Stream->setStride(4);
        geomVector4Data->resize(vertexCount * 4);
        mesh->addStream(geomVector4Stream);
    }

    const std::string GEOM_COLOR2_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_color2");
    mx::MeshFloatBuffer* geomColor2Data = nullptr;
    if (!mesh->getStream(GEOM_COLOR2_STREAM_NAME))
    {
        mx::MeshStreamPtr geomColor2Stream = mx::MeshStream::create(GEOM_COLOR2_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomColor2Data = &(geomColor2Stream->getData());
        geomColor2Stream->setStride(2);
        geomColor2Data->resize(vertexCount * 2);
        mesh->addStream(geomColor2Stream);
    }

    const std::string GEOM_COLOR3_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_color3");
    mx::MeshFloatBuffer* geomColor3Data = nullptr;
    if (!mesh->getStream(GEOM_COLOR3_STREAM_NAME))
    {
        mx::MeshStreamPtr geomColor3Stream = mx::MeshStream::create(GEOM_COLOR3_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomColor3Data = &(geomColor3Stream->getData());
        geomColor3Stream->setStride(3);
        geomColor3Data->resize(vertexCount * 3);
        mesh->addStream(geomColor3Stream);
    }

    const std::string GEOM_COLOR4_STREAM_NAME("i_" + mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE + "_geompropvalue_color4");
    mx::MeshFloatBuffer* geomColor4Data = nullptr;
    if (!mesh->getStream(GEOM_COLOR4_STREAM_NAME))
    {
        mx::MeshStreamPtr geomColor4Stream = mx::MeshStream::create(GEOM_COLOR4_STREAM_NAME, mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE, 1);
        geomColor4Data = &(geomColor4Stream->getData());
        geomColor4Stream->setStride(4);
        geomColor4Data->resize(vertexCount * 4);
        mesh->addStream(geomColor4Stream);
    }

    auto sineData = [](float uv, float freq){
        const float PI = std::acos(-1.0f);
        float angle = uv * 2 * PI * freq;
        return std::sin(angle) / 2.0f + 1.0f;
    };
    if (!uv.empty())
    {
        for (size_t i = 0; i < vertexCount; i++)
        {
            const size_t i2 = 2 * i;
            const size_t i21 = i2 + 1;
            const size_t i3 = 3 * i;
            const size_t i4 = 4 * i;

            // Fake second set of texture coordinates
            if (texCoordData2)
            {
                (*texCoordData2)[i2] = uv[i21];
                (*texCoordData2)[i21] = uv[i2];
            }
            if (colorData1)
            {
                // Fake some colors
                (*colorData1)[i4] = uv[i2];
                (*colorData1)[i4 + 1] = uv[i21];
                (*colorData1)[i4 + 2] = 1.0f;
                (*colorData1)[i4 + 3] = 1.0f;
            }
            if (colorData2)
            {
                (*colorData2)[i4] = 1.0f;
                (*colorData2)[i4 + 1] = uv[i2];
                (*colorData2)[i4 + 2] = uv[i21];
                (*colorData2)[i4 + 3] = 1.0f;
            }
            if (geomIntData)
            {
                geomIntData[i] = static_cast<int32_t>(uv[i21] * 5);
            }
            if (geomFloatData)
            {
                (*geomFloatData)[i] = sineData(uv[i21], 12.0f);
            }
            if (geomVector2Data)
            {
                (*geomVector2Data)[i2] = sineData(uv[i21], 6.0f);
                (*geomVector2Data)[i21] = 0.0f;
            }
            if (geomVector3Data)
            {
                (*geomVector3Data)[i3] = 0.0f;
                (*geomVector3Data)[i3 + 1] = sineData(uv[i21], 8.0f);
                (*geomVector3Data)[i3 + 2] = 0.0f;
            }
            if (geomVector4Data)
            {
                (*geomVector4Data)[i4] = 0.0f;
                (*geomVector4Data)[i4 + 1] = 0.0f;
                (*geomVector4Data)[i4 + 2] = sineData(uv[i21], 10.0f);
                (*geomVector4Data)[i4 + 3] = 1.0f;
            }

            if (geomColor2Data)
            {
                (*geomColor2Data)[i2] = sineData(uv[i2], 10.0f);
                (*geomColor2Data)[i21] = 0.0f;
            }
            if (geomColor3Data)
            {
                (*geomColor3Data)[i3] = 0.0f;
                (*geomColor3Data)[i3 + 1] = sineData(uv[i2], 8.0f);
                (*geomColor3Data)[i3 + 2] = 0.0f;
            }
            if (geomColor4Data)
            {
                (*geomColor4Data)[i4] = 0.0f;
                (*geomColor4Data)[i4 + 1] = 0.0f;
                (*geomColor4Data)[i4 + 2] = sineData(uv[i2], 6.0f);
                (*geomColor4Data)[i4 + 3] = 1.0f;
            }
        }
    }
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
    std::cout << "Validating GLSL rendering for: " << doc->getSourceUri() << std::endl;

    mx::ScopedTimer totalGLSLTime(&profileTimes.languageTimes.totalTime);

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
                mx::ScopedTimer ioDir(&profileTimes.languageTimes.ioTime);
                outputFilePath.createDirectory();
            }

            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                mx::ScopedTimer transpTimer(&profileTimes.languageTimes.transparencyTime);
                options.hwTransparency = mx::isTransparentSurface(element, shadergen.getTarget());
                transpTimer.endTimer();

                mx::ScopedTimer generationTimer(&profileTimes.languageTimes.generationTime);
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
                mx::ScopedTimer dumpTimer(&profileTimes.languageTimes.ioTime);
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
            MaterialX::GlslProgramPtr program = _renderer->getProgram();
            bool validated = false;
            try
            {
                // Set geometry
                mx::GeometryHandlerPtr geomHandler = _renderer->getGeometryHandler();
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
                    geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/sphere.obj");
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
                    mx::ScopedTimer compileTimer(&profileTimes.languageTimes.compileTime);
                    _renderer->createProgram(shader);
                    _renderer->validateInputs();
                }

                if (testOptions.dumpUniformsAndAttributes)
                {
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

                if (testOptions.renderImages)
                {
                    {
                        mx::ScopedTimer renderTimer(&profileTimes.languageTimes.renderTime);
                        _renderer->getImageHandler()->setSearchPath(imageSearchPath);
                        _renderer->setSize(static_cast<unsigned int>(testOptions.renderSize[0]), static_cast<unsigned int>(testOptions.renderSize[1]));
                        _renderer->render();
                    }

                    if (testOptions.saveImages)
                    {
                        mx::ScopedTimer ioTimer(&profileTimes.languageTimes.imageSaveTime);
                        std::string fileName = shaderPath + "_glsl.png";
                        mx::ImagePtr image = _renderer->captureImage();
                        if (image)
                        {
                            _renderer->getImageHandler()->saveImage(fileName, image, true);
                            if (imageVec)
                            {
                                imageVec->push_back(image);
                            }
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

void GlslShaderRenderTester::runBake(mx::DocumentPtr doc, const mx::FileSearchPath& imageSearchPath, const mx::FilePath& outputFileName,
                                     const GenShaderUtil::TestSuiteOptions::BakeSetting& bakeOptions, std::ostream& log)
{
    mx::ImageVec imageVec = _renderer->getImageHandler()->getReferencedImages(doc);
    auto maxImageSize = mx::getMaxDimensions(imageVec);
    const unsigned bakeWidth = std::max(bakeOptions.resolution, maxImageSize.first);
    const unsigned bakeHeight = std::max(bakeOptions.resolution, maxImageSize.second);

    mx::Image::BaseType baseType = bakeOptions.hdr ? mx::Image::BaseType::FLOAT : mx::Image::BaseType::UINT8;
    mx::TextureBakerPtr baker = mx::TextureBaker::create(bakeWidth, bakeHeight, baseType);
    baker->setupUnitSystem(doc);
    baker->setImageHandler(_renderer->getImageHandler());
    baker->setOptimizeConstants(true);
    baker->setHashImageNames(true);
    baker->setTextureSpaceMin(bakeOptions.uvmin);
    baker->setTextureSpaceMax(bakeOptions.uvmax);

    try
    {
        baker->setOutputStream(&log);
        baker->bakeAllMaterials(doc, imageSearchPath, outputFileName);
    }
    catch (mx::Exception& e)
    {
        const mx::FilePath& sourceUri = doc->getSourceUri();
        log << sourceUri.asString() + " failed baking process: " + e.what() << std::endl;
    }
}

TEST_CASE("Render: GLSL TestSuite", "[renderglsl]")
{
    GlslShaderRenderTester renderTester(mx::GlslShaderGenerator::create());

    mx::FilePath optionsFilePath("resources/Materials/TestSuite/_options.mtlx");

    renderTester.validate(optionsFilePath);
}
