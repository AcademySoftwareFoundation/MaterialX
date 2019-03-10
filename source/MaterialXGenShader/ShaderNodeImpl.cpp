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

void ShaderNodeImpl::initialize(const InterfaceElement&, GenContext&)
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

size_t ShaderNodeImpl::getHash() const
{
    // For now use the instance pointer as the hash
    return reinterpret_cast<size_t>(this);
}

} // namespace MaterialX
