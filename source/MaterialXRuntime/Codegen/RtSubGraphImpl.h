//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSUBGRAPHIMPL_H
#define MATERIALX_RTSUBGRAPHIMPL_H

/// @file RtCompoundNode.h
/// TODO: Docs

#include <MaterialXRuntime/Codegen/RtCodegenImpl.h>

namespace MaterialX
{

/// @class RtSubGraphImpl
/// A node implementation using a subgraph.
class RtSubGraphImpl : public RtCodegenImpl
{
    DECLARE_TYPED_SCHEMA(RtSubGraphImpl)

public:
    /// Constructor.
    RtSubGraphImpl(const RtPrim& prim) : RtCodegenImpl(prim) {}

    /// Initialize this implementation with the nodegraph prim to use.
    void initialize(const RtPrim& nodegraph);

    /// Return the nodegraph prim for this implementation.
    RtPrim getNodeGraph() const;

    /// Emit function definition for the given node instance in the given context.
    void emitFunctionDefinition(const RtNode& node, GenContext& context, ShaderStage& stage) const override;

    /// Emit the function call for the given node instance in the given context.
    void emitFunctionCall(const RtNode& node, GenContext& context, ShaderStage& stage) const override;
};

}

#endif
