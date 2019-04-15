//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//


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

// Create a list of generation options based on unit test options
// These options will override the original generation context options.
void ShaderRenderTester::getGenerationOptions(const RenderTestOptions& testOptions,
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

void RenderTestOptions::print(std::ostream& output) const
{
    output << "Render Test Options:" << std::endl;
    output << "\tOverride Files: { ";
    for (auto overrideFile : overrideFiles) { output << overrideFile << " "; }
    output << "} " << std::endl;
    output << "\tLight Setup Files: { ";
    for (auto lightFile : lightFiles) { output << lightFile << " "; }
    output << "} " << std::endl;
    output << "\tLanguage / Targets to run: " << std::endl;
    for (auto  l : languageAndTargets)
    {
        mx::StringVec languageAndTarget = mx::splitString(l, "_");
        size_t count = languageAndTarget.size();
        output << "\t\tLanguage: " << ((count > 0) ? languageAndTarget[0] : "NONE") << ". ";
        output << "Target: " << ((count > 1) ? languageAndTarget[1] : "NONE");
        output << std::endl;
    }
    output << "\tCheck Implementation Usage Count: " << checkImplCount << std::endl;
    output << "\tDump Generated Code: " << dumpGeneratedCode << std::endl;
    output << "\tShader Interfaces: " << shaderInterfaces << std::endl;
    output << "\tValidate Element To Render: " << validateElementToRender << std::endl;
    output << "\tCompile code: " << compileCode << std::endl;
    output << "\tRender Images: " << renderImages << std::endl;
    output << "\tSave Images: " << saveImages << std::endl;
    output << "\tDump uniforms and Attributes  " << dumpUniformsAndAttributes << std::endl;
    output << "\tNon-Shaded Geometry: " << unShadedGeometry.asString() << std::endl;
    output << "\tShaderdGeometry: " << shadedGeometry.asString() << std::endl;
    output << "\tRadiance IBL File Path " << radianceIBLPath.asString() << std::endl;
    output << "\tIrradiance IBL File Path: " << irradianceIBLPath.asString() << std::endl;
}



bool RenderTestOptions::readOptions(const std::string& optionFile)
{
    // These strings should make the input names defined in the
    // RenderTestOptions nodedef in test suite file _options.mtlx
    //
    const std::string RENDER_TEST_OPTIONS_STRING("RenderTestOptions");
    const std::string OVERRIDE_FILES_STRING("overrideFiles");
    const std::string LANGUAGE_AND_TARGETS_STRING("languageAndTargets");
    const std::string LIGHT_FILES_STRING("lightFiles");
    const std::string SHADER_INTERFACES_STRING("shaderInterfaces");
    const std::string VALIDATE_ELEMENT_TO_RENDER_STRING("validateElementToRender");
    const std::string COMPILE_CODE_STRING("compileCode");
    const std::string RENDER_IMAGES_STRING("renderImages");
    const std::string SAVE_IMAGES_STRING("saveImages");
    const std::string DUMP_UNIFORMS_AND_ATTRIBUTES_STRING("dumpUniformsAndAttributes");
    const std::string CHECK_IMPL_COUNT_STRING("checkImplCount");
    const std::string DUMP_GENERATED_CODE_STRING("dumpGeneratedCode");
    const std::string UNSHADED_GEOMETRY_STRING("unShadedGeometry");
    const std::string SHADED_GEOMETRY_STRING("shadedGeometry");
    const std::string RADIANCE_IBL_PATH_STRING("radianceIBLPath");
    const std::string IRRADIANCE_IBL_PATH_STRING("irradianceIBLPath");
    const std::string SPHERE_OBJ("sphere.obj");
    const std::string SHADERBALL_OBJ("shaderball.obj");

    overrideFiles.clear();
    dumpGeneratedCode = false;
    unShadedGeometry = SPHERE_OBJ;
    shadedGeometry = SHADERBALL_OBJ;

    MaterialX::DocumentPtr doc = MaterialX::createDocument();
    try {
        MaterialX::readFromXmlFile(doc, optionFile);

        MaterialX::NodeDefPtr optionDefs = doc->getNodeDef(RENDER_TEST_OPTIONS_STRING);
        if (optionDefs)
        {
            for (MaterialX::ParameterPtr p : optionDefs->getParameters())
            {
                const std::string& name = p->getName();
                MaterialX::ValuePtr val = p->getValue();
                if (val)
                {
                    if (name == OVERRIDE_FILES_STRING)
                    {
                        overrideFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    if (name == LIGHT_FILES_STRING)
                    {
                        lightFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    else if (name == SHADER_INTERFACES_STRING)
                    {
                        shaderInterfaces = val->asA<int>();
                    }
                    else if (name == VALIDATE_ELEMENT_TO_RENDER_STRING)
                    {
                        validateElementToRender = val->asA<bool>();
                    }
                    else if (name == COMPILE_CODE_STRING)
                    {
                        compileCode = val->asA<bool>();
                    }
                    else if (name == RENDER_IMAGES_STRING)
                    {
                        renderImages = val->asA<bool>();
                    }
                    else if (name == SAVE_IMAGES_STRING)
                    {
                        saveImages = val->asA<bool>();
                    }
                    else if (name == DUMP_UNIFORMS_AND_ATTRIBUTES_STRING)
                    {
                        dumpUniformsAndAttributes = val->asA<bool>();
                    }
                    else if (name == LANGUAGE_AND_TARGETS_STRING)
                    {
                        mx::StringVec list =  mx::splitString(p->getValueString(), ",");
                        for (auto l : list)
                        {
                            languageAndTargets.insert(l);
                        }
                    }
                    else if (name == CHECK_IMPL_COUNT_STRING)
                    {
                        checkImplCount = val->asA<bool>();
                    }
                    else if (name == DUMP_GENERATED_CODE_STRING)
                    {
                        dumpGeneratedCode = val->asA<bool>();
                    }
                    else if (name == UNSHADED_GEOMETRY_STRING)
                    {
                        unShadedGeometry = p->getValueString();
                    }
                    else if (name == SHADED_GEOMETRY_STRING)
                    {
                        shadedGeometry = p->getValueString();
                    }
                    else if (name == RADIANCE_IBL_PATH_STRING)
                    {
                        radianceIBLPath = p->getValueString();
                    }
                    else if (name == IRRADIANCE_IBL_PATH_STRING)
                    {
                        irradianceIBLPath = p->getValueString();
                    }
                }
            }
        }

        // Disable render and save of images if not compiled code will be generated
        if (!compileCode)
        {
            renderImages = false;
            saveImages = false;
        }
        // Disable saving images, if no images are to be produced
        if (!renderImages)
        {
            saveImages = false;
        }

        // If there is a filter on the files to run turn off profile checking
        if (!overrideFiles.empty())
        {
            checkImplCount = false;
        }
        return true;
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    return false;
}

void ShaderRenderTester::checkImplementationUsage(const std::string& language,
                                                  mx::StringSet& usedImpls,
                                                  mx::DocumentPtr dependLib,
                                                  mx::GenContext& context,
                                                  std::ostream& stream)
{
    // Get list of implementations a given langauge. 
    std::set<mx::ImplementationPtr> libraryImpls;
    const std::vector<mx::ElementPtr>& children = dependLib->getChildren();
    for (auto child : children)
    {
        mx::ImplementationPtr impl = child->asA<mx::Implementation>();
        if (!impl)
        {
            continue;
        }

        if (impl->getLanguage() == language)
        {
            libraryImpls.insert(impl);
        }
    }

    mx::StringSet whiteList;
    getImplementationWhiteList(whiteList);

    unsigned int implementationUseCount = 0;
    mx::StringVec skippedImplementations;
    mx::StringVec missedImplementations;
    for (auto libraryImpl : libraryImpls)
    {
        const std::string& implName = libraryImpl->getName();

        // Skip white-list items
        bool inWhiteList = false;
        for (auto w : whiteList)
        {
            if (implName.find(w) != std::string::npos)
            {
                inWhiteList = true;
                break;
            }
        }
        if (inWhiteList)
        {
            skippedImplementations.push_back(implName);
            implementationUseCount++;
            continue;
        }

        if (usedImpls.count(implName))
        {
            implementationUseCount++;
            continue;
        }

        if (context.findNodeImplementation(implName))
        {
            implementationUseCount++;
            continue;
        }
        missedImplementations.push_back(implName);
    }

    size_t libraryCount = libraryImpls.size();
    stream << "Tested: " << implementationUseCount << " out of: " << libraryCount << " library implementations." << std::endl;
    stream << "Skipped: " << skippedImplementations.size() << " implementations." << std::endl;
    if (skippedImplementations.size())
    {
        for (auto implName : skippedImplementations)
        {
            stream << "\t" << implName << std::endl;
        }
    }
    stream << "Untested: " << missedImplementations.size() << " implementations." << std::endl;
    if (missedImplementations.size())
    {
        for (auto implName : missedImplementations)
        {
            stream << "\t" << implName << std::endl;
        }
        CHECK(implementationUseCount == libraryCount);
    }
}

void ShaderRenderTester::printRunLog(const RenderProfileTimes &profileTimes,
                                     const RenderTestOptions& options,
                                     mx::StringSet& usedImpls,
                                     std::ostream& stream,
                                     mx::DocumentPtr dependLib,
                                     mx::GenContext& context,
                                     const std::string& language)
{
    profileTimes.print(stream);

    stream << "---------------------------------------" << std::endl;
    options.print(stream);

    if (options.checkImplCount)
    {
        stream << "---------------------------------------" << std::endl;
        checkImplementationUsage(language, usedImpls, dependLib, context, stream);
    }
}

bool ShaderRenderTester::validate()
{
    // Test has been turned off so just do nothing.
    // Check for an option file
    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath optionsPath = path / mx::FilePath("_options.mtlx");
    RenderUtil::RenderTestOptions options;
    if (!options.readOptions(optionsPath))
    {
        return false;
    }
    bool run = runTest(options);
    if (!run)
    {
        return false;
    }

    // Profiling times
    RenderUtil::RenderProfileTimes profileTimes;
    // Global setup timer
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

#ifdef LOG_TO_FILE
    const std::string prefex = languageTargetString();
    std::ofstream logfile(prefex + "_render_log.txt");
    std::ostream& log(logfile);
    std::string docValidLogFilename = prefex + "_render_doc_validatiion_log.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile(prefex + "__render_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
    std::ostream& log(std::cout);
    std::string docValidLogFilename = "std::cout";
    std::ostream& docValidLog(std::cout);
    std::ostream& profilingLog(std::cout);
#endif

    // For debugging, add files to this set to override
    // which files in the test suite are being tested.
    // Add only the test suite filename not the full path.
    mx::StringSet testfileOverride;
    for (auto filterFile : options.overrideFiles)
    {
        testfileOverride.insert(filterFile);
    }

    RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.ioTime, "Global I/O time");
    mx::FilePathVec dirs;
    mx::FilePath baseDirectory = path;
    dirs = baseDirectory.getSubDirectories();

    ioTimer.endTimer();

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib = mx::createDocument();
    mx::StringSet excludeFiles;

    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    GenShaderUtil::loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    GenShaderUtil::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);
    // Load any addition per validator libraries
    loadLibraries(dependLib, options);
    ioTimer.endTimer();

    // Create validators and generators
    RenderUtil::AdditiveScopedTimer setupTime(profileTimes.languageTimes.setupTime, "Setup time");

    createShaderGenerator();
    createValidator(log);

    mx::ColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getLanguage());
    colorManagementSystem->loadLibrary(dependLib);
    _shaderGenerator->setColorManagementSystem(colorManagementSystem);

    mx::GenContext context(_shaderGenerator);
    context.registerSourceCodeSearchPath(searchPath);
    registerSourceCodeSearchPaths(context);

    setupTime.endTimer();

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

    registerLights(dependLib, options, context);

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    RenderUtil::AdditiveScopedTimer validateTimer(profileTimes.validateTime, "Global validation time");
    RenderUtil::AdditiveScopedTimer renderableSearchTimer(profileTimes.renderableSearchTime, "Global renderable search time");

    mx::StringSet usedImpls;

    const std::string MTLX_EXTENSION("mtlx");
    const std::string OPTIONS_FILENAME("_options.mtlx");
    for (auto dir : dirs)
    {
        ioTimer.startTimer();
        mx::FilePathVec files;
        files = dir.getFilesInDirectory(MTLX_EXTENSION);
        ioTimer.endTimer();

        for (const std::string& file : files)
        {

            if (file == OPTIONS_FILENAME)
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
            mx::readFromXmlFile(doc, filename, dir);

            doc->importLibrary(dependLib, &importOptions);
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
            for (auto element : elements)
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
    printRunLog(profileTimes, options, usedImpls, profilingLog, dependLib, context,
                _shaderGenerator->getLanguage());
    return true;
}

} // namespace RenderUtil

