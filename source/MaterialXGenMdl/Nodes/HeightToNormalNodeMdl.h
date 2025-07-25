//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HEIGHTTONORMALNODEMDL_H
#define MATERIALX_HEIGHTTONORMALNODEMDL_H

#include <MaterialXGenMdl/Export.h>

#include <MaterialXGenMdl/Nodes/ConvolutionNode.h>

MATERIALX_NAMESPACE_BEGIN

/// HeightToNormal node implementation for MDL
class MX_GENMDL_API HeightToNormalNodeMdl : public ConvolutionNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    /// Return if given type is an acceptable input
    bool acceptsInputType(TypeDesc type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString,
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;
};

MATERIALX_NAMESPACE_END

#endif
