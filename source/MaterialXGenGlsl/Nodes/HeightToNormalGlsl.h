#ifndef MATERIALX_HEIGHTTONORMALGLSL_H
#define MATERIALX_HEIGHTTONORMALGLSL_H

#include <MaterialXGenGlsl/Nodes/ConvolutionGlsl.h>

namespace MaterialX
{

/// Implementation of height-field to normal for GLSL
class HeightToNormalGlsl : public ConvolutionGlsl
{
  public:
    using ParentClass = ConvolutionGlsl;

    static GenImplementationPtr create();

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  protected:
    /// Constructor
    HeightToNormalGlsl();

    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings) override;

    /// Name of filter function to call to compute normals from input samples
    string _filterFunctionName;
};

} // namespace MaterialX

#endif
