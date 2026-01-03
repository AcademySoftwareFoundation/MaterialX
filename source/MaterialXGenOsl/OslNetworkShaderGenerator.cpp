//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslNetworkShaderGenerator.h>
#include <MaterialXGenOsl/OslNetworkSyntax.h>
#include <MaterialXGenOsl/Nodes/OsoNode.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGraph.h>

#include <MaterialXCore/Document.h>

#ifdef USE_OSLCOMP
    #include <OSL/oslcomp.h>
#endif

#include <iostream>
#include <fstream>

MATERIALX_NAMESPACE_BEGIN

class OslNetworkStructTypeSyntax;
using OslNetworkStructTypeSyntaxPtr = shared_ptr<OslNetworkStructTypeSyntax>;

const string OslNetworkShaderGenerator::TARGET = "genoslnetwork";

namespace
{

ShaderNodeImplPtr createNewImplementation(const NodeDef& nodedef, TypeSystemPtr typeSystem, const Syntax& syntax, GenContext& context)
{
    OslSyntaxPtr oslSyntax = std::static_pointer_cast<OslSyntax>(OslSyntax::create(typeSystem));

    // we need to populate the OslSyntax with the any custom syntaxes present in the OslNetworkSyntax
    for (const auto& [typeDesc, syntaxPtr] : syntax.getCustomTypeSyntaxes())
    {
        OslNetworkStructTypeSyntaxPtr structSyntax = std::dynamic_pointer_cast<OslNetworkStructTypeSyntax>(syntaxPtr);
        if (!structSyntax)
            continue;

        StructTypeSyntaxPtr newStructSyntax = oslSyntax->createStructSyntax(structSyntax->getName(),
                                                                            structSyntax->getDefaultValue(false),
                                                                            structSyntax->getDefaultValue(true),
                                                                            structSyntax->getTypeAlias(),
                                                                            structSyntax->getTypeDefinition());
        oslSyntax->registerTypeSyntax(typeDesc, newStructSyntax, true);
    }

    // Context already has the source code search path
    const FileSearchPath& sourceCodeSearchPath = context.getSourceCodeSearchPath();

    OslShaderGeneratorPtr oslShaderGen = std::static_pointer_cast<OslShaderGenerator>(OslShaderGenerator::create(typeSystem, oslSyntax));

    // Create a new temporary GenContext to avoid clashing with the parent context
    GenContext localContext(oslShaderGen);
    localContext.registerSourceCodeSearchPath(sourceCodeSearchPath);
    localContext.getOptions() = context.getOptions();
    localContext.getOptions().oslImplicitSurfaceShaderConversion = false;
    localContext.getOptions().oslConnectCiWrapper = false;

    ConstNodeDefPtr nodeDefPtr = std::static_pointer_cast<const NodeDef>(nodedef.getSelf());

    ShaderPtr oslShader = OslNetworkShaderGenerator::generateOSLShader(nodeDefPtr, oslShaderGen, localContext);
    if (!oslShader)
    {
        throw ExceptionShaderGenError("Failed to generate OSL Shader for '" + nodedef.getName() + "'");
        return nullptr;
    }

    OslNetworkShaderGenerator::OslCompileOptions options;
    options.oslIncludePath = sourceCodeSearchPath;
    FilePath oslStdIncludePath = FilePath(MATERIALX_OSL_INCLUDE_PATH);
    if (oslStdIncludePath.exists())
    {
        options.oslIncludePath.append(oslStdIncludePath);
    }
#ifdef USE_OSLCOMP
    options.writeSourceToDisk = true;
    options.useOslComp = true;
#else
    options.writeSourceToDisk = false;
    FilePath oslCompilerPath = FilePath(MATERIALX_OSL_BINARY_OSLC);
    if (!oslCompilerPath.exists())
    {
        throw ExceptionShaderGenError("Dynamic OslNode generation not supported if oslc not provided");
    }
    options.oslCompilerPath = oslCompilerPath;
#endif

    FilePath osoPath = context.getOptions().oslTempOsoPath;
    if (osoPath.isEmpty())
    {
        osoPath = FilePath::createTemporaryDirectory();
    }

    FilePath oslFilePath = osoPath / FilePath(oslShader->getName() + ".osl");

    OslNetworkShaderGenerator::compileOSL(oslShader->getSourceCode(), oslFilePath, options);

    ShaderNodeImplPtr osoNodeImpl = OsoNode::create(oslShader->getName(), osoPath.asString());

    addPortImplementationNames(nodedef.getImplementation(OslShaderGenerator::TARGET), *osoNodeImpl);

    return osoNodeImpl;
}

} // namespace

