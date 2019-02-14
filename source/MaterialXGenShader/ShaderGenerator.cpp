#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Nodes/CompoundNode.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

namespace
{
    const bool debugOutput = true;
}

ShaderGenerator::ShaderGenerator(SyntaxPtr syntax)
    : _syntax(syntax)
{
    // Create a default context to be used by all nodes
    // that have no specific context assigned
    _defaultContext = createContext(CONTEXT_DEFAULT);
}

void ShaderGenerator::emitScopeBegin(ShaderStage& stage, ShaderStage::Brackets brackets)
{
    stage.beginScope(brackets);
}

void ShaderGenerator::emitScopeEnd(ShaderStage& stage, bool semicolon, bool newline)
{
    stage.endScope(semicolon, newline);
}

void ShaderGenerator::emitLineBegin(ShaderStage& stage)
{
    stage.beginLine();
}

void ShaderGenerator::emitLineEnd(ShaderStage& stage, bool semicolon)
{
    stage.endLine(semicolon);
}

void ShaderGenerator::emitLineBreak(ShaderStage& stage)
{
    stage.newLine();
}

void ShaderGenerator::emitString(ShaderStage& stage, const string& str)
{
    stage.addString(str);
}

void ShaderGenerator::emitLine(ShaderStage& stage, const string& str, bool semicolon)
{
    stage.addLine(str, semicolon);
}

void ShaderGenerator::emitBlock(ShaderStage& stage, const string& str)
{
    stage.addBlock(str, *this);
}

void ShaderGenerator::emitFunctionDefinition(ShaderStage& stage, ShaderNode* node)
{
    stage.addFunctionDefinition(node, *this);
}

void ShaderGenerator::emitFunctionCall(ShaderStage& stage, ShaderNode* node, const GenContext& context, bool checkScope)
{
    // Omit node if it's only used inside a conditional branch
    if (checkScope && node->referencedConditionally())
    {
        if (debugOutput)
        {
            std::stringstream str;
            str << "// Omitted node '" << node->getName() << "'. Only used in conditional node '" << node->getScopeInfo().conditionalNode->getName() << "'";
            stage.addLine(str.str(), false);
        }
        // Omit this node
        return;
    }

    // Check if this node has the given context defined
    if (node->getContextIDs().count(context.id()))
    {
        // Node has this context id defined so make the function call for this context
        stage.addFunctionCall(node, context, *this);
    }
    else if (node->getContextIDs().count(CONTEXT_DEFAULT))
    {
        // Node is defined in the default context so make the function call for default context
        stage.addFunctionCall(node, *_defaultContext, *this);
    }
    else
    {
        // Node is not defined in either this context or default context
        // Just emit the output variable set to default value, in case it
        // is referenced by another node in this context.
        stage.beginLine();
        emitOutput(stage, context, node->getOutput(), true, true);
        stage.endLine();
    }
}

void ShaderGenerator::emitInclude(ShaderStage& stage, const string& file)
{
    stage.addInclude(file, *this);
}

void ShaderGenerator::emitComment(ShaderStage& stage, const string& str)
{
    stage.addComment(str);
}

void ShaderGenerator::emitTypeDefinitions(ShaderStage& stage)
{
    // Emit typedef statements for all data types that have an alias
    for (auto syntax : _syntax->getTypeSyntaxs())
    {
        if (!syntax->getTypeAlias().empty())
        {
            stage.addLine("#define " + syntax->getName() + " " + syntax->getTypeAlias(), false);
        }
        if (!syntax->getTypeDefinition().empty())
        {
            stage.addLine(syntax->getTypeDefinition(), false);
        }
    }
    stage.newLine();
}

/*
void ShaderGenerator::emitFunctionDefinitions(ShaderStage& stage)
{
    // Emit function definitions for all nodes
    for (ShaderNode* node : shader.getGraph()->getNodes())
    {
        shader.addFunctionDefinition(node, *this);
    }
}
void ShaderGenerator::emitFinalOutput(ShaderStage& stage) const
{
    ShaderGraph* graph = shader.getGraph();
    const ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();

    if (!outputSocket->connection)
    {
        // Early out for the rare case where the whole graph is just a single value
        shadergen.emitLine(stage, outputSocket->variable + " = " + (outputSocket->value ?
            _syntax->getValue(outputSocket->type, *outputSocket->value) :
            _syntax->getDefaultValue(outputSocket->type)));
        return;
    }

    shadergen.emitLine(stage, outputSocket->variable + " = " + outputSocket->connection->variable);
}
*/

