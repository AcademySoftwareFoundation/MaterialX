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

string ShaderGenerator::SEMICOLON = ";";
string ShaderGenerator::COMMA = ",";

ShaderGenerator::ShaderGenerator(SyntaxPtr syntax)
    : _syntax(syntax)
{
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

void ShaderGenerator::emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, GenContext& context)
{
    stage.addFunctionDefinition(node, *this, context);
}

void ShaderGenerator::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, bool checkScope)
{
    // Omit node if it's only used inside a conditional branch
    if (checkScope && node.referencedConditionally())
    {
        emitComment(stage, "Omitted node '" + node.getName() + "'. Only used in conditional node '" +
                    node.getScopeInfo().conditionalNode->getName() + "'");
    }
    else
    {
        node.getImplementation().emitFunctionCall(stage, node, *this, context);
    }
}

void ShaderGenerator::emitFunctionDefinitions(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
    // Emit function definitions for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionDefinition(stage, *node, context);
    }
}

void ShaderGenerator::emitFunctionCalls(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
    // Emit function calls for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(stage, *node, context);
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

void ShaderGenerator::emitConstant(ShaderStage& stage, const Variable& constant)
{
    emitVariable(stage, constant, _syntax->getConstantQualifier(), true);
}

void ShaderGenerator::emitUniform(ShaderStage& stage, const Variable& uniform)
{
    emitVariable(stage, uniform, _syntax->getUniformQualifier(), true);
}

void ShaderGenerator::emitVariable(ShaderStage& stage, const Variable& variable, const string& qualifier, bool assingValue)
{
    // If an array we need an array qualifier (suffix) for the variable name
    string arraySuffix;
    variable.getArraySuffix(arraySuffix);

    string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
    str += _syntax->getTypeName(variable.getType()) + " " + variable.getName();
    str += arraySuffix;

    if (assingValue)
    {
        const string valueStr = (variable.getValue() ?
            _syntax->getValue(variable.getType(), *variable.getValue(), true) :
            _syntax->getDefaultValue(variable.getType(), true));
        str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
    }

    stage.addString(str);
}

void ShaderGenerator::emitVariableBlock(ShaderStage& stage, const VariableBlock& block, 
                                        const string& qualifier, const string& separator,
                                        bool assingValues)
{
    for (size_t i=0; i<block.size(); ++i)
    {
        emitLineBegin(stage);
        emitVariable(stage, block[i], qualifier, assingValues);
        emitString(stage, separator);
        emitLineEnd(stage, false);
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

void ShaderGenerator::registerImplementation(const string& name, CreatorFunction<ShaderNodeImpl> creator)
{
    _implFactory.registerClass(name, creator);
}

bool ShaderGenerator::implementationRegistered(const string& name) const
{
    return _implFactory.classRegistered(name);
}

ShaderNodeImplPtr ShaderGenerator::getImplementation(InterfaceElementPtr element, GenContext& context)
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
            // Fall back to the source code implementation
            impl = createSourceCodeImplementation(element->asA<Implementation>());
        }
    }
    else
    {
        throw ExceptionShaderGenError("Element '" + name + "' is neither an Implementation nor an NodeGraph");
    }

    impl->initialize(element, *this, context);
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

ShaderNodeImplPtr ShaderGenerator::createSourceCodeImplementation(ImplementationPtr impl)
{
    // The standard source code implementation
    // is the implementation to use by default
    return SourceCodeNode::create();
}

ShaderNodeImplPtr ShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return CompoundNode::create();
}

}