//
// OslNetworkShaderGenerator methods
//

OslNetworkShaderGenerator::OslNetworkShaderGenerator(TypeSystemPtr typeSystem) :
    ShaderGenerator(typeSystem, OslNetworkSyntax::create(typeSystem))
{
}

ShaderNodeImplPtr OslNetworkShaderGenerator::getImplementation(const NodeDef& nodedef, GenContext& context) const
{
    ShaderNodeImplPtr nodeImpl = ShaderGenerator::getImplementation(nodedef, context);
    if (nodeImpl)
        return nodeImpl;

    return createNewImplementation(nodedef, getTypeSystem(), getSyntax(), context);
}

ShaderNodeImplPtr OslNetworkShaderGenerator::createShaderNodeImplForImplementation(const Implementation& /* implElement */) const
{
    return OsoNode::create();
}

static string paramString(const string& paramType, const string& paramName, const string& paramValue)
{
    return "param " + paramType + " " + paramName + " " + paramValue + " ;";
}

static string connectString(const string& fromNode, const string& fromName, const string& toNode, const string& toName)
{
    return "connect " + fromNode + "." + fromName + " " + toNode + "." + toName + " ;";
}

ShaderPtr OslNetworkShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);
    ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    if (context.getOptions().oslConnectCiWrapper)
    {
        OslShaderGenerator::addSetCiTerminalNode(graph, element->getDocument(), getTypeSystem(), context);
    }

    graph.flattenGraph();
    graph.topologicalSort();

    ConstDocumentPtr document = element->getDocument();

    ShaderOutput* lastOutput = nullptr;
    std::vector<string> connections;

    std::set<std::string> osoPaths;

    // Walk the node graph, emitting shaders and param declarations.
    for (auto&& node : graph.getNodes())
    {
        const string& nodeName = node->getName();

        for (auto&& input : node->getInputs())
        {
            string inputName = node->getPortName(input->getName());

            ValuePtr inputValue = input->getValue();
            TypeDesc inputType = input->getType();

            const ShaderOutput* connection = input->getConnection();
            if (!connection || connection->getNode() == &graph)
            {
                bool hasAuthoredValue = input->hasAuthoredValue();

                if (connection && connection->getNode() == &graph)
                {
                    if (connection->getValue()) // why do this check?
                    {
                        inputValue = connection->getValue();
                        hasAuthoredValue = connection->hasAuthoredValue();
                        inputType = connection->getType();
                    }
                }

                if (!inputValue)
                    continue;

                if (!hasAuthoredValue)
                    continue;

                if (input->getName() == "backsurfaceshader" || input->getName() == "displacementshader")
                    continue; // FIXME: these aren't getting pruned by hasAuthoredValue

                const TypeSyntax& typeSyntax = _syntax->getTypeSyntax(inputType);

                const TypeSyntax* typeSyntaxPtr = &typeSyntax;

                const OslNetworkSyntaxEmit* oslTypeSyntax = dynamic_cast<const OslNetworkSyntaxEmit*>(typeSyntaxPtr);

                for (const auto& part : oslTypeSyntax->getEmitParamParts(inputName, inputType, *inputValue))
                {
                    emitLine(paramString(part.typeName, part.paramName, part.paramValue), stage, false);
                }
            }
            else
            {
                string connName = connection->getNode()->getPortName(connection->getName());

                string fromNodeName = connection->getNode()->getName();
                string connect = connectString(fromNodeName, connName, nodeName, inputName);
                // Save connect emits for the end, because they can't come
                // before both connected shaders have been declared.
                connections.push_back(connect);
            }
        }

        // Keep track of the root output, so we can connect it to our setCi node
        lastOutput = node->getOutput(0);

        const ShaderNodeImpl& impl = node->getImplementation();
        const OsoNode& osoNodeImpl = dynamic_cast<const OsoNode&>(impl);

        const string osoPath = osoNodeImpl.getOsoPath();
        osoPaths.insert(osoPath);

        emitLine("shader " + osoNodeImpl.getOsoName() + " " + nodeName + " ;", stage, false);
    }

    if (!lastOutput)
    {
        printf("Invalid shader\n");
        return nullptr;
    }

    for (auto&& connect : connections)
    {
        emitLine(connect, stage, false);
    }

    // From our set of required oso paths, build the path string that oslc will need.
    string osoPathStr;
    string separator = "";
    for (const auto& osoPath : osoPaths)
    {
        auto fullOsoPath = context.resolveSourceFile(osoPath, "");
        auto fullOsoPathStr = fullOsoPath.asString();

        osoPathStr += separator + fullOsoPathStr;
        separator = ":";
    }

    shader->setAttribute("osoPath", Value::createValue<string>(osoPathStr));

    return shader;
}

