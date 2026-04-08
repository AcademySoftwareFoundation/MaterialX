//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXFormat/Util.h>

#ifdef MATERIALX_BUILD_OCIO
#include <MaterialXGenShader/OcioColorManagementSystem.h>
#endif

#ifdef MATERIALX_BUILD_PERFETTO_TRACING
#include <MaterialXTrace/Tracing.h>
#include <optional>
#endif

namespace mx = MaterialX;

namespace RenderUtil
{

ShaderRenderTester::ShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
    _shaderGenerator(shaderGenerator),
    _resolveImageFilenames(false)
{
}

ShaderRenderTester::~ShaderRenderTester()
{
}

bool ShaderRenderTester::validate(const mx::FilePath optionsFilePath)
{
    // per-run state objects, logger, profiler
    TestRunState runState;
    TestRunLogger logger;
    TestRunProfiler profiler;

    // Read options first so we can use outputDirectory for log files
    if (!loadOptions(optionsFilePath, runState))
        return false;

    // Start instrumentation after options are loaded so that log file paths
    // are available. Profiling excludes option loading, but includes the
    // subsequent test-file collection, library loading, and generator setup.
    const std::string& target = _shaderGenerator->getTarget();
#ifdef MATERIALX_BUILD_PERFETTO_TRACING
    TestRunTracer tracer;
    tracer.start(target, runState.options);
#endif
    logger.start(target, runState.options);
    profiler.start();

    // Data search path
    runState.searchPath = mx::getDefaultDataSearchPath();

    mx::ScopedTimer ioTimer(&profiler.times().ioTime);
    mx::FilePathVec files = collectTestFiles(runState);
    ioTimer.endTimer();

    // Load in the library dependencies once.
    // This will be imported in each test document below.
    ioTimer.startTimer();
    loadDependentLibraries(runState);
    ioTimer.endTimer();

    // Create renderers and generators
    initializeGeneratorContext(runState, logger, profiler);

    // Map to replace "/" in Element path and ":" in namespaced names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";
    pathMap[":"] = "_";

    for (const mx::FilePath& filename : files)
    {
        DocumentInfo docInfo = loadAndValidateDocument(filename, runState, logger, profiler);

        if (!docInfo.doc || !docInfo.valid)
            continue;

        for (const auto& element : docInfo.elements)
        {
            mx::string elementName = mx::createValidName(mx::replaceSubstrings(element->getNamePath(), pathMap));
            runRenderer(elementName, element, *runState.context, docInfo.doc, logger.renderLog(), runState.options,
                        profiler.times(), docInfo.imageSearchPath, docInfo.outputPath, nullptr);
        }
    }

    // Print profiling summary on the normal path.
    // All resource cleanup is handled by destructors.
    profiler.printSummary(runState.options, logger.profilingLog());

    return true;
}

void ShaderRenderTester::loadDependentLibraries(TestRunState& runState)
{
    runState.dependLib = mx::createDocument();

    mx::loadLibraries({ "libraries" }, runState.searchPath, runState.dependLib);
    for (size_t i = 0; i < runState.options.extraLibraryPaths.size(); i++)
    {
        const mx::FilePath& libraryPath = runState.options.extraLibraryPaths[i];
        for (const mx::FilePath& libraryFile : libraryPath.getFilesInDirectory("mtlx"))
        {
            std::cout << "Extra library path: " << (libraryPath / libraryFile).asString() << std::endl;
            mx::loadLibrary((libraryPath / libraryFile), runState.dependLib);
        }
    }

    // Load any additional per-renderer libraries
    loadAdditionalLibraries(runState.dependLib, runState.options);
}

// Create a list of generation options based on unit test options
// These options will override the original generation context options.
void ShaderRenderTester::getGenerationOptions(const GenShaderUtil::TestSuiteOptions& testOptions,
                                              const mx::GenOptions& originalOptions,
                                              std::vector<mx::GenOptions>& optionsList)
{
    optionsList.clear();
    if (testOptions.shaderInterfaces & 1)
    {
        mx::GenOptions reducedOption = originalOptions;
        reducedOption.shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;
        optionsList.push_back(reducedOption);
    }
    // Always fallback to complete if no options specified.
    if ((testOptions.shaderInterfaces & 2) || optionsList.empty())
    {
        mx::GenOptions completeOption = originalOptions;
        completeOption.shaderInterfaceType = mx::SHADER_INTERFACE_COMPLETE;
        optionsList.push_back(completeOption);
    }
}

void ShaderRenderTester::addAdditionalTestStreams(mx::MeshPtr mesh)
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

