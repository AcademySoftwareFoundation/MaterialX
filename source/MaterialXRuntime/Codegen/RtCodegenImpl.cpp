//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Codegen/RtCodegenImpl.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

void RtCodegenImpl::addInputs(const RtNode& /*node*/, GenContext& /*context*/) const
{
}

void RtCodegenImpl::createVariables(const RtNode& /*node*/, GenContext& /*context*/, Shader& /*shader*/) const
{
}

void RtCodegenImpl::emitFunctionDefinition(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

void RtCodegenImpl::emitFunctionCall(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

}