ShaderPtr OslNetworkShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(Stage::PIXEL, *shader);
    stage->createUniformBlock(OSLNetwork::UNIFORMS);
    stage->createInputBlock(OSLNetwork::INPUTS);
    stage->createOutputBlock(OSLNetwork::OUTPUTS);

    // Create shader variables for all nodes that need this.
    createVariables(graph, context, *shader);

    // Create uniforms for the published graph interface.
    VariableBlock& uniforms = stage->getUniformBlock(OSLNetwork::UNIFORMS);
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->getConnections().size() && graph->isEditable(*inputSocket))
        {
            uniforms.add(inputSocket->getSelf());
        }
    }

    // Create outputs from the graph interface.
    VariableBlock& outputs = stage->getOutputBlock(OSLNetwork::OUTPUTS);
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs.add(outputSocket->getSelf());
    }

    return shader;
}

bool OslNetworkShaderGenerator::compileOSL(const std::string& oslSourceCode, const FilePath& oslFilePath, const OslCompileOptions& options)
{
    if (!options.useOslComp && !options.writeSourceToDisk)
    {
        throw ExceptionShaderGenError("If OslComp library is not being used the source must be written to disk");
    }

    if (options.createDirectories)
    {
        oslFilePath.getParentPath().createDirectory(true);
    }

    if (!oslFilePath.getParentPath().isDirectory())
    {
        throw ExceptionShaderGenError("Cannot compile OSL shader, destination directory does not exist - '"+oslFilePath.getParentPath().asString()+"'");
    }

    if (options.writeSourceToDisk)
    {
        std::ofstream oslFile;
        oslFile.open(oslFilePath);
        oslFile << oslSourceCode;
        oslFile.close();
    }

    FilePath osoFilePath = oslFilePath;
    osoFilePath.removeExtension();
    osoFilePath.addExtension("oso");

    // build up a vector of compiler arguments that will be
    // used in both compiler modes.
    std::vector<std::string> oslCompilerArgs;
    oslCompilerArgs.emplace_back("-o");
    oslCompilerArgs.emplace_back(osoFilePath);
    for (FilePath p : options.oslIncludePath)
    {
        oslCompilerArgs.emplace_back("-I" + p.asString() + "");
    }

#ifdef USE_OSLCOMP
    if (options.useOslComp)
    {
        // Use OSL::oslcomp to compile the shader - this is significantly faster than using the system
        // call to involke the `oslc` command line tool
        OIIO::ErrorHandler errorHandler;
        ::OSL::OSLCompiler compiler(&errorHandler);
        if (options.writeSourceToDisk)
        {
            // Compile from the source file
            compiler.compile(oslFilePath.asString(), oslCompilerArgs);
        }
        else
        {
            // Compile directly from the string buffer
            std::string osoBuffer;
            compiler.compile_buffer(oslSourceCode, osoBuffer, oslCompilerArgs, std::string_view(), oslFilePath.asString());

            std::ofstream osoFile;
            osoFile.open(osoFilePath.asString());
            osoFile << osoBuffer;
            osoFile.close();
        }
    }
    else
#endif
    {
        // If no command and include path specified then skip checking.
        if (options.oslCompilerPath.isEmpty())
        {
            throw ExceptionShaderGenError("OSL compiler path missing");
        }
        if (!options.oslCompilerPath.exists())
        {
            throw ExceptionShaderGenError("OSL compiler doesn't exist at '" + options.oslCompilerPath.asString() + "'");
        }

        // Use a known error file name to check
        std::string errorFile(osoFilePath.asString() + "_compile_errors.txt");
        const std::string redirectString(" 2>&1");

        // Run the command and get back the result. If non-empty string throw exception with error
        std::string command = options.oslCompilerPath.asString() + " -q " + oslFilePath.asString();
        for (const auto& arg : oslCompilerArgs)
        {
            command += " " + arg;
        }
        command += " > " + errorFile + redirectString;

        int returnValue = std::system(command.c_str());

        std::ifstream errorStream(errorFile);
        std::string result;
        result.assign(std::istreambuf_iterator<char>(errorStream),
                      std::istreambuf_iterator<char>());

        if (!result.empty())
        {
            StringVec errors;
            errors.push_back("Command string: " + command);
            errors.push_back("Command return code: " + std::to_string(returnValue));
            errors.push_back("Shader failed to compile:");
            errors.push_back(result);
            throw ExceptionOslCompileError("OSL compilation error", errors);
        }
    }

    return true;
}