    auto sineData = [](float texCoord, float freq){
        const float PI = std::acos(-1.0f);
        float angle = texCoord * 2 * PI * freq;
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

bool ShaderRenderTester::loadOptions(const mx::FilePath& optionsFilePath, TestRunState& runState)
{
    // Read options first so we can use outputDirectory for log files
    runState.options = GenShaderUtil::TestSuiteOptions();
    if (!runState.options.readOptions(optionsFilePath))
    {
        std::cerr << "Can't find options file. Skip test." << std::endl;
        return false;
    }
    if (!runTest(runState.options))
    {
        std::cerr << "Target: " << _shaderGenerator->getTarget() << " not set to run. Skip test." << std::endl;
        return false;
    }
    return true;
}

mx::FilePathVec ShaderRenderTester::collectTestFiles(const TestRunState& runState)
{
    const std::string MTLX_EXTENSION("mtlx");

    // Add files to override the files in the test suite to be tested.
    mx::StringSet testfileOverride;
    for (const auto& filterFile : runState.options.overrideFiles)
    {
        testfileOverride.insert(filterFile);
    }

    mx::FilePathVec files;
    for (const auto& root : runState.options.renderTestPaths)
    {
        auto resolvedRoot = runState.searchPath.find(root);
        if (!resolvedRoot.exists())
            continue;
        if (resolvedRoot.isDirectory())
        {
            mx::FilePathVec testRootDirs = resolvedRoot.getSubDirectories();
            for (auto& dir : testRootDirs)
            {
                mx::FilePathVec dirFiles = dir.getFilesInDirectory(MTLX_EXTENSION);
                files.reserve(files.size() + dirFiles.size());
                for (auto& file : dirFiles)
                    files.push_back(dir / file);
            }
        }
        else
        {
            files.push_back(resolvedRoot);
        }
    }

    if (!testfileOverride.empty())
    {
        mx::FilePathVec filtered;
        for (const auto& f : files)
        {
            if (testfileOverride.count(f.getBaseName()))
                filtered.push_back(f);
        }
        return filtered;
    }

    return files;
}

void ShaderRenderTester::initializeGeneratorContext(TestRunState& runState, TestRunLogger& logger,
                                                    TestRunProfiler& profiler)
{
    mx::ScopedTimer setupTime(&profiler.times().languageTimes.setupTime);

    createRenderer(logger.renderLog());

    addSkipFiles();

    mx::ColorManagementSystemPtr colorManagementSystem;
#ifdef MATERIALX_BUILD_OCIO
    try
    {
        colorManagementSystem =
            mx::OcioColorManagementSystem::createFromBuiltinConfig(
                "ocio://studio-config-latest",
                _shaderGenerator->getTarget());
    }
    catch (const std::exception& /*e*/)
    {
        colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getTarget());
    }
#else
    colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getTarget());
#endif
    colorManagementSystem->loadLibrary(runState.dependLib);
    _shaderGenerator->setColorManagementSystem(colorManagementSystem);

    // Setup Unit system and working space
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(_shaderGenerator->getTarget());
    _shaderGenerator->setUnitSystem(unitSystem);
    mx::UnitConverterRegistryPtr registry = mx::UnitConverterRegistry::create();
    mx::UnitTypeDefPtr distanceTypeDef = runState.dependLib->getUnitTypeDef("distance");
    registry->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = runState.dependLib->getUnitTypeDef("angle");
    registry->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
    _shaderGenerator->getUnitSystem()->loadLibrary(runState.dependLib);
    _shaderGenerator->getUnitSystem()->setUnitConverterRegistry(registry);

    runState.context = std::make_unique<mx::GenContext>(_shaderGenerator);
    runState.context->registerSourceCodeSearchPath(runState.searchPath);
    runState.context->registerSourceCodeSearchPath(runState.searchPath.find("libraries/stdlib/genosl/include"));

    // Set target unit space
    runState.context->getOptions().targetDistanceUnit = "meter";

    // Register shader metadata defined in the libraries.
    _shaderGenerator->registerShaderMetadata(runState.dependLib, *runState.context);

    setupTime.endTimer();

    if (!runState.options.enableDirectLighting)
    {
        runState.context->getOptions().hwMaxActiveLightSources = 0;
    }
    registerLights(runState.dependLib, runState.options, *runState.context);
}

