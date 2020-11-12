//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(__linux__)
#define NonePrev None
#undef None
#endif
#include <MaterialXTest/Catch/catch.hpp>
#if defined(__linux__)
#define None NonePrev
#endif
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXCore/Types.h>

#include <MaterialXFormat/Util.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXRenderGlsl/GlslRenderer.h>
#include <MaterialXRenderGlsl/TextureBaker.h>

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

    void transformUVs(const mx::MeshList& meshes, const mx::Matrix44& matrixTransform) const;

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
                 unsigned int bakeWidth, unsigned int bakeHeight, bool bakeHdr, std::ostream& log) override;

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
    mx::ImagePtr envRadiance = _renderer->getImageHandler()->acquireImage(options.radianceIBLPath, true);
    mx::ImagePtr envIrradiance = _renderer->getImageHandler()->acquireImage(options.irradianceIBLPath, true);
    REQUIRE(envRadiance);
    REQUIRE(envIrradiance);
    _lightHandler->setEnvRadianceMap(envRadiance);
    _lightHandler->setEnvIrradianceMap(envIrradiance);
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
    catch (mx::ExceptionShaderRenderError& e)
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

    if (!uv.empty())
    {
        for (size_t i = 0; i < vertexCount; i++)
        {
            const size_t i2 = 2 * i;
            const size_t i21 = i2 + 1;
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
        }
    }
}

void GlslShaderRenderTester::transformUVs(const mx::MeshList& meshes, const mx::Matrix44& matrixTransform) const
{
    for(mx::MeshPtr mesh : meshes)
    {
        // Transform texture coordinates.
        mx::MeshStreamPtr uvStream = mesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);
        uvStream->transform(matrixTransform);

        // Regenerate tangents
        mx::MeshStreamPtr tangentStream = mesh->getStream(mx::MeshStream::TANGENT_ATTRIBUTE, 0);
        if (tangentStream)
        {
            mesh->removeStream(tangentStream);
            tangentStream = mesh->generateTangents(mesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0),
                                                   mesh->getStream(mx::MeshStream::NORMAL_ATTRIBUTE, 0),
                                                   uvStream);
            if (tangentStream)
            {
                mesh->addStream(tangentStream);
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
                contextOptions.targetColorSpaceOverride = "lin_rec709";
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
            MaterialX::GlslProgramPtr program = _renderer->getProgram();
            bool validated = false;
            try
            {
                mx::GeometryHandlerPtr geomHandler = _renderer->getGeometryHandler();

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
                        const mx::MeshList& meshes = geomHandler->getMeshes();
                        if (!meshes.empty())
                        {
                            addAdditionalTestStreams(meshes[0]);
                            transformUVs(meshes, testOptions.transformUVs);
                        }
                    }

                    // Set shaded element lights
                    _renderer->setLightHandler(_lightHandler);
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
                        const mx::MeshList& meshes = geomHandler->getMeshes();
                        if (!meshes.empty())
                        {
                            addAdditionalTestStreams(meshes[0]);
                            transformUVs(meshes, testOptions.transformUVs);
                        }
                    }

                    // Clear lights for unshaded element
                    _renderer->setLightHandler(nullptr);
                }

                {
                    RenderUtil::AdditiveScopedTimer compileTimer(profileTimes.languageTimes.compileTime, "GLSL compile time");
                    _renderer->createProgram(shader);
                    _renderer->validateInputs();
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
                    for (const auto& uniform : uniforms)
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
                        RenderUtil::AdditiveScopedTimer renderTimer(profileTimes.languageTimes.renderTime, "GLSL render time");
                        _renderer->getImageHandler()->setSearchPath(imageSearchPath);
                        _renderer->setSize(static_cast<unsigned int>(testOptions.renderSize[0]), static_cast<unsigned int>(testOptions.renderSize[1]));
                        _renderer->render();
                    }

                    if (testOptions.saveImages)
                    {
                        RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.languageTimes.imageSaveTime, "GLSL image save time");
                        std::string fileName = shaderPath + "_glsl.png";
                        mx::ImagePtr image = _renderer->captureImage();
                        if (image)
                        {
                            _renderer->saveImage(fileName, image, true);
                            if (imageVec)
                            {
                                imageVec->push_back(image);
                            }
                        }
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderRenderError& e)
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

void  GlslShaderRenderTester::runBake(mx::DocumentPtr doc, const mx::FileSearchPath& imageSearchPath, const mx::FilePath& outputFileName,
                                           unsigned int bakeWidth, unsigned int bakeHeight, bool bakeHdr, std::ostream& log)
{
    if (bakeWidth < 2)
    {
        bakeWidth = 2;
    }
    if (bakeHeight < 2)
    {
        bakeHeight = 2;
    }
    mx::Image::BaseType baseType = bakeHdr ? mx::Image::BaseType::FLOAT : mx::Image::BaseType::UINT8;
    mx::TextureBakerPtr baker = mx::TextureBaker::create(bakeWidth, bakeHeight, baseType);
    baker->setupUnitSystem(doc);
    baker->setTargetUnitSpace("meter");
    baker->setImageHandler(_renderer->getImageHandler());

    try
    {
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
