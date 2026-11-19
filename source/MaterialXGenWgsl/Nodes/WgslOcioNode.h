//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLOCIO_NODE_H
#define MATERIALX_WGSLOCIO_NODE_H

#ifdef MATERIALX_BUILD_OCIO

#include <MaterialXGenWgsl/Export.h>
#include <MaterialXGenShader/Nodes/OcioNode.h>

MATERIALX_NAMESPACE_BEGIN

/// OCIO node for genwgsl: includes pre-baked WGSL from stdlib/genwgsl/ocio/.
class MX_GENWGSL_API WgslOcioNode : public OcioNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_OCIO

#endif