ShaderPtr OslNetworkShaderGenerator::generateOSLShader(ConstNodeDefPtr nodeDef, OslShaderGeneratorPtr generator, GenContext& context, const string& osoNameStrategy)
{
    if (!generator)
    {
        // raise error
        return nullptr;
    }

    // Determine whether or not there's a valid implementation of the current `NodeDef` for the type associated
    // to our OSL shader generator, i.e. OSL, and if not, skip it.
    InterfaceElementPtr nodeImpl = nodeDef->getImplementation(generator->getTarget(), false);

    if (!nodeImpl)
    {
        std::cout << "The following `NodeDef` does not provide a valid OSL implementation, "
                     "and will be skipped: "
                  << nodeDef->getName() << std::endl;

        return nullptr;
    }

    // Intention is here is to name the new node the same as the genosl implementation name
    // but replacing "_genosl" with "_genoslnetwork"
    std::string nodeName;
    if (osoNameStrategy == "implementation")
    {
        // Name the node the same as the implementation with _genoslnetwork added as a suffix.
        // NOTE : If the implementation currently has _genosl as a suffix then we remove it.
        nodeName = nodeImpl->getName();
        nodeName = replaceSubstrings(nodeName, { { "_genosl", "" } });
        nodeName += "_genoslnetwork";
    }
    else
    {
        // Name the node the same as the node definition
        nodeName = nodeDef->getName();
    }

    // We shouldn't need to do this - but it doesn't hurt just to be safe.
    generator->getSyntax().makeValidName(nodeName);

    // Codegen the `Node` to OSL.
    return generator->generate(nodeName, std::const_pointer_cast<NodeDef>(nodeDef), context);
}

namespace OSLNetwork
{

// Identifiers for OSL variable blocks
const string UNIFORMS = "u";
const string INPUTS = "i";
const string OUTPUTS = "o";

} // namespace OSLNetwork

MATERIALX_NAMESPACE_END
