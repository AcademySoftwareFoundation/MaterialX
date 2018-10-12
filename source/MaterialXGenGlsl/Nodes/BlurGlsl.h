#ifndef MATERIALX_BLURGLSL_H
#define MATERIALX_BLURGLSL_H

#include <MaterialXGenGlsl/Nodes/ConvolutionGlsl.h>

namespace MaterialX
{

/// Implementation of blur for GLSL
class BlurGlsl : public ConvolutionGlsl
{
  public:
    using ParentClass = ConvolutionGlsl;

    static SgImplementationPtr create();

    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    BlurGlsl();

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) override;
    
    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings) override;

    /// Name of filter function to call to compute normals from input samples
    std::string _filterFunctionName;

    /// Type of filter 
    string _filterType;

    /// Language dependent input type string
    string _inputTypeString;
};

} // namespace MaterialX

#endif
