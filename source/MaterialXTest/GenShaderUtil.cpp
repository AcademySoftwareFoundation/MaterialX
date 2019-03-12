//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/GenShaderUtil.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

namespace mx = MaterialX;

namespace GenShaderUtil
{

void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc)
{
    mx::DocumentPtr libDoc = mx::createDocument();
    mx::readFromXmlFile(libDoc, file);
    libDoc->setSourceUri(file);
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;
    doc->importLibrary(libDoc, &copyOptions);
}

void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
                   const mx::StringSet* excludeFiles)
{
    const std::string MTLX_EXTENSION("mtlx");
    for (const std::string& library : libraryNames)
    {
        mx::StringVec librarySubPaths;
        mx::FilePath libraryPath = searchPath / library;
        mx::getSubDirectories(libraryPath, librarySubPaths);

        for (auto path : librarySubPaths)
        {
            mx::StringVec filenames;
            mx::getFilesInDirectory(path, filenames, MTLX_EXTENSION);

            for (const std::string& filename : filenames)
            {
                if (excludeFiles && excludeFiles->count(filename))
                {
                    continue;
                }
                loadLibrary(mx::FilePath(path)/ filename, doc);
            }
        }
    }
    REQUIRE(doc->getNodeDefs().size() > 0);
}

bool getShaderSource(mx::GenContext& context,
                    const mx::ImplementationPtr implementation,
                    mx::FilePath& sourcePath,
                    mx::FilePath& resolvedPath,
                    std::string& sourceContents)
{
    if (implementation)
    {
        sourcePath = implementation->getFile();
        resolvedPath = context.resolveSourceFile(sourcePath);
        return mx::readFile(resolvedPath.asString(), sourceContents);
    }
    return false;
}

