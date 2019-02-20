#ifndef MATERIALX_HEIGHTTONORMALNODEGLSL_H
#define MATERIALX_HEIGHTTONORMALNODEGLSL_H

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

namespace MaterialX
{

/// Implementation of height-field to normal for GLSL
class HeightToNormalNodeGlsl : public ConvolutionNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

  protected:
    /// Constructor
    HeightToNormalNodeGlsl();

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, 
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;
};

} // namespace MaterialX

#endif
