#ifndef MATERIALX_CONVOLUTIONGLSL_H
#define MATERIALX_CONVOLUTIONGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/Compound.h>

namespace MaterialX
{

/// GLSL utility class for implementations of nodes which perform convolutions
///
class ConvolutionGlsl : public SgImplementation
{
  protected:
    /// Constructor
    ConvolutionGlsl();

    /// Derived classes are responsible for returning if a given type is an acceptable input.
    virtual bool acceptsInputType(const TypeDesc* type) = 0;

    // Derived classes are responsible for computing offset strings relative to the center sample
    // The sample size is passed over.
    virtual void computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings) = 0;

    /// Generate upstream / input sampling code in uv space and cache the output variable names which 
    /// will hold the sample values after execution.
    void emitInputSamplesUV(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, HwShader& shader,
                            StringVec& sampleStrings);
    
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