DocumentInfo ShaderRenderTester::loadAndValidateDocument(const mx::FilePath& filename,
                                                         TestRunState& runState,
                                                         TestRunLogger& logger,
                                                         TestRunProfiler& profiler)
{
    DocumentInfo info;

    mx::ScopedTimer ioTimer(&profiler.times().ioTime);

    if (_skipFiles.count(filename.getBaseName()) > 0)
        return info;

    info.doc = mx::createDocument();
    try
    {
        mx::readFromXmlFile(info.doc, filename, runState.searchPath);
    }
    catch (mx::Exception& e)
    {
        logger.validationLog() << "Failed to load in file: " << filename.asString() << ". Error: " << e.what() << std::endl;
        FAIL_CHECK("Failed to load in file: " + filename.asString() + ". See: " + logger.validationLogFilename() + " for details.");
        return info;
    }

    // For each new file clear the implementation cache.
    // Since the new file might contain implementations with names
    // colliding with implementations in previous test cases.
    runState.context->clearNodeImplementations();

    info.doc->setDataLibrary(runState.dependLib);

    // Register types from the document.
    _shaderGenerator->registerTypeDefs(info.doc);

    ioTimer.endTimer();

    mx::ScopedTimer validateTimer(&profiler.times().validateTime);
    logger.renderLog() << "MTLX Filename: " << filename.asString() << std::endl;

    // Validate the test document
    std::string validationErrors;
    info.valid = info.doc->validate(&validationErrors);
    if (!info.valid)
    {
        logger.validationLog() << filename.asString() << std::endl;
        logger.validationLog() << validationErrors << std::endl;
    }
    validateTimer.endTimer();
    CHECK(info.valid);

    info.imageSearchPath = mx::FileSearchPath(filename.getParentPath());
    info.imageSearchPath.append(runState.searchPath);

    // Resolve file names if specified
    if (_resolveImageFilenames)
    {
        mx::flattenFilenames(info.doc, info.imageSearchPath, _customFilenameResolver);
    }

    info.outputPath = filename;
    info.outputPath.removeExtension();

    // If outputDirectory is set, redirect output to that directory
    // while preserving the material name as a subdirectory
    if (!runState.options.outputDirectory.isEmpty())
    {
        // Get just the material directory name (e.g., "standard_surface_carpaint")
        mx::FilePath materialDir = info.outputPath.getBaseName();
        info.outputPath = runState.options.outputDirectory / materialDir;
    }

    mx::ScopedTimer renderableSearchTimer(&profiler.times().renderableSearchTime);
    try
    {
        info.elements = mx::findRenderableElements(info.doc);
    }
    catch (mx::Exception& e)
    {
        logger.validationLog() << e.what() << std::endl;
        WARN("Shader generation error in " + filename.asString() + ": " + e.what());
    }
    renderableSearchTimer.endTimer();

    return info;
}

// ---------------------------------------------------------------------------
// TestRunLogger
// ---------------------------------------------------------------------------

void TestRunLogger::start(const std::string& target, const GenShaderUtil::TestSuiteOptions& options)
{
#ifdef LOG_TO_FILE
    mx::FilePath logPath = options.resolveOutputPath(target + "_render_log.txt");
    _renderLog = std::make_unique<std::ofstream>(logPath.asString());

    mx::FilePath docValidLogPath = options.resolveOutputPath(target + "_render_doc_validation_log.txt");
    _validationLogFilename = docValidLogPath.asString();
    _validationLog = std::make_unique<std::ofstream>(_validationLogFilename);

    mx::FilePath profilingLogPath = options.resolveOutputPath(target + "_render_profiling_log.txt");
    _profilingLog = std::make_unique<std::ofstream>(profilingLogPath.asString());
#else
    _renderLog.reset();
    _validationLog.reset();
    _profilingLog.reset();
    _validationLogFilename = "std::cout";
#endif
}

TestRunLogger::~TestRunLogger()
{
    _renderLog.reset();
    _validationLog.reset();
    _profilingLog.reset();
}

// ---------------------------------------------------------------------------
// TestRunProfiler
// ---------------------------------------------------------------------------

void TestRunProfiler::start()
{
    _profileTimes = RenderProfileTimes();
    _totalTimer = std::make_unique<mx::ScopedTimer>(&_profileTimes.totalTime);
}

void TestRunProfiler::printSummary(const GenShaderUtil::TestSuiteOptions& options,
                                   std::ostream& profilingLog)
{
    _totalTimer.reset();
    _profileTimes.print(profilingLog);
    profilingLog << "---------------------------------------" << std::endl;
    options.print(profilingLog);

    // Print effective output directory for easy access (clickable in terminals)
    if (!options.outputDirectory.isEmpty())
    {
        std::cout << std::endl << "Test artifacts written to: " << options.outputDirectory.asString() << std::endl;
    }
}

// ---------------------------------------------------------------------------
// TestRunTracer
// ---------------------------------------------------------------------------

#ifdef MATERIALX_BUILD_PERFETTO_TRACING

struct TestRunTracer::State
{
    std::optional<mx::Tracing::Dispatcher::ShutdownGuard> guard;
};

void TestRunTracer::start(const std::string& target, const GenShaderUtil::TestSuiteOptions& options)
{
    _state = std::make_unique<State>();
    // Initialize tracing with target-specific trace filename (if enabled in options)
    if (options.enableTracing)
    {
        mx::FilePath tracePath = options.resolveOutputPath(target + "_render_trace.perfetto-trace");
        mx::Tracing::Dispatcher::getInstance().setSink(
            mx::Tracing::createPerfettoSink(tracePath.asString()));
        // Scope guard ensures tracing is shut down on any exit path (return, exception, etc.)
        _state->guard.emplace();
    }
}

TestRunTracer::~TestRunTracer()
{
    _state.reset();
}

#endif // MATERIALX_BUILD_PERFETTO_TRACING

} // namespace RenderUtil
