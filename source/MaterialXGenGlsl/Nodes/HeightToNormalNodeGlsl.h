#ifndef MATERIALX_HEIGHTTONORMALNODEGLSL_H
#define MATERIALX_HEIGHTTONORMALNODEGLSL_H

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

namespace MaterialX
{

/// Implementation of height-field to normal for GLSL
class HeightToNormalNodeGlsl : public ConvolutionNode
{
  public:
    using ParentClass = ConvolutionNode;

    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    HeightToNormalNodeGlsl();

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, StringVec& offsetStrings) override;

    /// Name of filter function to call to compute normals from input samples
    string _filterFunctionName;
};

} // namespace MaterialX

#endif