// Check that implementations exist for all nodedefs supported per generator
void checkImplementations(mx::GenContext& context,
                          const mx::StringSet& generatorSkipNodeTypes,
                          const mx::StringSet& generatorSkipNodeDefs)
{
    mx::DocumentPtr doc = mx::createDocument();

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);

    std::string generatorId = shadergen.getLanguage() + "_" + shadergen.getTarget();
    std::string fileName = generatorId + "_implementation_check.txt";

    std::filebuf implDumpBuffer;
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    context.registerSourceCodeSearchPath(searchPath);

    const std::string& language = shadergen.getLanguage();
    const std::string& target = shadergen.getTarget();

    // Node types to explicitly skip temporarily.
    mx::StringSet skipNodeTypes =
    {
        "ambientocclusion",
        "arrayappend",
        "curveadjust",
    };
    skipNodeTypes.insert(generatorSkipNodeTypes.begin(), generatorSkipNodeTypes.end());

    // Explicit set of node defs to skip temporarily
    mx::StringSet skipNodeDefs =
    {
        "ND_add_displacementshader",
        "ND_add_volumeshader",
        "ND_multiply_displacementshaderF",
        "ND_multiply_displacementshaderV",
        "ND_multiply_volumeshaderF",
        "ND_multiply_volumeshaderC",
        "ND_mix_displacementshader",
        "ND_mix_volumeshader"
    };
    skipNodeDefs.insert(generatorSkipNodeDefs.begin(), generatorSkipNodeDefs.end());

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Scanning language: " << language << ". Target: " << target << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
    implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    for (auto impl : impls)
    {
        if (language == impl->getLanguage())
        {
            std::string msg("Impl: ");
            msg += impl->getName();
            std::string targetName = impl->getTarget();
            if (targetName.size())
            {
                msg += ", target: " + targetName;
            }
            else
            {
                msg += ", target: NONE ";
            }
            mx::NodeDefPtr nodedef = impl->getNodeDef();
            if (!nodedef)
            {
                std::string nodedefName = impl->getNodeDefString();
                msg += ". Does NOT have a nodedef with name: " + nodedefName;
            }
            implDumpStream << msg << std::endl;
        }
    }

    std::string nodeDefNode;
    std::string nodeDefType;
    unsigned int count = 0;
    unsigned int missing = 0;
    unsigned int skipped = 0;
    std::string missing_str;
    std::string found_str;

    // Scan through every nodedef defined
    for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
    {
        count++;

        const std::string& nodeDefName = nodeDef->getName();
        const std::string& nodeName = nodeDef->getNodeString();

        if (skipNodeTypes.count(nodeName))
        {
            found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
            skipped++;
            continue;
        }
        if (skipNodeDefs.count(nodeDefName))
        {
            found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
            skipped++;
            continue;
        }

        if (!requiresImplementation(nodeDef))
        {
            found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
            continue;
        }

        mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
        if (!inter)
        {
            missing++;
            missing_str += "Missing nodeDef implementation: " + nodeDefName + ", Node: " + nodeName + ".\n";

            std::vector<mx::InterfaceElementPtr> inters = doc->getMatchingImplementations(nodeDefName);
            for (auto inter2 : inters)
            {
                mx::ImplementationPtr impl = inter2->asA<mx::Implementation>();
                if (impl)
                {
                    std::string msg("\t Cached Impl: ");
                    msg += impl->getName();
                    msg += ", nodedef: " + impl->getNodeDefString();
                    msg += ", target: " + impl->getTarget();
                    msg += ", language: " + impl->getLanguage();
                    missing_str += msg + ".\n";
                }
            }

            for (auto childImpl : impls)
            {
                if (childImpl->getNodeDefString() == nodeDefName)
                {
                    std::string msg("\t Doc Impl: ");
                    msg += childImpl->getName();
                    msg += ", nodedef: " + childImpl->getNodeDefString();
                    msg += ", target: " + childImpl->getTarget();
                    msg += ", language: " + childImpl->getLanguage();
                    missing_str += msg + ".\n";
                }
            }

        }
        else
        {
            mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
            if (impl)
            {
                // Test if the generator has an interal implementation first
                if (shadergen.implementationRegistered(impl->getName()))
                {
                    found_str += "Found generator impl for nodedef: " + nodeDefName + ", Node: "
                        + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                }

                // Check for an implementation explicitly stored
                else
                {
                    mx::FilePath sourcePath, resolvedPath;
                    std::string contents;
                    if (!getShaderSource(context, impl, sourcePath, resolvedPath, contents))
                    {
                        missing++;
                        missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                            + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                    }
                    else
                    {
                        found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                            + nodeName + +". Impl: " + impl->getName() + "Path: " + resolvedPath.asString() + ".\n";
                    }
                }
            }
            else
            {
                mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Graph Impl: " + graph->getName();
                mx::InterfaceElementPtr graphNodeDefImpl = graph->getImplementation();
                if (graphNodeDefImpl)
                {
                    found_str += ". Graph Nodedef Impl: " + graphNodeDefImpl->getName();
                }
                found_str += ".\n";
            }
        }
    }

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs. Skipped: " << skipped << std::endl;
    implDumpStream << missing_str << std::endl;
    implDumpStream << found_str << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    // Should have 0 missing including skipped
    REQUIRE(missing == 0);
    REQUIRE(skipped == 38);

    implDumpBuffer.close();
}

void testUniqueNames(mx::GenContext& context, const std::string& stage)
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "unique_names";

    // Generate a shader with an internal node having the same name as the shader,
    // which will result in a name conflict between the shader output and the
    // internal node output
    const std::string shaderName = "unique_names";
    const std::string nodeName = shaderName;

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");
    mx::NodePtr node1 = nodeGraph->addNode("noise2d", nodeName, "color3");

    output1->setConnectedNode(node1);

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    // Set the output to a restricted name
    const std::string& outputQualifier = shadergen.getSyntax().getOutputQualifier();
    output1->setName(outputQualifier);

    mx::GenOptions options;
    mx::ShaderPtr shader = shadergen.generate(shaderName, output1, context);
    REQUIRE(shader != nullptr);
    REQUIRE(shader->getSourceCode(stage).length() > 0);

    // Make sure the output and internal node output has their variable names set
    const mx::ShaderGraphOutputSocket* sgOutputSocket = shader->getGraph().getOutputSocket();
    REQUIRE(sgOutputSocket->getVariable() != outputQualifier);
    const mx::ShaderNode* sgNode1 = shader->getGraph().getNode(node1->getName());
    REQUIRE(sgNode1->getOutput()->getVariable() == "unique_names_out");
}

