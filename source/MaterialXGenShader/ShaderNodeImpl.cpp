//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderNodeImpl.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderNode.h>

MATERIALX_NAMESPACE_BEGIN

//
// ShaderNodeImpl methods
//

ShaderNodeImpl::ShaderNodeImpl() :
    _name(EMPTY_STRING),
    _hash(0)
{
}

void ShaderNodeImpl::initialize(const InterfaceElement& element, GenContext&)
{
    // Store name
    _name = element.getName();

    // By default use the implementation name as hash to make it unique.
    // Derived classes can override this to create other hashes,
    // e.g. to share the same hash between nodes that can share
    // the same function definition.
    _hash = std::hash<string>{}(_name);
}

void ShaderNodeImpl::addInputs(ShaderNode&, GenContext&) const
{
}

void ShaderNodeImpl::setValues(const Node&, ShaderNode&, GenContext&) const
{
}

void ShaderNodeImpl::addClassification(ShaderNode&) const
{
}

void ShaderNodeImpl::createVariables(const ShaderNode&, GenContext&, Shader&) const
{
}

void ShaderNodeImpl::emitFunctionDefinition(const ShaderNode&, GenContext&, ShaderStage&) const
{
}

void ShaderNodeImpl::emitFunctionCall(const ShaderNode&, GenContext&, ShaderStage&) const
{
}

void ShaderNodeImpl::emitOutputVariables(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // Default implementation of output variable declaration.
    // Initialize variables to their type default value.
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(i), true, true, context, stage);
        shadergen.emitLineEnd(stage);
    }
}

ShaderGraph* ShaderNodeImpl::getGraph() const
{
    return nullptr;
}

bool ShaderNodeImpl::nodeOutputIsClosure(const ShaderNode& node) const
{
    const auto& outputs = node.getOutputs();
    if (outputs.empty())
    {
        // This should never happen as we auto populate the default 'out' output based on the
        // node type if no output is present.
        throw ExceptionShaderGenError("Node has no outputs defined'");
    }
    return outputs[0]->getType().isClosure();
}

ShaderNodeImplPtr NopNode::create()
{
    return std::make_shared<NopNode>();
}

MATERIALX_NAMESPACE_END
