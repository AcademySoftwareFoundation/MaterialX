//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HEIGHTTONORMALNODEMDL_H
#define MATERIALX_HEIGHTTONORMALNODEMDL_H

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

namespace MaterialX
{

/// HeightToNormal node implementation for MDL
class HeightToNormalNodeMdl : public ConvolutionNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    const string& getTarget() const override;

  protected:
    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, 
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;
};

} // namespace MaterialX

#endif
