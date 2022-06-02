//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

namespace RenderUtil
{

ShaderRenderTester::ShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
    _shaderGenerator(shaderGenerator),
    _resolveImageFilenames(false),
    _emitColorTransforms(true)
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
}

void ShaderRenderTester::loadDependentLibraries(GenShaderUtil::TestSuiteOptions options, mx::FileSearchPath searchPath, mx::DocumentPtr& dependLib)
{
    dependLib = mx::createDocument();

    mx::loadLibraries({ "libraries" }, searchPath, dependLib);
    for (size_t i = 0; i < options.extraLibraryPaths.size(); i++)
    {
        const mx::FilePath& libraryPath = options.extraLibraryPaths[i];
        for (const mx::FilePath& libraryFile : libraryPath.getFilesInDirectory("mtlx"))
        {
            std::cout << "Extra library path: " << (libraryPath / libraryFile).asString() << std::endl;
            mx::loadLibrary((libraryPath / libraryFile), dependLib);
        }
    }

    // Load any addition per renderer libraries
    loadAdditionalLibraries(dependLib, options);
}

void ShaderRenderTester::addSkipFiles()
{
    _skipFiles.insert("_options.mtlx");
    _skipFiles.insert("light_rig_test_1.mtlx");
    _skipFiles.insert("light_rig_test_2.mtlx");
    _skipFiles.insert("light_compound_test.mtlx");
    _skipFiles.insert("xinclude_search_path.mtlx");
    _skipFiles.insert("1_38_parameter_to_input.mtlx");
    _skipFiles.insert("1_36_to_1_37.mtlx");
    _skipFiles.insert("1_37_to_1_38.mtlx");
    _skipFiles.insert("material_element_to_surface_material.mtlx");
}

