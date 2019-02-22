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

void ShaderGenerator::emitScopeBegin(ShaderStage& stage, ShaderStage::Brackets brackets) const
{
    stage.beginScope(brackets);
}

void ShaderGenerator::emitScopeEnd(ShaderStage& stage, bool semicolon, bool newline) const
{
    stage.endScope(semicolon, newline);
}

void ShaderGenerator::emitLineBegin(ShaderStage& stage) const
{
    stage.beginLine();
}

void ShaderGenerator::emitLineEnd(ShaderStage& stage, bool semicolon) const
{
    stage.endLine(semicolon);
}

void ShaderGenerator::emitLineBreak(ShaderStage& stage) const
{
    stage.newLine();
}

void ShaderGenerator::emitString(ShaderStage& stage, const string& str) const
{
    stage.addString(str);
}

void ShaderGenerator::emitLine(ShaderStage& stage, const string& str, bool semicolon) const
{
    stage.addLine(str, semicolon);
}

void ShaderGenerator::emitComment(ShaderStage& stage, const string& str) const
{
    stage.addComment(str);
}

void ShaderGenerator::emitBlock(ShaderStage& stage, const string& str, GenContext& context) const
{
    stage.addBlock(str, context);
}

void ShaderGenerator::emitInclude(ShaderStage& stage, const string& file, GenContext& context) const
{
    stage.addInclude(file, context);
}

void ShaderGenerator::emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, GenContext& context) const
{
    stage.addFunctionDefinition(node, *this, context);
}

void ShaderGenerator::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, bool checkScope) const
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

void ShaderGenerator::emitFunctionDefinitions(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const
{
    // Emit function definitions for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionDefinition(stage, *node, context);
    }
}

void ShaderGenerator::emitFunctionCalls(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const
{
    // Emit function calls for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(stage, *node, context);
    }
}

void ShaderGenerator::emitTypeDefinitions(ShaderStage& stage) const
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

void ShaderGenerator::emitConstant(ShaderStage& stage, const Variable& constant) const
{
    emitVariable(stage, constant, _syntax->getConstantQualifier(), true);
}

void ShaderGenerator::emitUniform(ShaderStage& stage, const Variable& uniform) const
{
    emitVariable(stage, uniform, _syntax->getUniformQualifier(), true);
}

void ShaderGenerator::emitVariable(ShaderStage& stage, const Variable& variable, const string& qualifier, bool assingValue) const
{
    string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
    str += _syntax->getTypeName(variable.getType()) + " " + variable.getName();

    // If an array we need an array qualifier (suffix) for the variable name
    if (variable.getType()->isArray() && variable.getValue())
    {
        str += _syntax->getArraySuffix(variable.getType(), *variable.getValue());
    }

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
                                        bool assingValues) const
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

ShaderNodeImplPtr ShaderGenerator::getImplementation(InterfaceElementPtr element, GenContext& context) const
{
    const string& name = element->getName();

    // Check if it's created and cached already.
    ShaderNodeImplPtr impl = context.findNodeImplementation(name, getTarget());
    if (impl)
    {
        return impl;
    }

    if (element->isA<NodeGraph>())
    {
        // Use a compound implementation.
        impl = createCompoundImplementation(element->asA<NodeGraph>());
    }
    else if (element->isA<Implementation>())
    {
        // Try creating a new in the factory.
        impl = _implFactory.create(name);
        if (!impl)
        {
            // Fall back to the source code implementation.
            impl = createSourceCodeImplementation(element->asA<Implementation>());
        }
    }
    else
    {
        throw ExceptionShaderGenError("Element '" + name + "' is neither an Implementation nor an NodeGraph");
    }
    impl->initialize(element, *this, context);

    // Cache it.
    context.addNodeImplementation(name, getTarget(), impl);

    return impl;
}

ValuePtr ShaderGenerator::remapEnumeration(const ValueElementPtr&, const InterfaceElement&, 
                                           const TypeDesc*&) const
{
    return nullptr;
}

ValuePtr ShaderGenerator::remapEnumeration(const string&, const string&, const string&,
                                            const InterfaceElement&, const TypeDesc*&) const
{
    return nullptr;
}

ShaderStagePtr ShaderGenerator::createStage(Shader& shader, const string& name) const
{
    return shader.createStage(name, _syntax);
}

ShaderNodeImplPtr ShaderGenerator::createSourceCodeImplementation(ImplementationPtr impl) const
{
    // The standard source code implementation
    // is the implementation to use by default
    return SourceCodeNode::create();
}

ShaderNodeImplPtr ShaderGenerator::createCompoundImplementation(NodeGraphPtr impl) const
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return CompoundNode::create();
}

}
