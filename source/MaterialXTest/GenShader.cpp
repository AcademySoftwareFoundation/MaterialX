#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShader.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>

namespace mx = MaterialX;

static const std::string LIGHT_SHADER_TYPE = "lightshader";
static const std::string DIRECTIONAL_LIGHT = "directionallight";
static const std::string POINT_LIGHT = "pointlight";
static const std::string SPOT_LIGHT = "spotlight";
static const std::string LIGHT_COMPOUND = "lightcompoundtest";
static const size_t LIGHT_COMPOUND_ID = 42;

void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc)
{
    mx::DocumentPtr libDoc = mx::createDocument();
    mx::readFromXmlFile(libDoc, file);
    libDoc->setSourceUri(file);
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;
    doc->importLibrary(libDoc, &copyOptions);
}

// Loads all the MTLX files below a given library path
void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
                   const std::set<std::string>* excludeFiles = nullptr)
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

void loadExamples(const mx::StringVec& exampleNames, const mx::FilePath& examplesPath, const mx::FilePath searchPath, mx::DocumentPtr doc)
{
    try
    {
        for (const std::string& filename : exampleNames)
        {
            mx::FilePath file = examplesPath / filename;
            mx::readFromXmlFile(doc, file, searchPath);
        }
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
}


//
// Get source content, source path and resolved paths for
// an implementation
//
bool getShaderSource(mx::ShaderGeneratorPtr generator,
                    const mx::ImplementationPtr implementation,
                    mx::FilePath& sourcePath,
                    mx::FilePath& resolvedPath,
                    std::string& sourceContents)
{
    if (implementation)
    {
        sourcePath = implementation->getFile();
        resolvedPath = generator->findSourceCode(sourcePath);
        if (mx::readFile(resolvedPath.asString(), sourceContents))
        {
            return true;
        }
    }
    return false;
}

void registerLightType(mx::DocumentPtr doc, mx::HwShaderGenerator& shadergen, const mx::GenOptions& options)
{
    // Scan for lights
    std::vector<mx::NodePtr> lights;
    for (mx::NodePtr node : doc->getNodes())
    {
        if (node->getType() == LIGHT_SHADER_TYPE)
        {
            lights.push_back(node);
        }
    }
    if (!lights.empty())
    {
        // Create a list of unique nodedefs and ids for them
        std::unordered_map<std::string, unsigned int> identifiers;
        mx::mapNodeDefToIdentiers(lights, identifiers);
        for (auto id : identifiers)
        {
            mx::NodeDefPtr nodeDef = doc->getNodeDef(id.first);
            if (nodeDef)
            {
                shadergen.bindLightShader(*nodeDef, id.second, options);
            }
        }
    }
}

TEST_CASE("GenShader Valid Libraries", "[genshader]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);

    std::string validationErrors;
    bool valid = doc->validate(&validationErrors);
    if (!valid)
    {
        std::cout << validationErrors << std::endl;
    }
    REQUIRE(valid);
}

TEST_CASE("GenShader TypeDesc Check", "[genshader]")
{
    // Make sure the standard types are registered
    const mx::TypeDesc* floatType = mx::TypeDesc::get("float");
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    const mx::TypeDesc* integerType = mx::TypeDesc::get("integer");
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::TypeDesc::BASETYPE_INTEGER);
    const mx::TypeDesc* booleanType = mx::TypeDesc::get("boolean");
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::TypeDesc::BASETYPE_BOOLEAN);
    const mx::TypeDesc* color2Type = mx::TypeDesc::get("color2");
    REQUIRE(color2Type != nullptr);
    REQUIRE(color2Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color2Type->getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color2Type->isFloat2());
    const mx::TypeDesc* color3Type = mx::TypeDesc::get("color3");
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color3Type->isFloat3());
    const mx::TypeDesc* color4Type = mx::TypeDesc::get("color4");
    REQUIRE(color4Type != nullptr);
    REQUIRE(color4Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color4Type->getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color4Type->isFloat4());

    // Make sure we can register a new sutom type
    const mx::TypeDesc* fooType = mx::TypeDesc::registerType("foo", mx::TypeDesc::BASETYPE_FLOAT, mx::TypeDesc::SEMANTIC_COLOR, 5);
    REQUIRE(fooType != nullptr);

    // Make sure we can't use a name already take
    REQUIRE_THROWS(mx::TypeDesc::registerType("color3", mx::TypeDesc::BASETYPE_FLOAT));

    // Make sure we can't request an unknown type
    REQUIRE_THROWS(mx::TypeDesc::get("bar"));
}

