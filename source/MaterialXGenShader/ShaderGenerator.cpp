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

void ShaderGenerator::emitBlock(ShaderStage& stage, GenContext& context, const string& str) const
{
    stage.addBlock(context, str);
}

void ShaderGenerator::emitInclude(ShaderStage& stage, GenContext& context, const string& file) const
{
    stage.addInclude(context, file);
}

void ShaderGenerator::emitFunctionDefinition(ShaderStage& stage, GenContext& context, const ShaderNode& node) const
{
    stage.addFunctionDefinition(context, *this, node);
}

void ShaderGenerator::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderNode& node, bool checkScope) const
{
    // Omit node if it's only used inside a conditional branch
    if (checkScope && node.referencedConditionally())
    {
        emitComment(stage, "Omitted node '" + node.getName() + "'. Only used in conditional node '" +
                    node.getScopeInfo().conditionalNode->getName() + "'");
    }
    else
    {
        node.getImplementation().emitFunctionCall(stage, context, *this, node);
    }
}

void ShaderGenerator::emitFunctionDefinitions(ShaderStage& stage, GenContext& context, const ShaderGraph& graph) const
{
    // Emit function definitions for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionDefinition(stage, context, *node);
    }
}

void ShaderGenerator::emitFunctionCalls(ShaderStage& stage, GenContext& context, const ShaderGraph& graph) const
{
    // Emit function calls for all nodes in the graph.
    for (ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(stage, context, *node);
    }
}

void ShaderGenerator::emitTypeDefinitions(ShaderStage& stage, GenContext&) const
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

void ShaderGenerator::emitVariableDeclaration(ShaderStage& stage, GenContext&, const ShaderPort* variable, 
                                              const string& qualifier, bool assingValue) const
{
    string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
    str += _syntax->getTypeName(variable->getType()) + " " + variable->getVariable();

    // If an array we need an array qualifier (suffix) for the variable name
    if (variable->getType()->isArray() && variable->getValue())
    {
        str += _syntax->getArraySuffix(variable->getType(), *variable->getValue());
    }

    if (assingValue)
    {
        const string valueStr = (variable->getValue() ?
            _syntax->getValue(variable->getType(), *variable->getValue(), true) :
            _syntax->getDefaultValue(variable->getType(), true));
        str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
    }

    stage.addString(str);
}

void ShaderGenerator::emitVariableDeclarations(ShaderStage& stage, GenContext& context, const VariableBlock& block,
                                               const string& qualifier, const string& separator, bool assingValues) const
{
    for (size_t i=0; i<block.size(); ++i)
    {
        emitLineBegin(stage);
        emitVariableDeclaration(stage, context, block[i], qualifier, assingValues);
        emitString(stage, separator);
        emitLineEnd(stage, false);
    }
}

void ShaderGenerator::emitInput(ShaderStage& stage, GenContext& context, const ShaderInput* input) const
{
    stage.addString(getUpstreamResult(context, input));
}

void ShaderGenerator::emitOutput(ShaderStage& stage, GenContext& context, const ShaderOutput* output, bool includeType, bool assignDefaultValue) const
{
    stage.addString(includeType ? _syntax->getTypeName(output->getType()) + " " + output->getVariable() : output->getVariable());

    // Look for any additional suffix to append
    string suffix;
    context.getOutputSuffix(output, suffix);
    if (!suffix.empty())
    {
        stage.addString(suffix);
    }

    if (assignDefaultValue)
    {
        const string& value = _syntax->getDefaultValue(output->getType());
        if (!value.empty())
        {
            stage.addString(" = " + value);
        }
    }
}

string ShaderGenerator::getUpstreamResult(GenContext& context, const ShaderInput* input) const
{
    if (!input->getConnection())
    {
        return input->getValue() ? _syntax->getValue(input->getType(), *input->getValue()) : _syntax->getDefaultValue(input->getType());
    }

    string variable = input->getConnection()->getVariable();

    // Look for any additional suffix to append
    string suffix;
    context.getInputSuffix(input, suffix);
    if (!suffix.empty())
    {
        variable += suffix;
    }

    return variable;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunction<ShaderNodeImpl> creator)
{
    _implFactory.registerClass(name, creator);
}

bool ShaderGenerator::implementationRegistered(const string& name) const
{
    return _implFactory.classRegistered(name);
}

ShaderNodeImplPtr ShaderGenerator::getImplementation(GenContext& context, InterfaceElementPtr element) const
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
    impl->initialize(context, *this, element);

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