bool ShaderGeneratorTester::generateCode(mx::GenContext& context, const std::string& shaderName, mx::TypedElementPtr element,
                                         std::ostream& log, mx::StringVec testStages, mx::StringVec& sourceCode)
{
    mx::ShaderPtr shader = nullptr;
    try
    {
        shader = context.getShaderGenerator().generate(shaderName, element, context);
    }
    catch (mx::ExceptionShaderGenError& e)
    {
        log << ">> Code generation failure: " << e.what() << "\n";
        shader = nullptr;
    }
    CHECK(shader);
    if (!shader)
    {
        log << ">> Failed to generate shader for element: " << element->getNamePath() << std::endl;
        return false;
    }
    
    bool stageFailed = false;
    for (auto stage : testStages)
    {
        const std::string& code = shader->getSourceCode(stage);
        sourceCode.push_back(code);
        bool noSource = code.empty();
        CHECK(!noSource);
        if (noSource)
        {
            log << ">> Failed to generate source code for stage: " << stage << std::endl;
            stageFailed = true;
        }
    }
    return !stageFailed;
}

void ShaderGeneratorTester::addColorManagement()
{
    if (!_colorManagementSystem && _shaderGenerator)
    {
        const std::string language = _shaderGenerator->getLanguage();
        _colorManagementSystem = mx::DefaultColorManagementSystem::create(language);
        if (!_colorManagementSystem)
        {
            _logFile << ">> Failed to create color management system for language: " << language << std::endl;
        }
        else
        {
            _shaderGenerator->setColorManagementSystem(_colorManagementSystem);
            _colorManagementSystem->loadLibrary(_dependLib);
        }
    }
}

void ShaderGeneratorTester::setupDependentLibraries()
{
    _dependLib = mx::createDocument();

    // Load the standard libraries.
    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    GenShaderUtil::loadLibraries(libraries, _libSearchPath, _dependLib, &_excludeLibraryFiles);

    // Load the standard_surface definition since it's used in the test suite.
    GenShaderUtil::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples/BxDF/standard_surface.mtlx"), _dependLib);
}

void ShaderGeneratorTester::setExcludeLibraryFiles()
{
    // Base class adds no additional library files to exclude
}

void ShaderGeneratorTester::addSkipFiles()
{
    _skipFiles.insert("_options.mtlx");
    _skipFiles.insert("light_rig.mtlx");
    _skipFiles.insert("lightcompoundtest_ng.mtlx");
    _skipFiles.insert("lightcompoundtest.mtlx");
}

void ShaderGeneratorTester::addSkipNodeDefs()
{
}

void ShaderGeneratorTester::mapNodeDefToIdentiers(const std::vector<mx::NodePtr>& nodes,
                                                  std::unordered_map<std::string, unsigned int>& ids)
{
    unsigned int id = 1;
    for (auto node : nodes)
    {
        auto nodedef = node->getNodeDef();
        if (nodedef)
        {
            const std::string& name = nodedef->getName();
            if (!ids.count(name))
            {
                ids[name] = id++;
            }
        }
    }
}

void ShaderGeneratorTester::findLights(mx::DocumentPtr doc, std::vector<mx::NodePtr>& lights)
{
    lights.clear();
    for (mx::NodePtr node : doc->getNodes())
    {
        const mx::TypeDesc* type = mx::TypeDesc::get(node->getType());
        if (type == mx::Type::LIGHTSHADER)
        {
            lights.push_back(node);
        }
    }
}