TEST_CASE("OSL Reference Implementation Check", "[genshader]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    // Set source code search path
    mx::FileSearchPath sourceCodeSearchPath;
    sourceCodeSearchPath.append(searchPath);

    std::filebuf implDumpBuffer;
    std::string fileName = "osl_vanilla_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Scanning language: osl. Target: reference" << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    const std::string language("osl");
    const std::string target("");

    std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
    implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    for (auto impl : impls)
    {
        if (language == impl->getLanguage() && impl->getTarget().empty())
        {
            std::string msg("Impl: ");
            msg += impl->getName();

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
    std::string missing_str;
    std::string found_str;

    // Scan through every nodedef defined
    for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
    {
        count++;

        std::string nodeDefName = nodeDef->getName();
        std::string nodeName = nodeDef->getNodeString();
        if (!mx::requiresImplementation(nodeDef))
        {
            found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
            continue;
        }

        mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
        if (!inter)
        {
            missing++;
            missing_str += "Missing nodeDef implemenation: " + nodeDefName + ", Node: " + nodeName + ".\n";
        }
        else
        {
            mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
            if (impl)
            {
                // Scan for file and see if we can read in the contents
                std::string sourceContents;
                mx::FilePath sourcePath = impl->getFile();
                mx::FilePath resolvedPath = sourceCodeSearchPath.find(sourcePath);
                bool found = mx::readFile(resolvedPath.asString(), sourceContents);
                if (!found)
                {
                    missing++;
                    missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                        + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                }
                else
                {
                    found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + impl->getName() + ".\n";
                }
            }
            else
            {
                mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Impl: " + graph->getName() + ".\n";
            }
        }
    }

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs\n";
    implDumpStream << missing_str << std::endl;
    implDumpStream << found_str << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    implDumpBuffer.close();

    // To enable once this is true
    //REQUIRE(missing == 0);
}

// Check that implementations exist for all nodedefs supported per generator
void checkImplementations(mx::ShaderGeneratorPtr generator, std::set<std::string> generatorSkipNodeTypes, 
                          std::set<std::string> generatorSkipNodeDefs)
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);

    std::string generatorId = generator->getLanguage() + "_" + generator->getTarget();
    std::string fileName = generatorId + "_implementation_check.txt";

    std::filebuf implDumpBuffer;
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    generator->registerSourceCodeSearchPath(searchPath);

    const std::string& language = generator->getLanguage();
    const std::string& target = generator->getTarget();

    // Node types to explicitly skip temporarily.
    std::set<std::string> skipNodeTypes =
    {
        "ambientocclusion",
        "arrayappend",
        "curveadjust",
    };
    skipNodeTypes.insert(generatorSkipNodeTypes.begin(), generatorSkipNodeTypes.end());

    // Explicit set of node defs to skip temporarily
    std::set<std::string> skipNodeDefs =
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
                if (generator->implementationRegistered(impl->getName()))
                {
                    found_str += "Found generator impl for nodedef: " + nodeDefName + ", Node: "
                        + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                }

                // Check for an implementation explicitly stored
                else
                {
                    mx::FilePath sourcePath, resolvedPath;
                    std::string contents;
                    if (!getShaderSource(generator, impl, sourcePath, resolvedPath, contents))
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

void testUniqueNames(mx::ShaderGeneratorPtr shaderGenerator, const std::string& stage)
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

    // Set the output to a restricted name
    const std::string outputQualifier = shaderGenerator->getSyntax()->getOutputQualifier();
    output1->setName(outputQualifier);

    mx::GenOptions options;
    mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
    REQUIRE(shader != nullptr);
    REQUIRE(shader->getSourceCode(stage).length() > 0);

    // Make sure the output and internal node output has their variable names set
    mx::ShaderGraphOutputSocket* sgOutputSocket = shader->getGraph()->getOutputSocket();
    REQUIRE(sgOutputSocket->variable != outputQualifier);
    mx::ShaderNode* sgNode1 = shader->getGraph()->getNode(node1->getName());
    REQUIRE(sgNode1->getOutput()->variable == "unique_names_out");
}