void ShaderGenerator::emitConstant(ShaderStage& stage, const Variable& constant)
{
    emitVariable(stage, constant, _syntax->getConstantQualifier());
}

void ShaderGenerator::emitUniform(ShaderStage& stage, const Variable& uniform)
{
    emitVariable(stage, uniform, _syntax->getUniformQualifier());
}

void ShaderGenerator::emitVariable(ShaderStage& stage, const Variable& variable, const string& qualifier)
{
    const string initStr = (variable.getValue() ? 
        _syntax->getValue(variable.getType(), *variable.getValue(), true) : 
        _syntax->getDefaultValue(variable.getType(), true));

    // If an array we need an array qualifier (suffix) for the variable name
    string arraySuffix;
    variable.getArraySuffix(arraySuffix);

    string line = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
    line += _syntax->getTypeName(variable.getType()) + " " + variable.getName();
    line += arraySuffix;
    line += initStr.empty() ? EMPTY_STRING : " = " + initStr;

    stage.addLine(line);
}

void ShaderGenerator::emitVariableBlock(ShaderStage& stage, const VariableBlock& block, const string& qualifier)
{
    if (!block.empty())
    {
        for (size_t i=0; i<block.size(); ++i)
        {
            emitVariable(stage, *block[i], qualifier);
        }
        stage.newLine();
    }
}

void ShaderGenerator::emitInput(ShaderStage& stage, const GenContext& context, const ShaderInput* input) const
{
    string result;
    getInput(context, input, result);
    stage.addString(result);
}

void ShaderGenerator::emitOutput(ShaderStage& stage, const GenContext& context, const ShaderOutput* output, bool includeType, bool assignDefaultValue) const
{
    stage.addString(includeType ? _syntax->getTypeName(output->type) + " " + output->variable : output->variable);

    // Look for any additional suffix to append
    string suffix;
    context.getOutputSuffix(output, suffix);
    if (!suffix.empty())
    {
        stage.addString(suffix);
    }

    if (assignDefaultValue)
    {
        const string& value = _syntax->getDefaultValue(output->type);
        if (!value.empty())
        {
            stage.addString(" = " + value);
        }
    }
}

void ShaderGenerator::pushActiveGraph(ShaderStage& stage, ShaderGraph* graph) const
{
    stage.pushActiveGraph(graph);
}

void ShaderGenerator::popActiveGraph(ShaderStage& stage) const
{
    stage.popActiveGraph();
}

void ShaderGenerator::getInput(const GenContext& context, const ShaderInput* input, string& result) const
{
    if (input->connection)
    {
        result = input->connection->variable;

        // Look for any additional suffix to append
        string suffix;
        context.getInputSuffix(input, suffix);
        if (!suffix.empty())
        {
            result += suffix;
        }
    }
    else
    {
        result = input->value ? _syntax->getValue(input->type, *input->value) : _syntax->getDefaultValue(input->type);
    }
}

void ShaderGenerator::addContextIDs(ShaderNode* node) const
{
    node->addContextID(CONTEXT_DEFAULT);
}

const GenContext* ShaderGenerator::getContext(int id) const
{
    auto it = _contexts.find(id);
    return it != _contexts.end() ? it->second.get() : nullptr;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunction<ShaderNodeImpl> creator)
{
    _implFactory.registerClass(name, creator);
}

bool ShaderGenerator::implementationRegistered(const string& name) const
{
    return _implFactory.classRegistered(name);
}

ShaderNodeImplPtr ShaderGenerator::getImplementation(InterfaceElementPtr element, const GenOptions& options)
{
    const string& name = element->getName();

    // Check if it's created already
    auto it = _cachedImpls.find(name);
    if (it != _cachedImpls.end())
    {
        return it->second;
    }

    ShaderNodeImplPtr impl;
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

    impl->initialize(element, *this, options);
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

ShaderStagePtr ShaderGenerator::createStage(Shader& shader, const string& name)
{
    return shader.createStage(name, _syntax);
}

ShaderNodeImplPtr ShaderGenerator::createDefaultImplementation(ImplementationPtr impl)
{
    // The data driven source code implementation
    // is the implementation to use by default
    return SourceCodeNode::create();
}

ShaderNodeImplPtr ShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return CompoundNode::create();
}

GenContextPtr ShaderGenerator::createContext(int id)
{
    GenContextPtr context = std::make_shared<GenContext>(id);
    _contexts[id] = context;
    return context;
}

}