bool ShaderRenderTester::validate(const mx::FilePath optionsFilePath)
{
#ifdef LOG_TO_FILE
    std::ofstream logfile(_shaderGenerator->getTarget() + "_render_log.txt");
    std::ostream& log(logfile);
    std::string docValidLogFilename = _shaderGenerator->getTarget() + "_render_doc_validation_log.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile(_shaderGenerator->getTarget() + "_render_profiling_log.txt");
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
        log << "Target: " << _shaderGenerator->getTarget() << " not set to run. Skip test." << std::endl;
        return false;
    }

    // Profiling times
    RenderUtil::RenderProfileTimes profileTimes;
    // Global setup timer
    mx::ScopedTimer totalTime(&profileTimes.totalTime);

    // Add files to override the files in the test suite to be tested.
    mx::StringSet testfileOverride;
    for (const auto& filterFile : options.overrideFiles)
    {
        testfileOverride.insert(filterFile);
    }

    mx::ScopedTimer ioTimer(&profileTimes.ioTime);
    mx::FilePathVec dirs;
    for (const auto& root : options.renderTestPaths)
    {
        mx::FilePathVec testRootDirs = root.getSubDirectories();
        dirs.insert(std::end(dirs), std::begin(testRootDirs), std::end(testRootDirs));
    }
    ioTimer.endTimer();

    // Add files to skip
    addSkipFiles();

    // Library search path
    mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    mx::FileSearchPath searchPath;
    searchPath.append(currentPath);

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib;
    loadDependentLibraries(options, searchPath, dependLib);
    ioTimer.endTimer();

    // Create renderers and generators
    mx::ScopedTimer setupTime(&profileTimes.languageTimes.setupTime);

    createRenderer(log);

    mx::ColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getTarget());
    colorManagementSystem->loadLibrary(dependLib);
    _shaderGenerator->setColorManagementSystem(colorManagementSystem);

    // Setup Unit system and working space
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(_shaderGenerator->getTarget());
    _shaderGenerator->setUnitSystem(unitSystem);
    mx::UnitConverterRegistryPtr registry = mx::UnitConverterRegistry::create();
    mx::UnitTypeDefPtr distanceTypeDef = dependLib->getUnitTypeDef("distance");
    registry->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
    mx::UnitTypeDefPtr angleTypeDef = dependLib->getUnitTypeDef("angle");
    registry->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
    _shaderGenerator->getUnitSystem()->loadLibrary(dependLib);
    _shaderGenerator->getUnitSystem()->setUnitConverterRegistry(registry);

    mx::GenContext context(_shaderGenerator);
    context.registerSourceCodeSearchPath(currentPath);
    context.registerSourceCodeSearchPath(currentPath / mx::FilePath("libraries/stdlib/genosl/include"));

    // Set target unit space
    context.getOptions().targetDistanceUnit = "meter";

    // Set whether to emit colorspace transforms
    context.getOptions().emitColorTransforms = _emitColorTransforms;

    // Register shader metadata defined in the libraries.
    _shaderGenerator->registerShaderMetadata(dependLib, context);

    setupTime.endTimer();

    registerLights(dependLib, options, context);

    // Map to replace "/" in Element path and ":" in namespaced names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";
    pathMap[":"] = "_";

    mx::ScopedTimer validateTimer(&profileTimes.validateTime);
    mx::ScopedTimer renderableSearchTimer(&profileTimes.renderableSearchTime);

    mx::StringSet usedImpls;

    const std::string MTLX_EXTENSION("mtlx");
    for (const auto& dir : dirs)
    {
        ioTimer.startTimer();
        mx::FilePathVec files;
        files = dir.getFilesInDirectory(MTLX_EXTENSION);
        ioTimer.endTimer();

        for (const mx::FilePath& file : files)
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
                mx::readFromXmlFile(doc, filename, readSearchPath);
            }
            catch (mx::Exception& e)
            {
                docValidLog << "Failed to load in file: " << filename.asString() << ". Error: " << e.what() << std::endl;
                WARN("Failed to load in file: " + filename.asString() + "See: " + docValidLogFilename + " for details.");
            }

            // For each new file clear the implementation cache.
            // Since the new file might contain implementations with names
            // colliding with implementations in previous test cases.
            context.clearNodeImplementations();

            doc->importLibrary(dependLib);
            ioTimer.endTimer();

            validateTimer.startTimer();
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

            mx::FileSearchPath imageSearchPath(dir);
            imageSearchPath.append(searchPath);
            
            // Resolve file names if specified
            if (_resolveImageFilenames)
            {
                mx::flattenFilenames(doc, imageSearchPath, _customFilenameResolver);
            }

            mx::FilePath outputPath = mx::FilePath(dir) / file;
            outputPath.removeExtension();

            // Perform bake and use that file for rendering
            if (canBake())
            {
                for (auto& doctoBake : options.bakeSettings)
                {
                    mx::FilePath outputBakeFile = file;
                    if (doctoBake.bakeFile == outputBakeFile.asString())
                    {
                        outputBakeFile.removeExtension();
                        outputBakeFile = outputPath / (outputBakeFile.asString() + "_baked.mtlx");
                        runBake(doc, imageSearchPath, outputBakeFile, doctoBake, log);
                        break;
                    }
                }
            }

            renderableSearchTimer.startTimer();
            std::vector<mx::TypedElementPtr> elements;
            try
            {
                mx::findRenderableElements(doc, elements);
            }
            catch (mx::Exception& e)
            {
                docValidLog << e.what() << std::endl;
                WARN("Shader generation error in " + filename.asString() + ": " + e.what());
            }
            renderableSearchTimer.endTimer();

            for (const auto& element : elements)
            {
                const mx::string elementName = mx::createValidName(mx::replaceSubstrings(element->getNamePath(), pathMap));

                auto it = std::find_if(options.wedgeSettings.begin(), options.wedgeSettings.end(),
                    [&file] (const GenShaderUtil::TestSuiteOptions::WedgeSetting& setting) {
                        return (file.asString() == setting.wedgeFile);
                    });

                const bool performWedge = (it != options.wedgeSettings.end()) ? true : false;
                if (!performWedge)
                {
                    runRenderer(elementName, element, context, doc, log, options, profileTimes, imageSearchPath, outputPath, nullptr);
                }
                else
                {
                    for (auto &wedgesetting: options.wedgeSettings)
                    {
                        mx::ImageVec imageVec;

                        const std::string& wedgeFile = wedgesetting.wedgeFile;
                        if (wedgeFile != file.asString())
                        {
                            continue;
                        }

                        // Make this a utility
                        std::string parameterPath = wedgesetting.parameter;
                        mx::ElementPtr uniformElement = doc->getDescendant(parameterPath);
                        if (!uniformElement)
                        {
                            std::string nodePath = mx::parentNamePath(parameterPath);
                            mx::ElementPtr uniformParent = doc->getDescendant(nodePath);
                            if (uniformParent)
                            {
                                mx::NodePtr uniformNode = uniformParent->asA<mx::Node>();
                                if (uniformNode)
                                {
                                    mx::StringVec pathVec = mx::splitNamePath(parameterPath);
                                    uniformNode->addInputFromNodeDef(pathVec[pathVec.size() - 1]);
                                }
                            }
                        }
                        uniformElement = doc->getDescendant(parameterPath);
                        mx::ValueElementPtr valueElement = uniformElement ? uniformElement->asA<mx::ValueElement>() : nullptr;
                        if (!valueElement)
                        {
                            continue;
                        }

                        mx::ValuePtr origPropertyValue(valueElement ? valueElement->getValue() : nullptr);
                        mx::ValuePtr newValue = valueElement->getValue();

                        float wedgePropertyMin = wedgesetting.range[0];
                        float wedgePropertyMax = wedgesetting.range[1];
                        int wedgeImageCount = std::max(wedgesetting.steps, 2);

                        float wedgePropertyStep = (wedgePropertyMax - wedgePropertyMin) / (wedgeImageCount - 1);
                        for (int w = 0; w < wedgeImageCount; w++)
                        {
                            bool setValue = false;
                            float propertyValue = (w == wedgeImageCount - 1) ? wedgePropertyMax : wedgePropertyMin + wedgePropertyStep * w;
                            if (origPropertyValue->isA<int>())
                            {
                                valueElement->setValue(static_cast<int>(propertyValue));
                                setValue = true;
                            }
                            else if (origPropertyValue->isA<float>())
                            {
                                valueElement->setValue(propertyValue);
                                setValue = true;
                            }
                            else if (origPropertyValue->isA<mx::Vector2>())
                            {
                                mx::Vector2 val(propertyValue, propertyValue);
                                valueElement->setValue(val);
                                setValue = true;
                            }
                            else if (origPropertyValue->isA<mx::Color3>() ||
                                origPropertyValue->isA<mx::Vector3>())
                            {
                                mx::Vector3 val(propertyValue, propertyValue, propertyValue);
                                valueElement->setValue(val);
                                setValue = true;
                            }
                            else if (origPropertyValue->isA<mx::Color4>() ||
                                origPropertyValue->isA<mx::Vector4>())
                            {
                                mx::Vector4 val(propertyValue, propertyValue, propertyValue, origPropertyValue->isA<mx::Color4>() ? 1.0f : propertyValue);
                                valueElement->setValue(val);
                                setValue = true;
                            }

                            if (setValue)
                            {
                                runRenderer(elementName, element, context, doc, log, options, profileTimes, imageSearchPath, outputPath, &imageVec);
                            }
                        }

                        if (!imageVec.empty())
                        {
                            mx::ImagePtr wedgeImage = mx::createImageStrip(imageVec);
                            if (wedgeImage)
                            {
                                std::string wedgeFileName = mx::createValidName(mx::replaceSubstrings(parameterPath, pathMap));
                                wedgeFileName += "_" + _shaderGenerator->getTarget() + ".bmp";
                                mx::FilePath wedgePath = outputPath / wedgeFileName;
                                saveImage(wedgePath, wedgeImage, true);
                            }
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
