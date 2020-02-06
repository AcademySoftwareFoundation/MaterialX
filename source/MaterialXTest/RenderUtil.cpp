//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/RenderUtil.h>
#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXRender/Image.h>

#include <MaterialXGenShader/UnitConverter.h>
#include <MaterialXGenShader/Util.h>

namespace mx = MaterialX;

namespace RenderUtil
{

ShaderRenderTester::ShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
    _shaderGenerator(shaderGenerator),
    _languageTargetString(shaderGenerator->getLanguage() + "_" + shaderGenerator->getTarget())
{
}

ShaderRenderTester::~ShaderRenderTester()
{
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
    // Alway fallback to complete if no options specified.
    if ((testOptions.shaderInterfaces & 2) || optionsList.empty())
    {
        mx::GenOptions completeOption = originalOptions;
        completeOption.shaderInterfaceType = mx::SHADER_INTERFACE_COMPLETE;
        optionsList.push_back(completeOption);
    }
}

void ShaderRenderTester::printRunLog(const RenderProfileTimes &profileTimes,
                                     const GenShaderUtil::TestSuiteOptions& options,
                                     std::ostream& stream,
                                     mx::DocumentPtr dependLib)
{
    profileTimes.print(stream);

    stream << "---------------------------------------" << std::endl;
    options.print(stream);

    //if (options.checkImplCount)
    //{
    //    stream << "---------------------------------------" << std::endl;
    //    mx::StringSet whiteList;
    //    getImplementationWhiteList(whiteList);
    //    GenShaderUtil::checkImplementationUsage(language, usedImpls, whiteList, dependLib, context, stream);
    //}
}

void ShaderRenderTester::loadDependentLibraries(GenShaderUtil::TestSuiteOptions options, mx::FilePath searchPath, mx::DocumentPtr& dependLib)
{
    dependLib = mx::createDocument();

    const mx::StringVec libraries = { "stdlib", "pbrlib", "lights" };
    mx::loadLibraries(libraries, searchPath, dependLib, nullptr);
    for (size_t i = 0; i < options.externalLibraryPaths.size(); i++)
    {
        const mx::FilePath& libraryPath = options.externalLibraryPaths[i];
        for (const mx::FilePath& libraryFile : libraryPath.getFilesInDirectory("mtlx"))
        {
            std::cout << "Extra library path: " << (libraryPath / libraryFile).asString() << std::endl;
            mx::loadLibrary((libraryPath / libraryFile), dependLib);
        }
    }

    // Load shader definitions used in the test suite.
    loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);
    loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/usd_preview_surface.mtlx"), dependLib);

    // Load any addition per renderer libraries
    loadAdditionalLibraries(dependLib, options);
}

