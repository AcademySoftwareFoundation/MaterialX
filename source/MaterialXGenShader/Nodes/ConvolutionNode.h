#ifndef MATERIALX_CONVOLUTIONNODE_H
#define MATERIALX_CONVOLUTIONNODE_H

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

/// Utility class for implementations of nodes which perform convolutions
///
class ConvolutionNode : public ShaderNodeImpl
{
  public:
     void createVariables(Shader& shader, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

  protected:
    /// Constructor
    ConvolutionNode();

    /// Derived classes are responsible for returning if a given type is an acceptable input.
    virtual bool acceptsInputType(const TypeDesc* type) const = 0;

    // Derived classes are responsible for computing offset strings relative to the center sample
    // The sample size and offset type are passed in as arguments.
    virtual void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings) const = 0;

    /// Generate upstream / input sampling code in uv space and cache the output variable names which 
    /// will hold the sample values after execution.
    void emitInputSamplesUV(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context,
                            StringVec& sampleStrings) const;

    /// Number of samples.
    unsigned int _sampleCount;

    /// Filter size. Default value is 1
    float _filterSize;

    /// Filter offset. Default value is 0
    float _filterOffset;

    /// Name of function to compute sample size in uv space. Takes uv, filter size, and filter offset
    /// as input, and return a 2 channel vector as output
    string _sampleSizeFunctionUV;
};

} // namespace MaterialX

#endif
