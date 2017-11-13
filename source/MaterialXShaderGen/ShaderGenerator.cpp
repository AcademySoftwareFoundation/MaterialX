#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/Implementations/SourceCode.h>
#include <MaterialXShaderGen/Implementations/Compound.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

FileSearchPath ShaderGenerator::_sourceCodeSearchPath;

ShaderGenerator::ShaderGenerator(SyntaxPtr syntax) 
    : _syntax(syntax) 
{
}

Shader::VDirection ShaderGenerator::getTargetVDirection() const
{
    // Default is to use vdirection up
    return Shader::VDirection::UP;
}

void ShaderGenerator::emitTypeDefs(Shader& shader)
{
    // Emit typedefs for all data types that needs it
    for (auto syntax : _syntax->getTypeSyntax())
    {
        if (syntax.typeDef.length())
        {
            shader.addLine(syntax.typeDef, false);
        }
    }
    shader.newLine();
}

void ShaderGenerator::emitFunctions(Shader& shader)
{
    // Emit funtion definitions for all nodes
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        shader.addFunctionDefinition(node, *this);
    }
}

void ShaderGenerator::emitShaderBody(Shader &shader)
{
    const bool debugOutput = true;

    // Emit function calls for all nodes
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        // Omit node if it's only used inside a conditional branch
        if (node->referencedConditionally())
        {
            if (debugOutput)
            {
                std::stringstream str;
                str << "// Omitted node '" << node->getName() << "'. Only used in conditional node '" << node->getScopeInfo().conditionalNode->getName() << "'";
                shader.addLine(str.str(), false);
            }
            // Omit this node
            continue;
        }

        shader.addFunctionCall(node, *this);
    }
}

void ShaderGenerator::emitFinalOutput(Shader& shader) const
{
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutputSocket* outputSocket = graph->getOutputSocket();
    const string outputVariable = _syntax->getVariableName(outputSocket);

    string finalResult = _syntax->getVariableName(outputSocket->connection);

    if (outputSocket->channels != EMPTY_STRING)
    {
        finalResult = _syntax->getSwizzledVariable(finalResult, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
    }

    shader.addLine(outputVariable + " = " + finalResult);
}

void ShaderGenerator::emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader)
{
    const string initStr = (value ? _syntax->getValue(*value, true) : _syntax->getTypeDefault(type, true));
    shader.addStr(_syntax->getTypeName(type) + " " + name + (initStr.empty() ? "" : " = " + initStr));
}

void ShaderGenerator::emitInput(const SgInput* input, Shader &shader)
{
    if (input->connection)
    {
        string name = _syntax->getVariableName(input->connection);
        if (input->channels != EMPTY_STRING)
        {
            name = _syntax->getSwizzledVariable(name, input->type, input->connection->type, input->channels);
        }

        shader.addStr(name);

        return;
    }

    if (input->value)
    {
        shader.addStr(_syntax->getValue(*input->value));
    }
    else
    {
        shader.addStr(_syntax->getTypeDefault(input->type));
    }
}

void ShaderGenerator::emitOutput(const SgOutput* output, bool includeType, Shader& shader)
{
    string typeStr;
    if (includeType)
    {
        typeStr = _syntax->getTypeName(output->type) + " ";
    }
    shader.addStr(typeStr + _syntax->getVariableName(output));
}

bool ShaderGenerator::shouldPublish(const ValueElement* port, string& publicName) const
{
    ConstInputPtr inputElem = port->asA<Input>();
    const string& connectedNode = inputElem ? inputElem->getNodeName() : EMPTY_STRING;
    publicName = connectedNode.empty() ? port->getPublicName() : EMPTY_STRING;
    return !publicName.empty();
}

const Arguments* ShaderGenerator::getExtraArguments(const SgNode&) const
{
    return nullptr;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunc<SgImplementation> creator)
{
    _implFactory.registerClass(name, creator);
}

SgImplementationPtr ShaderGenerator::getImplementation(ElementPtr element)
{
    const string& name = element->getName();

    // Check if it's created already
    auto it = _cachedImpls.find(name);
    if (it != _cachedImpls.end())
    {
        return it->second;
    }

    SgImplementationPtr impl;
    if (element->isA<NodeGraph>())
    {
        // Use the graph based compound implementation
        impl = Compound::creator();
    }
    else
    {
        // Try creating a new in the factory
        impl = _implFactory.create(name);
        if (!impl)
        {
            // No implementation was registed for this name
            // Fall back to the data driven source code implementation
            impl = SourceCode::creator();
        }
    }

    impl->initialize(element, *this);
    _cachedImpls[name] = impl;

    return impl;
}

void ShaderGenerator::registerSourceCodeSearchPath(const FilePath& path)
{
    _sourceCodeSearchPath.append(path);
}

/// Resolve a file using the registered search paths.
FilePath ShaderGenerator::findSourceCode(const FilePath& filename)
{
    return _sourceCodeSearchPath.find(filename);
}

}
