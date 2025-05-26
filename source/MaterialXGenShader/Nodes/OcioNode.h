//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OCIO_NODE_H
#define MATERIALX_OCIO_NODE_H

#ifdef MATERIALX_BUILD_OCIO

/// @file
/// OCIO node implementation

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

/// OCIO node implementation. Takes an OCIO GpuProcessor and
/// uses it to inject shadergen code.
class OcioNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage)
        const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage)
        const override;

  private:
    string getFunctionName() const;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_OCIO

#endif
