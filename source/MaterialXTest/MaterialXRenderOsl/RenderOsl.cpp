//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXRenderOsl/OslRenderer.h>

#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslNetworkShaderGenerator.h>

#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

namespace
{

//
// Define local overrides for the tangent frame in shader generation, aligning conventions
// between MaterialXRender and testrender.
//

class TangentOsl : public mx::ShaderNodeImpl
{
  public:
    static mx::ShaderNodeImplPtr create()
    {
        return std::make_shared<TangentOsl>();
    }

    void emitFunctionCall(const  mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

        DEFINE_SHADER_STAGE(stage, mx::Stage::PIXEL)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = normalize(dPdu)", stage);
            shadergen.emitLineEnd(stage);
        }
    }
};

class BitangentOsl : public mx::ShaderNodeImpl
{
  public:
    static mx::ShaderNodeImplPtr create()
    {
        return std::make_shared<BitangentOsl>();
    }

    void emitFunctionCall(const  mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

        DEFINE_SHADER_STAGE(stage, mx::Stage::PIXEL)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = normalize(cross(N, normalize(dPdu)))", stage);
            shadergen.emitLineEnd(stage);
        }
    }
};

} // anonymous namespace

class OslShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    explicit OslShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator, bool useOslCmdStr) :
        RenderUtil::ShaderRenderTester(shaderGenerator)
    {
        // Preprocess to resolve to absolute image file names
        // and all non-POSIX separators must be converted to POSIX ones (this only affects running on Windows)
        _resolveImageFilenames = true;
        _customFilenameResolver = mx::StringResolver::create();
        _customFilenameResolver->setFilenameSubstitution("\\\\", "/");
        _customFilenameResolver->setFilenameSubstitution("\\", "/");
        _useOslCmdStr = useOslCmdStr;
    }

  protected:
    void setTargetGenerationOptions(mx::GenOptions& options) override
    {
        // Flip file texture lookups vertically, compensating for the top-left
        // image origin of OSL texture lookups.
        options.fileTextureVerticalFlip = true;

        // Generate a wrapper shader connecting the output to Ci, as required
        // for rendering with testrender.
        options.oslConnectCiWrapper = true;
    }

    void createRenderer(std::ostream& log) override;

    unsigned int getRenderWorkerCount(const GenShaderUtil::TestSuiteOptions& options) const override
    {
        return static_cast<unsigned int>(options.oslRenderWorkers);
    }

    RenderUtil::RenderProfileResult runRenderer(
        const RenderUtil::RenderSession& session,
        const RenderUtil::RenderItem& item,
        mx::GenContext& context) override;

    void addSkipFiles() override
    {
        if (_useOslCmdStr)
        {
            _skipFiles.insert("filename_cm_test.mtlx");
            _skipFiles.insert("shader_ops.mtlx");
            _skipFiles.insert("chiang_hair_surfaceshader.mtlx");
            _skipFiles.insert("network_surfaceshader.mtlx");
            _skipFiles.insert("sheen.mtlx");
            _skipFiles.insert("toon_shade.mtlx");
            _skipFiles.insert("bsdf_graph.mtlx");
            _skipFiles.insert("varying_ior.mtlx");
            _skipFiles.insert("vertical_layering.mtlx");
            _skipFiles.insert("mix_bsdf.mtlx");
        }

        RenderUtil::ShaderRenderTester::addSkipFiles();
    }

    bool saveImage(const mx::FilePath& filePath, mx::ConstImagePtr image, bool /*verticalFlip*/) const override
    {
        return _renderer->getImageHandler()->saveImage(filePath, image, false);
    }

    mx::ImageLoaderPtr _imageLoader;
    mx::OslRendererPtr _renderer;
    bool _useOslCmdStr;
};