bool ShaderRenderTester::validate(const mx::FilePathVec& testRootPaths, const mx::FilePath optionsFilePath)
{
#ifdef LOG_TO_FILE
    std::ofstream logfile(_languageTargetString + "_render_log.txt");
    std::ostream& log(logfile);
    std::string docValidLogFilename = _languageTargetString + "_render_doc_validation_log.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile(_languageTargetString + "__render_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
    std::ostream& log(std::cout);
    std::string docValidLogFilename = "std::cout";
    std::ostream& docValidLog(std::cout);
    std::ostream& profilingLog(std::cout);
#endif

    // Test has been turned off so just do nothing.
    // Check for an option file
    GenShaderUtil::TestSuiteOptions options;
    if (!options.readOptions(optionsFilePath))
    {
        log << "Can't find options file. Skip test." << std::endl;
        return false;
    }
    if (!runTest(options))
    {
        log << "Language / target: " << _languageTargetString << " not set to run. Skip test." << std::endl;
        return false;
    }

    // Profiling times
    RenderUtil::RenderProfileTimes profileTimes;
    // Global setup timer
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

    // Add files to override the files in the test suite to be tested.
    mx::StringSet testfileOverride;
    for (const auto& filterFile : options.overrideFiles)
    {
        testfileOverride.insert(filterFile);
    }

    RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.ioTime, "Global I/O time");
    mx::FilePathVec dirs;
    if (options.externalTestPaths.size() == 0)
    {
        for (const auto& testRoot : testRootPaths)
        {
            mx::FilePathVec testRootDirs = testRoot.getSubDirectories();
            dirs.insert(std::end(dirs), std::begin(testRootDirs), std::end(testRootDirs));
        }
    }
    else
    {
        // Use test roots from options file
        for (size_t i = 0; i < options.externalTestPaths.size(); i++)
        {
            std::cout << "Test root: " << options.externalTestPaths[i].asString() << std::endl;
            dirs.push_back(options.externalTestPaths[i]);
        }
    }

    ioTimer.endTimer();

    // Add files to skip
    addSkipFiles();

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib;
    loadDependentLibraries(options, searchPath, dependLib);
    ioTimer.endTimer();

    // Create renderers and generators
    RenderUtil::AdditiveScopedTimer setupTime(profileTimes.languageTimes.setupTime, "Setup time");

    createRenderer(log);

    mx::ColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getLanguage());
    colorManagementSystem->loadLibrary(dependLib);
    _shaderGenerator->setColorManagementSystem(colorManagementSystem);

    // Setup Unit system and working space
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(_shaderGenerator->getLanguage());
    _shaderGenerator->setUnitSystem(unitSystem);
    mx::UnitConverterRegistryPtr registry = mx::UnitConverterRegistry::create();
    mx::UnitTypeDefPtr distanceTypeDef = dependLib->getUnitTypeDef("distance");
    registry->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = dependLib->getUnitTypeDef("angle");
    registry->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
    _shaderGenerator->getUnitSystem()->loadLibrary(dependLib);
    _shaderGenerator->getUnitSystem()->setUnitConverterRegistry(registry);

    mx::GenContext context(_shaderGenerator);
    context.registerSourceCodeSearchPath(searchPath);
    registerSourceCodeSearchPaths(context);

    // Set target unit space
    context.getOptions().targetDistanceUnit = "meter";

    setupTime.endTimer();

    registerLights(dependLib, options, context);

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    RenderUtil::AdditiveScopedTimer validateTimer(profileTimes.validateTime, "Global validation time");
    RenderUtil::AdditiveScopedTimer renderableSearchTimer(profileTimes.renderableSearchTime, "Global renderable search time");

    mx::StringSet usedImpls;

    mx::CopyOptions copyOptions;
    copyOptions.skipConflictingElements = true;

    const std::string MTLX_EXTENSION("mtlx");
    for (const auto& dir : dirs)
    {
        ioTimer.startTimer();
        mx::FilePathVec files;
        files = dir.getFilesInDirectory(MTLX_EXTENSION);
        ioTimer.endTimer();

        for (const std::string& file : files)
        {
            if (_skipFiles.count(file))
            {
                continue;
            }

            ioTimer.startTimer();
            // Check if a file override set is used and ignore all files
            // not part of the override set
            if (testfileOverride.size() && testfileOverride.count(file) == 0)
            {
                ioTimer.endTimer();
                continue;
            }

            const mx::FilePath filename = mx::FilePath(dir) / mx::FilePath(file);
            mx::DocumentPtr doc = mx::createDocument();
            try
            {
                mx::FileSearchPath readSearchPath(searchPath);
                readSearchPath.append(dir);
                mx::XmlReadOptions readOptions;
                readOptions.desiredMajorVersion = options.desiredMajorVersion;
                readOptions.desiredMinorVersion = options.desiredMinorVersion;
                mx::readFromXmlFile(doc, filename, readSearchPath, &readOptions);
            }
            catch (mx::Exception& e)
            {
                docValidLog << "Failed to load in file: " << filename.asString() << ". Error: " << e.what() << std::endl;
                WARN("Failed to load in file: " + filename.asString() + "See: " + docValidLogFilename + " for details.");
            }

            doc->importLibrary(dependLib, &copyOptions);
            ioTimer.endTimer();

            validateTimer.startTimer();
            std::cout << "- Validating MTLX file: " << filename.asString() << std::endl;
            log << "MTLX Filename: " << filename.asString() << std::endl;

            // Validate the test document
            std::string validationErrors;
            bool validDoc = doc->validate(&validationErrors);
            if (!validDoc)
            {
                docValidLog << filename.asString() << std::endl;
                docValidLog << validationErrors << std::endl;
            }
            validateTimer.endTimer();
            CHECK(validDoc);

            renderableSearchTimer.startTimer();
            std::vector<mx::TypedElementPtr> elements;
            try
            {
                mx::findRenderableElements(doc, elements);
            }
            catch (mx::Exception& e)
            {
                docValidLog << e.what() << std::endl;
                WARN("Find renderable elements failed, see: " + docValidLogFilename + " for details.");
            }
            renderableSearchTimer.endTimer();

            std::string outputPath = mx::FilePath(dir) / mx::FilePath(mx::removeExtension(file));
            mx::FileSearchPath imageSearchPath(dir);
            for (const auto& element : elements)
            {
                mx::TypedElementPtr targetElement = element;
                mx::OutputPtr output = targetElement->asA<mx::Output>();
                mx::ShaderRefPtr shaderRef = targetElement->asA<mx::ShaderRef>();
                mx::NodePtr outputNode = targetElement->asA<mx::Node>();
                mx::NodeDefPtr nodeDef = nullptr;
                if (output)
                {
                    outputNode = output->getConnectedNode();
                    // Handle connected upstream material nodes later on.
                    if (outputNode->getType() != mx::MATERIAL_TYPE_STRING)
                    {
                        nodeDef = outputNode->getNodeDef();
                    }
                }
                else if (shaderRef)
                {
                    nodeDef = shaderRef->getNodeDef();
                }

                // Handle material node checking. For now only check first surface shader if any
                if (outputNode && outputNode->getType() == mx::MATERIAL_TYPE_STRING)
                {
                    std::vector<mx::NodePtr> shaderNodes = getShaderNodes(outputNode, mx::SURFACE_SHADER_TYPE_STRING);
                    if (!shaderNodes.empty())
                    {
                        nodeDef = shaderNodes[0]->getNodeDef();
                        targetElement = shaderNodes[0];
                    }
                }

                if (nodeDef)
                {
                    mx::string elementName = mx::replaceSubstrings(targetElement->getNamePath(), pathMap);
                    elementName = mx::createValidName(elementName);
                    {
                        renderableSearchTimer.startTimer();
                        mx::InterfaceElementPtr impl = nodeDef->getImplementation(_shaderGenerator->getTarget(), _shaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl->getName());
                            }
                            runRenderer(elementName, targetElement, context, doc, log, options, profileTimes, imageSearchPath, outputPath);
                        }
                    }
                }
            }
        }
    }

    // Dump out profiling information
    totalTime.endTimer();
    printRunLog(profileTimes, options, profilingLog, dependLib);

    return true;
}

} // namespace RenderUtil
