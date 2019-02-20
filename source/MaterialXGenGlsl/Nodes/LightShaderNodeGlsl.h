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

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen, GenContext& context) override;

    void createVariables(Shader& shader, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

protected:
    VariableBlock _lightUniforms;
};

} // namespace MaterialX

#endif
