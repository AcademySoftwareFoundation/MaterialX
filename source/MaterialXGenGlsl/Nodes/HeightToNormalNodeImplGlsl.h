#ifndef MATERIALX_HEIGHTTONORMALGLSL_H
#define MATERIALX_HEIGHTTONORMALGLSL_H

#include <MaterialXGenShader/Nodes/ConvolutionNodeImpl.h>

namespace MaterialX
{

/// Implementation of height-field to normal for GLSL
class HeightToNormalNodeImplGlsl : public ConvolutionNodeImpl
{
  public:
    using ParentClass = ConvolutionNodeImpl;

    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    HeightToNormalNodeImplGlsl();

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings) override;

    /// Name of filter function to call to compute normals from input samples
    string _filterFunctionName;
};

} // namespace MaterialX

#endif
