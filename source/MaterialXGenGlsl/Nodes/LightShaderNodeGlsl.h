#ifndef MATERIALX_LIGHTSHADERNODEGLSL_H
#define MATERIALX_LIGHTSHADERNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

/// Implementation of light shaders for GLSL.
/// Used for all light shaders implemented in source code.
class LightShaderNodeGlsl : public SourceCodeNode
{
public:
    LightShaderNodeGlsl();

    static ShaderNodeImplPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(GenContext& context, const ShaderGenerator& shadergen, ElementPtr implementation) override;

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

protected:
    VariableBlock _lightUniforms;
};

} // namespace MaterialX

#endif
