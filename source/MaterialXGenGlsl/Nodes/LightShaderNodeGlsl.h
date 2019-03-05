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

    void initialize(ElementPtr implementation, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    VariableBlock _lightUniforms;
};

} // namespace MaterialX

#endif
