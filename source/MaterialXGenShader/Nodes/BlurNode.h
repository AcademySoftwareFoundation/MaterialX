#ifndef MATERIALX_BLURNODE_H
#define MATERIALX_BLURNODE_H

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

namespace MaterialX
{

/// Implementation of blur convolution
class BlurNode : public ConvolutionNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

  protected:
    /// Constructor
    BlurNode();

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString,
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;

    /// Box filter option on blur
    static const string BOX_FILTER;
    /// Box filter weights variable name
    static const string BOX_WEIGHTS_VARIABLE;

    /// Gaussian filter option on blur
    static const string GAUSSIAN_FILTER;
    /// Gaussian filter weights variable name
    static const string GAUSSIAN_WEIGHTS_VARIABLE;

    /// String constants
    static const string IN_STRING;
    static const string FILTER_TYPE_STRING;
    static const string FILTER_SIZE_STRING;
};

} // namespace MaterialX

#endif
