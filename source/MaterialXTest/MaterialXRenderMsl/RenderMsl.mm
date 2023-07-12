//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef __APPLE__

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXGenMsl/MslShaderGenerator.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXRenderMsl/MslRenderer.h>
#include <MaterialXRenderMsl/TextureBaker.h>

#include <cmath>

#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

//
// Render validation tester for the Metal shading language
//
class MslShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    explicit MslShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
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

    mx::MslRendererPtr _renderer;
    mx::LightHandlerPtr _lightHandler;
    id<MTLDevice> _device;
};

// In addition to standard texture and shader definition libraries, additional lighting files
// are loaded in. If no files are specifed in the input options, a sample
// compound light type and a set of lights in a "light rig" are loaded in to a given
// document.
void MslShaderRenderTester::loadAdditionalLibraries(mx::DocumentPtr document,
                                                     GenShaderUtil::TestSuiteOptions& options)
{
    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/lights");
    for (const auto& lightFile : options.lightFiles)
    {
        loadLibrary(lightDir / mx::FilePath(lightFile), document);
    }
}

// Create a light handler and populate it based on lights found in a given document
void MslShaderRenderTester::registerLights(mx::DocumentPtr document,
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
    _lightHandler->setEnvSampleCount(options.enableReferenceQuality ? 4096 : 1024);
}

//
// Create a renderer with the apporpraite image, geometry and light handlers.
// The light handler on the renderer is cleared on initialization to indicate no lighting
// is required. During code generation, if the element to validate requires lighting then
// the handler _lightHandler will be used.
//
void MslShaderRenderTester::createRenderer(std::ostream& log)
{
    bool initialized = false;
    try
    {
        _renderer = mx::MslRenderer::create();
        _renderer->initialize();
        
        _device = _renderer->getMetalDevice();

        // Set image handler on renderer
        mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
        mx::ImageHandlerPtr imageHandler =
            _renderer->createImageHandler(stbLoader);
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

bool MslShaderRenderTester::saveImage(const mx::FilePath& filePath, mx::ConstImagePtr image, bool verticalFlip) const
{
    return _renderer->getImageHandler()->saveImage(filePath, image, verticalFlip);
}

bool MslShaderRenderTester::runRenderer(const std::string& shaderName,
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
    std::cout << "Validating MSL rendering for: " << doc->getSourceUri() << std::endl;

    mx::ScopedTimer totalMSLTime(&profileTimes.languageTimes.totalTime);

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
        log << "------------ Run MSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

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
                file.open(shaderPath + "_vs.metal");
                file << vertexSourceCode;
                file.close();
                file.open(shaderPath + "_ps.metal");
                file << pixelSourceCode;
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
                // Set geometry
                mx::GeometryHandlerPtr geomHandler = _renderer->getGeometryHandler();
                mx::FilePath geomPath;
                if (!testOptions.renderGeometry.isEmpty())
                {
                    if (!testOptions.renderGeometry.isAbsolute())
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.renderGeometry;
                    }
                    else
                    {
                        geomPath = testOptions.renderGeometry;
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
                    MaterialX::MslProgramPtr program = _renderer->getProgram();
                    mx::ScopedTimer printTimer(&profileTimes.languageTimes.ioTime);
                    log << "* Uniform:" << std::endl;
                    program->printUniforms(log);
                    log << "* Attributes:" << std::endl;
                    program->printAttributes(log);

                    log << "* Uniform UI Properties:" << std::endl;
                    const std::string& target = shadergen.getTarget();
                    const MaterialX::MslProgram::InputMap& uniforms = program->getUniformsList();
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
                        std::string fileName = shaderPath + "_msl.png";
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
                file.open(shaderPath + "_vs.metal");
                file << shader->getSourceCode(mx::Stage::VERTEX);
                file.close();
                file.open(shaderPath + "_ps.metal");
                file << shader->getSourceCode(mx::Stage::PIXEL);
                file.close();

                for (const auto& error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump files: " << shaderPath << "_ps.metal and _vs.metal files" << std::endl;
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

void MslShaderRenderTester::runBake(mx::DocumentPtr doc, const mx::FileSearchPath& imageSearchPath, const mx::FilePath& outputFileName,
                                     const GenShaderUtil::TestSuiteOptions::BakeSetting& bakeOptions, std::ostream& log)
{
    mx::ImageVec imageVec = _renderer->getImageHandler()->getReferencedImages(doc);
    auto maxImageSize = mx::getMaxDimensions(imageVec);
    const unsigned bakeWidth = std::max(bakeOptions.resolution, maxImageSize.first);
    const unsigned bakeHeight = std::max(bakeOptions.resolution, maxImageSize.second);

    mx::Image::BaseType baseType = bakeOptions.hdr ? mx::Image::BaseType::FLOAT : mx::Image::BaseType::UINT8;
    mx::TextureBakerPtr baker = mx::TextureBakerMsl::create(bakeWidth, bakeHeight, baseType);
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

TEST_CASE("Render: MSL TestSuite", "[rendermsl]")
{
    MslShaderRenderTester renderTester(mx::MslShaderGenerator::create());

    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    const mx::FilePath testRootPath3 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    testRootPaths.push_back(testRootPath2);
    testRootPaths.push_back(testRootPath3);

    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");

    renderTester.validate(optionsFilePath);
}

#endif
