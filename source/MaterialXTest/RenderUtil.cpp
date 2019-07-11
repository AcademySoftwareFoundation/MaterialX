//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Util.h>
#include <MaterialXTest/RenderUtil.h>
#include <MaterialXTest/Catch/catch.hpp>

namespace mx = MaterialX;

namespace RenderUtil
{
// Utility to create a light rig for hardware render testing
void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::GenContext& context,
                    const mx::FilePath& envIrradiancePath, const mx::FilePath& envRadiancePath)
{
    // Scan for lights
    std::vector<mx::NodePtr> lights;
    lightHandler.findLights(doc, lights);
    lightHandler.registerLights(doc, lights, context);

    // Set the list of lights on the with the generator
    lightHandler.setLightSources(lights);
    // Set up IBL inputs
    lightHandler.setLightEnvIrradiancePath(envIrradiancePath);
    lightHandler.setLightEnvRadiancePath(envRadiancePath);
}

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

bool ShaderRenderTester::validate(const mx::FilePathVec& testRootPaths, const mx::FilePath optionsFilePath)
{
    // Test has been turned off so just do nothing.
    // Check for an option file
    GenShaderUtil::TestSuiteOptions options;
    if (!options.readOptions(optionsFilePath))
    {
        std::cout << "Can't find options file. Skip test." << std::endl;
        return false;
    }
    if (!runTest(options))
    {
        std::cout << "Language / target: " << _languageTargetString << " not set to run. Skip test." << std::endl;
        return false;
    }

    // Profiling times
    RenderUtil::RenderProfileTimes profileTimes;
    // Global setup timer
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

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

    // Add files to override the files in the test suite to be tested.
    mx::StringSet testfileOverride;
    for (const auto& filterFile : options.overrideFiles)
    {
        testfileOverride.insert(filterFile);
    }

    RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.ioTime, "Global I/O time");
    mx::FilePathVec dirs;
    for (const auto& testRoot : testRootPaths)
    {
        mx::FilePathVec testRootDirs = testRoot.getSubDirectories();
        dirs.insert(std::end(dirs), std::begin(testRootDirs), std::end(testRootDirs));
    }

    ioTimer.endTimer();

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib = mx::createDocument();

    // Add files to skip
    addSkipFiles();

    const mx::StringVec libraries = { "stdlib", "pbrlib", "lights" };
    loadLibraries(libraries, searchPath, dependLib, nullptr);

    // Load shader definitions used in the test suite.
    loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);
    loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/usd.mtlx"), dependLib);

    // Load any addition per validator libraries
    loadAdditionalLibraries(dependLib, options);
    ioTimer.endTimer();

    // Create validators and generators
    RenderUtil::AdditiveScopedTimer setupTime(profileTimes.languageTimes.setupTime, "Setup time");

    createValidator(log);

    mx::ColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getLanguage());
    colorManagementSystem->loadLibrary(dependLib);
    _shaderGenerator->setColorManagementSystem(colorManagementSystem);

    mx::GenContext context(_shaderGenerator);
    context.registerSourceCodeSearchPath(searchPath);
    registerSourceCodeSearchPaths(context);

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

            const mx::FilePath filePath = mx::FilePath(dir) / mx::FilePath(file);
            const std::string filename = filePath;

            mx::DocumentPtr doc = mx::createDocument();
            try
            {
                mx::FileSearchPath readSearchPath(searchPath.asString());
                readSearchPath.append(dir);
                mx::readFromXmlFile(doc, filename, readSearchPath.asString());
            }
            catch (mx::Exception& e)
            {
                docValidLog << "Failed to load in file: " << filename << ". Error: " << e.what() << std::endl;
                WARN("Failed to load in file: " + filename + "See: " + docValidLogFilename + " for details.");                    
            }

            doc->importLibrary(dependLib, &copyOptions);
            ioTimer.endTimer();

            validateTimer.startTimer();
            std::cout << "- Validating MTLX file: " << filename << std::endl;
            log << "MTLX Filename: " << filename << std::endl;

            // Validate the test document
            std::string validationErrors;
            bool validDoc = doc->validate(&validationErrors);
            if (!validDoc)
            {
                docValidLog << filename << std::endl;
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
                mx::OutputPtr output = element->asA<mx::Output>();
                mx::ShaderRefPtr shaderRef = element->asA<mx::ShaderRef>();
                mx::NodeDefPtr nodeDef = nullptr;
                if (output)
                {
                    nodeDef = output->getConnectedNode()->getNodeDef();
                }
                else if (shaderRef)
                {
                    nodeDef = shaderRef->getNodeDef();
                }
                if (nodeDef)
                {
                    mx::string elementName = mx::replaceSubstrings(element->getNamePath(), pathMap);
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
                            runValidator(elementName, element, context, doc, log, options, profileTimes, imageSearchPath, outputPath);
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
