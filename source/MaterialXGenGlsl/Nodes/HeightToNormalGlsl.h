#ifndef MATERIALX_HEIGHTTONORMALGLSL_H
#define MATERIALX_HEIGHTTONORMALGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/Compound.h>

namespace MaterialX
{

/// Implementation of height-field to normal for GLSL
class HeightToNormalGlsl : public SgImplementation
{
  public:
    static SgImplementationPtr create();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

  private:
    void emitInputSamples(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, HwShader& shader,
                          const unsigned int sampleCount, StringVec& sampleStrings) const;

    // TODO: Add kernal option
};

} // namespace MaterialX

#endif
