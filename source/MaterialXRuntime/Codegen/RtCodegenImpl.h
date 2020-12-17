//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCODEGENIMPL_H
#define MATERIALX_RTCODEGENIMPL_H

/// @file RtCodegenImpl.h
/// TODO: Docs

#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtNode.h>

#include <MaterialXGenShader/GenContext.h>

namespace MaterialX
{

/// @class RtCodegenImpl
/// Base class for all node implementations that use shader code generation.
class RtCodegenImpl : public RtNodeImpl
{
public:
    /// Add additional inputs on the node if needed by the implementation.
    virtual void addInputs(const RtNode& node, GenContext& context) const;

    /// Create shader variables needed for the implementation of this node (e.g. uniforms, inputs and outputs).
    /// Used if the node requires explicit input data from the application.
    virtual void createVariables(const RtNode& node, GenContext& context, Shader& shader) const;

    /// Emit function definition for the given node instance in the given context.
    virtual void emitFunctionDefinition(const RtNode& node, GenContext& context, ShaderStage& stage) const;

    /// Emit the function call or inline source code for given node instance in the given context.
    virtual void emitFunctionCall(const RtNode& node, GenContext& context, ShaderStage& stage) const;

protected:
    /// Constructor.
    RtCodegenImpl(const RtPrim& prim) : RtNodeImpl(prim) {}
};

}

#endif
