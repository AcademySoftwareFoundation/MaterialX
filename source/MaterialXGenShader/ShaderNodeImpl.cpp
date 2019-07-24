//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ShaderNodeImpl.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

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
    // e.g. to share the same hash beteen nodes that can share
    // the same function definition.
    _hash = std::hash<string>{}(_name);
}

void ShaderNodeImpl::addInputs(ShaderNode&, GenContext&) const
{
}

void ShaderNodeImpl::setValues(const Node&, ShaderNode&, GenContext&) const
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

ShaderGraph* ShaderNodeImpl::getGraph() const
{
    return nullptr;
}

} // namespace MaterialX