// Renderer setup
void OslShaderRenderTester::createRenderer(std::ostream& log)
{
    bool initialized = false;

    _renderer = mx::OslRenderer::create();
    _imageLoader = mx::StbImageLoader::create();

    // Set up additional utilities required to run OSL testing including
    // oslc and testrender paths and OSL include path
    //
    const std::string oslcExecutable(MATERIALX_OSL_BINARY_OSLC);
    _renderer->setOslCompilerExecutable(oslcExecutable);
    const std::string testRenderExecutable(MATERIALX_OSL_BINARY_TESTRENDER);
    _renderer->setOslTestRenderExecutable(testRenderExecutable);
    mx::FilePath oslStandardIncludePath = mx::FilePath(MATERIALX_OSL_INCLUDE_PATH);
    if (!oslStandardIncludePath.isEmpty())
    {
        _renderer->setOslIncludePath(mx::FileSearchPath(oslStandardIncludePath));
    }

    try
    {
        _renderer->initialize();

        mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
        mx::ImageHandlerPtr imageHandler = mx::ImageHandler::create(stbLoader);
        imageHandler->setSearchPath(mx::getDefaultDataSearchPath());
#if defined(MATERIALX_BUILD_OIIO)
        mx::OiioImageLoaderPtr oiioLoader = mx::OiioImageLoader::create();
        imageHandler->addLoader(oiioLoader);
#endif
        _renderer->setImageHandler(imageHandler);
        _renderer->setLightHandler(nullptr);
        // Pre-compile some required shaders for testrender
        if (!oslcExecutable.empty() && !testRenderExecutable.empty())
        {
            mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
            mx::FilePath shaderPath = searchPath.find("resources/Utilities/");
            _renderer->setOslOutputFilePath(shaderPath);

            const std::string OSL_EXTENSION("osl");
            for (const mx::FilePath& filename : shaderPath.getFilesInDirectory(OSL_EXTENSION))
            {
                _renderer->compileOSL(shaderPath / filename);
            }

            // Set the search path for these compiled shaders.
            _renderer->setOslUtilityOSOPath(shaderPath);
        }
        initialized = true;
    }
    catch (mx::ExceptionRenderError& e)
    {
        for (const auto& error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);
}

// Renderer execution
RenderUtil::RenderProfileResult OslShaderRenderTester::runRenderer(
    const RenderUtil::RenderSession& session,
    const RenderUtil::RenderItem& item,
    mx::GenContext& context)
{
    RenderUtil::RenderProfileResult result;
    const std::string& shaderName = item.shaderName;
    mx::DocumentPtr doc = item.document;
    mx::TypedElementPtr element = item.element;
    const GenShaderUtil::TestSuiteOptions& testOptions = session.testOptions;
    std::ostream& log = session.log;

    std::cout << "Validating OSL rendering for: " << doc->getSourceUri() << std::endl;

    mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run OSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (const auto& options : optionsList)
        {
            result.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                mx::ScopedTimer genTimer(&result.languageTimes.generationTime);
                context.getOptions() = options;

                // Apply local overrides for shader generation.
                shadergen.registerImplementation("IM_tangent_vector3_" + mx::OslShaderGenerator::TARGET, TangentOsl::create);
                shadergen.registerImplementation("IM_bitangent_vector3_" + mx::OslShaderGenerator::TARGET, BitangentOsl::create);

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
                result.success = false;
                return result;
            }
            std::string source = shader->getSourceCode();
            CHECK(!source.empty());
            if (source.empty())
            {
                result.success = false;
                continue;
            }

            auto& outputs = shader->getStage(mx::Stage::PIXEL).getOutputBlock(mx::OSL::OUTPUTS);
            CHECK(!outputs.empty());
            if (outputs.empty())
            {
                result.success = false;
                continue;
            }
            auto output = outputs[0];
            const auto& syntax = shadergen.getSyntax().getTypeSyntax(output->getType());
            std::string outputName = output->getVariable();
            std::string outputType = syntax.getTypeAlias().empty() ? syntax.getName() : syntax.getTypeAlias();
            auto outputFilePath = item.outputPath;
            bool reduced = options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED;
            if (reduced)
                outputFilePath = outputFilePath / "reduced";
            std::string shaderPath = (outputFilePath / shaderName).asString();
            bool network = _useOslCmdStr;
            auto osoPathAttr = shader->getAttribute("osoPath");
            mx::FilePath dataOsoPath = osoPathAttr ? osoPathAttr->getValueString() : mx::EMPTY_STRING;
            auto utilityPath = mx::getDefaultDataSearchPath().find("resources/Utilities");
            auto settings = testOptions;
            // testrender supports OBJ geometry; retain the sphere fallback
            // for other formats and pass the resolved path to each private renderer.
            auto renderGeometry = settings.renderGeometry;
            if (renderGeometry.getExtension() != "obj")
                renderGeometry = mx::FilePath();
            auto geomPath = RenderUtil::findRenderGeometry(renderGeometry, mx::getDefaultDataSearchPath());
            auto threadCount = RenderUtil::oslThreadCount(settings.oslRenderWorkers,
                                                         settings.oslThreadsPerWorker,
                                                         RenderUtil::availableCpuCount());
            std::string label = doc->getSourceUri() + ": " + element->getNamePath() +
                                (reduced ? " [reduced]" : " [complete]");
            try
            {
                session.jobs.submit(label, shaderPath,
                    [source, shaderName, shaderPath, outputFilePath, outputName, outputType,
                     network, dataOsoPath, utilityPath, settings, threadCount, geomPath](RenderUtil::RenderJobResult& job) {
                        outputFilePath.createDirectory(true);
                        auto renderer = mx::OslRenderer::create();
                        renderer->setOslCompilerExecutable(MATERIALX_OSL_BINARY_OSLC);
                        renderer->setOslTestRenderExecutable(MATERIALX_OSL_BINARY_TESTRENDER);
                        renderer->setOslIncludePath(mx::FileSearchPath(MATERIALX_OSL_INCLUDE_PATH));
                        renderer->initialize();
                        renderer->setOslUtilityOSOPath(utilityPath);
                        renderer->setOslOutputFilePath(outputFilePath);
                        renderer->setOslShaderName(shaderName);
                        renderer->setOslShaderOutput(outputName, outputType);
                        renderer->setSize(static_cast<unsigned int>(settings.renderSize[0]),
                                          static_cast<unsigned int>(settings.renderSize[1]));
                        renderer->setAaLit(settings.enableReferenceQuality ? 16 : 2);
                        renderer->setAaUnlit(settings.enableReferenceQuality ? 2 : 1);
                        renderer->setThreadCount(threadCount);
                        renderer->setRenderGeometry(geomPath);
                        renderer->setEnvShaderParameterOverrides({
                            "string envmap_filename \"" + settings.radianceIBLPath.asString() + "\";\n" });
                        renderer->setOSLCmdStr(source);
                        renderer->useOslCommandString(network);
                        renderer->setDataLibraryOSOPath(dataOsoPath);
                        renderer->setOslTestRenderSceneTemplateFile(utilityPath /
                            (network ? "scene_template_oslcmd.xml" : "scene_template.xml"));
                        job.imagePath = shaderPath + (network ? "_oslcmd.png" : "_osl.png");
                        if (network && settings.dumpGeneratedCode)
                        {
                            std::ofstream file(shaderPath + ".oslcmd");
                            file << source;
                            file.close();
                            if (!file)
                                throw mx::ExceptionRenderError("Cannot write OSL command source: " + shaderPath);
                        }
                        if (!network)
                        {
                            mx::ScopedTimer compileTimer(&job.compileTime);
                            renderer->createProgram(mx::ShaderRenderer::StageMap { { mx::Stage::PIXEL, source } });
                        }
                        mx::ScopedTimer renderTimer(&job.renderTime);
                        renderer->render();
                    });
            }
            catch (std::exception& e)
            {
                log << e.what() << std::endl;
                FAIL_CHECK(e.what());
                result.success = false;
            }
        }
    }

    result.languageTimes.totalTime = result.languageTimes.generationTime + result.languageTimes.ioTime;
    return result;
}

TEST_CASE("Render: OSL TestSuite", "[renderosl]")
{
    if (std::string(MATERIALX_OSL_BINARY_OSLC).empty() &&
        std::string(MATERIALX_OSL_BINARY_TESTRENDER).empty())
    {
        INFO("Skipping the OSL test suite as its executable locations haven't been set.");
        return;
    }

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");

    OslShaderRenderTester renderTester(mx::OslShaderGenerator::create(), false);
    renderTester.validate(optionsFilePath);
}

#ifdef MATERIALX_BUILD_OSOS
TEST_CASE("Render: OSL Network TestSuite", "[renderoslnetwork]")
{
    if (std::string(MATERIALX_OSL_BINARY_OSLC).empty() &&
        std::string(MATERIALX_OSL_BINARY_TESTRENDER).empty())
    {
        INFO("Skipping the OSL test suite as its executable locations haven't been set.");
        return;
    }

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");

    OslShaderRenderTester renderTester(mx::OslNetworkShaderGenerator::create(), true);
    renderTester.validate(optionsFilePath);
}
#endif
