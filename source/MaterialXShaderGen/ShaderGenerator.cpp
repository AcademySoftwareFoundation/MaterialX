#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/SourceCode.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Compound.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

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

void ShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    // Emit funtion definitions for all nodes
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        shader.addFunctionDefinition(node, *this);
    }
}

void ShaderGenerator::emitFunctionCalls(Shader &shader)
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
    const string outputVariable = getVariableName(outputSocket);

    if (!outputSocket->connection)
    {
        // Early out for the rare case where the whole graph is just a single value
        shader.addLine(outputVariable + " = " + (outputSocket->value ? 
            _syntax->getValue(*outputSocket->value, outputSocket->type) : 
            _syntax->getTypeDefault(outputSocket->type)));
        return;
    }

    string finalResult = getVariableName(outputSocket->connection);
    if (outputSocket->channels != EMPTY_STRING)
    {
        finalResult = _syntax->getSwizzledVariable(finalResult, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
    }

    shader.addLine(outputVariable + " = " + finalResult);
}

void ShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    const string initStr = (uniform.value ? 
        _syntax->getValue(*uniform.value, uniform.type, true) : 
        _syntax->getTypeDefault(uniform.type, true));
    shader.addStr(_syntax->getTypeName(uniform.type) + " " + uniform.name + (initStr.empty() ? "" : " = " + initStr));
}

void ShaderGenerator::emitInput(const SgInput* input, Shader &shader) const
{
    if (input->connection)
    {
        string name = getVariableName(input->connection);

        if (input->channels != EMPTY_STRING)
        {
            name = _syntax->getSwizzledVariable(name, input->type, input->connection->type, input->channels);
        }

        shader.addStr(name);
    }
    else if (input->value)
    {
        shader.addStr(_syntax->getValue(*input->value, input->type));
    }
    else
    {
        shader.addStr(_syntax->getTypeDefault(input->type));
    }
}

void ShaderGenerator::emitOutput(const SgOutput* output, bool includeType, Shader& shader) const
{
    string typeStr;
    if (includeType)
    {
        typeStr = _syntax->getTypeName(output->type) + " ";
    }
    shader.addStr(typeStr + getVariableName(output));
}

string ShaderGenerator::getVariableName(const SgInput* input) const
{
    // TODO: Improve this to make sure we never get name collisions
    return input->node->getName() + "_" + input->name;
}

string ShaderGenerator::getVariableName(const SgOutput* output) const
{
    // TODO: Improve this to make sure we never get name collisions
    return output->node->getName() + "_" + output->name;
}

const Arguments* ShaderGenerator::getExtraArguments(const SgNode&) const
{
    return nullptr;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunc<SgImplementation> creator)
{
    _implFactory.registerClass(name, creator);
}

bool ShaderGenerator::implementationRegistered(const string& name) const
{
    return _implFactory.classRegistered(name);
}

SgImplementationPtr ShaderGenerator::getImplementation(InterfaceElementPtr element)
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
        // Use a compound implementation
        impl = createCompoundImplementation(element->asA<NodeGraph>());
    }
    else if (element->isA<Implementation>())
    {
        // Try creating a new in the factory
        impl = _implFactory.create(name);
        if (!impl)
        {
            // Fall back to the default implementation
            impl = createDefaultImplementation(element->asA<Implementation>());
        }
    }
    else
    {
        throw ExceptionShaderGenError("Element '" + name + "' is neither an Implementation nor an NodeGraph");
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

SgImplementationPtr ShaderGenerator::createDefaultImplementation(ImplementationPtr impl)
{
    // The data driven source code implementation
    // is the implementation to use by default
    return SourceCode::creator();
}

SgImplementationPtr ShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return Compound::creator();
}

}
