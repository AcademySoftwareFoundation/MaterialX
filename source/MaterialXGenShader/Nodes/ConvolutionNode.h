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
     void createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

  protected:
    /// Constructor
    ConvolutionNode();

    /// Derived classes are responsible for returning if a given type is an acceptable input.
    virtual bool acceptsInputType(const TypeDesc* type) const = 0;

    // Derived classes are responsible for computing offset strings relative to the center sample
    // The sample size and offset type are passed in as arguments.
    virtual void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, 
                                            unsigned int filterWidth, StringVec& offsetStrings) const = 0;

    /// Get input which is used for sampling. If there is none
    /// then a null pointer is returned.
    virtual const ShaderInput* getSamplingInput(const ShaderNode& node) const;

    /// Generate upstream / input sampling code in uv space and cache the output variable names which 
    /// will hold the sample values after execution.
    void emitInputSamplesUV(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context,
                            unsigned int sampleCount, unsigned int filterWidth, float filterSize, float filterOffset,
                            const string& sampleSizeFunctionUV,  StringVec& sampleStrings) const;

    static const string SAMPLE2D_INPUT;
    static const string SAMPLE3D_INPUT;
};

} // namespace MaterialX

#endif
