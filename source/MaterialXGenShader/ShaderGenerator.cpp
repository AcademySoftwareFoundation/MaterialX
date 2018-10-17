#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenImplementation.h>
#include <MaterialXGenShader/Nodes/SourceCode.h>
#include <MaterialXGenShader/Nodes/Compound.h>

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
    // Create a default context to be used by all nodes
    // that have no specific context assigned
    _defaultContext = createContext(CONTEXT_DEFAULT);
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
    for (DagNode* node : shader.getDag()->getNodes())
    {
        shader.addFunctionDefinition(node, *this);
    }
}

void ShaderGenerator::emitFunctionCalls(const GenContext& context, Shader &shader)
{
    const bool debugOutput = true;

    // Emit function calls for all nodes
    for (DagNode* node : shader.getDag()->getNodes())
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
        else if (node->getContextIDs().count(CONTEXT_DEFAULT))
        {
            // Node is defined in the default context so make the function call for default context
            shader.addFunctionCall(node, *_defaultContext, *this);
        }
        else
        {
            // Node is not defined in either this context or default context 
            // Just emit the output variable set to default value, in case it
            // is referenced by another node in this context.
            shader.beginLine();
            emitOutput(context, node->getOutput(), true, true, shader);
            shader.endLine();
        }
    }
}

void ShaderGenerator::emitFinalOutput(Shader& shader) const
{
    Dag* graph = shader.getDag();
    const DagOutputSocket* outputSocket = graph->getOutputSocket();

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

void ShaderGenerator::getInput(const GenContext& context, const DagInput* input, string& result) const
{
    if (input->connection)
    {
        result = input->connection->name;
    }
    else if (input->value)
    {
        result = _syntax->getValue(input->type, *input->value);
    }
    else
    {
        result = _syntax->getDefaultValue(input->type);
    }

    // Look for any additional suffix to append
    string suffix;
    context.getInputSuffix(const_cast<DagInput*>(input), suffix);
    if (!suffix.empty())
    {
        result += suffix;
    }
}


void ShaderGenerator::emitInput(const GenContext& context, const DagInput* input, Shader &shader) const
{
    string result;
    getInput(context, input, result);
    shader.addStr(result);
}

void ShaderGenerator::emitOutput(const GenContext& context, const DagOutput* output, bool includeType, bool assignDefault, Shader& shader) const
{
    shader.addStr(includeType ? _syntax->getTypeName(output->type) + " " + output->name : output->name);

    // Look for any additional suffix to append
    string suffix;
    context.getOutputSuffix(const_cast<DagOutput*>(output), suffix);
    if (!suffix.empty())
    {
        shader.addStr(suffix);
    }

    if (assignDefault)
    {
        const string& value = _syntax->getDefaultValue(output->type);
        if (!value.empty())
        {
            shader.addStr(" = " + value);
        }
    }
}

void ShaderGenerator::addNodeContextIDs(DagNode* node) const
{
    node->addContextID(CONTEXT_DEFAULT);
}

const GenContext* ShaderGenerator::getNodeContext(int id) const
{
    auto it = _contexts.find(id);
    return it != _contexts.end() ? it->second.get() : nullptr;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunction<GenImplementation> creator)
{
    _implFactory.registerClass(name, creator);
}

bool ShaderGenerator::implementationRegistered(const string& name) const
{
    return _implFactory.classRegistered(name);
}

GenImplementationPtr ShaderGenerator::getImplementation(InterfaceElementPtr element)
{
    const string& name = element->getName();

    // Check if it's created already
    auto it = _cachedImpls.find(name);
    if (it != _cachedImpls.end())
    {
        return it->second;
    }

    GenImplementationPtr impl;
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

GenImplementationPtr ShaderGenerator::createDefaultImplementation(ImplementationPtr impl)
{
    // The data driven source code implementation
    // is the implementation to use by default
    return SourceCode::create();
}

GenImplementationPtr ShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return Compound::create();
}

GenContextPtr ShaderGenerator::createContext(int id)
{
    GenContextPtr context = std::make_shared<GenContext>(id);
    _contexts[id] = context;
    return context;
}

}
