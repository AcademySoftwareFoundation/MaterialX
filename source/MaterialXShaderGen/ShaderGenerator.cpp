#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/Nodes/SourceCode.h>
#include <MaterialXShaderGen/Nodes/Compound.h>

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
    // Create a default node context to be used by all nodes
    // that have no specific context assigned
    _defaultNodeContext = createNodeContext(NODE_CONTEXT_DEFAULT);
}

Shader::VDirection ShaderGenerator::getTargetVDirection() const
{
    // Default is to use vdirection up
    return Shader::VDirection::UP;
}

void ShaderGenerator::emitTypeDefs(Shader& shader)
{
    // Emit typedef statements for all data types that needs it
    for (auto syntax : _syntax->getTypeSyntaxs())
    {
        if (syntax->getTypeDefStatement().length())
        {
            shader.addLine(syntax->getTypeDefStatement(), false);
        }
    }
    shader.newLine();
}

void ShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    // Emit function definitions for all nodes
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        shader.addFunctionDefinition(node, *this);
    }
}

void ShaderGenerator::emitFunctionCalls(const SgNodeContext& context, Shader &shader)
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

        // Check if this node has the given context defined
        if (node->getContextIDs().count(context.id()))
        {
            // Node has this context id defined so make the function call for this context
            shader.addFunctionCall(node, context, *this);
        }
        else if (node->getContextIDs().count(NODE_CONTEXT_DEFAULT))
        {
            // Node is defined in the default context so make the function call for default context
            shader.addFunctionCall(node, *_defaultNodeContext, *this);
        }
    }
}

void ShaderGenerator::emitFinalOutput(Shader& shader) const
{
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutputSocket* outputSocket = graph->getOutputSocket();

    if (!outputSocket->connection)
    {
        // Early out for the rare case where the whole graph is just a single value
        shader.addLine(outputSocket->name + " = " + (outputSocket->value ?
            _syntax->getValue(outputSocket->type, *outputSocket->value) :
            _syntax->getDefaultValue(outputSocket->type)));
        return;
    }

    shader.addLine(outputSocket->name + " = " + outputSocket->connection->name);
}

void ShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    const string initStr = (uniform.value ? _syntax->getValue(uniform.type, *uniform.value, true) : _syntax->getDefaultValue(uniform.type, true));
    shader.addStr(_syntax->getTypeName(uniform.type) + " " + uniform.name + (initStr.empty() ? "" : " = " + initStr));
}

void ShaderGenerator::emitInput(const SgInput* input, Shader &shader) const
{
    if (input->connection)
    {
        shader.addStr(input->connection->name);
    }
    else if (input->value)
    {
        shader.addStr(_syntax->getValue(input->type, *input->value));
    }
    else
    {
        shader.addStr(_syntax->getDefaultValue(input->type));
    }
}

void ShaderGenerator::emitOutput(const SgOutput* output, bool includeType, Shader& shader) const
{
    string typeStr;
    if (includeType)
    {
        typeStr = _syntax->getTypeName(output->type) + " ";
    }
    shader.addStr(typeStr + output->name);
}

void ShaderGenerator::addNodeContextIDs(SgNode* node) const
{
    node->addContextID(NODE_CONTEXT_DEFAULT);
}

const SgNodeContext* ShaderGenerator::getNodeContext(int id) const
{
    auto it = _nodeContexts.find(id);
    return it != _nodeContexts.end() ? it->second.get() : nullptr;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunction<SgImplementation> creator)
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
    return SourceCode::create();
}

SgImplementationPtr ShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return Compound::create();
}

SgNodeContextPtr ShaderGenerator::createNodeContext(int id)
{
    SgNodeContextPtr context = std::make_shared<SgNodeContext>(id);
    _nodeContexts[id] = context;
    return context;
}

}