void ShaderGeneratorTester::registerLights(mx::DocumentPtr doc, const std::vector<mx::NodePtr>& lights,
                                           mx::GenContext& context)
{
    // Clear context light user data which is set when bindLightShader() 
    // is called. This is necessary in case the light types have already been
    // registered.
    mx::HwShaderGenerator::unbindLightShaders(context);

    if (!lights.empty())
    {
        // Create a list of unique nodedefs and ids for them
        mapNodeDefToIdentiers(lights, _lightIdentifierMap);
        for (auto id : _lightIdentifierMap)
        {
            mx::NodeDefPtr nodeDef = doc->getNodeDef(id.first);
            if (nodeDef)
            {
                mx::HwShaderGenerator::bindLightShader(*nodeDef, id.second, context);
            }
        }
    }

    // Clamp the number of light sources to the number registered
    unsigned int lightSourceCount = static_cast<unsigned int>(lights.size());
    context.getOptions().hwMaxActiveLightSources = lightSourceCount;
}

void ShaderGeneratorTester::testGeneration(const mx::GenOptions& generateOptions)
{
    // Start logging
    _logFile.open(_logFilePath);

    // Generator setup
    createGenerator();

    // Dependent library setup
    setExcludeLibraryFiles();
    setupDependentLibraries();
    addColorManagement();

    // Test suite setup
    addSkipFiles();

    // Generation setup
    setTestStages();

    // Load in all documents to test
    mx::loadDocuments(_testRootPath, _skipFiles, _documents, _documentPaths, nullptr);

    // Scan each document for renderable elements and check code generation
    //
    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    // Add nodedefs to skip when testing
    addSkipNodeDefs();

    // Create our context
    mx::GenContext context(_shaderGenerator);
    context.getOptions() = generateOptions;
    context.registerSourceCodeSearchPath(_srcSearchPath);

    mx::XmlReadOptions importOptions;
    importOptions.skipDuplicateElements = true;
    size_t documentIndex = 0;
    for (auto doc : _documents)
    {
        // Add in dependent libraries
        doc->importLibrary(_dependLib, &importOptions);

        // Find and register lights
        findLights(doc, _lights);
        registerLights(doc, _lights, context);

        // Find elements to render in the document
        std::vector<mx::TypedElementPtr> elements;
        try
        {
            mx::findRenderableElements(doc, elements);
        }
        catch (mx::ExceptionShaderGenError& e)
        {
            _logFile << "Renderables search errors: " << e.what() << std::endl;
        }

        if (!elements.empty())
        {
            _logFile << "MTLX Filename :" << _documentPaths[documentIndex] << ". Elements tested: "
                << std::to_string(elements.size()) << std::endl;
            documentIndex++;
        }

        // Perform document validation
        std::string docErrors;
        bool documentIsValid = doc->validate(&docErrors);
        CHECK(documentIsValid);
        if (!documentIsValid)
        {
            _logFile << ">> Validation errors: " << docErrors << std::endl;
        }

        // Traverse the renderable documents and run validation the validation step
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

            // Allow to skip nodedefs to test if specified
            const std::string nodeDefName = nodeDef->getName();
            if (_skipNodeDefs.count(nodeDefName))
            {
                _logFile << ">> Skipped testing nodedef: " << nodeDefName << std::endl;
                continue;
            }

            const std::string namePath(element->getNamePath());
            CHECK(nodeDef);
            if (nodeDef)
            {
                mx::string elementName = mx::replaceSubstrings(namePath, pathMap);
                elementName = mx::createValidName(elementName);

                mx::InterfaceElementPtr impl = nodeDef->getImplementation(_shaderGenerator->getTarget(), _shaderGenerator->getLanguage());
                CHECK(impl);
                if (impl)
                {
                    _logFile << "------------ Run validation with element: " << namePath << "------------" << std::endl;
                    mx::StringVec sourceCode;
                    bool generatedCode = generateCode(context, elementName, element, _logFile, _testStages, sourceCode);
                    CHECK(generatedCode);
                }
                else
                {
                    _logFile << ">> Failed to find implementation for nodedef: " << nodeDefName << std::endl;
                }
            }
            else
            {
                _logFile << ">> Failed to find nodedef for: " << namePath << std::endl;
            }
        }
    }

    // End logging
    if (_logFile.is_open())
    {
        _logFile.close();
    }
}

} //namespace GenShaderUtil

