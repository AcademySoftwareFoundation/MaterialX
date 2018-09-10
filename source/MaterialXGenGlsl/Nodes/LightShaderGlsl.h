#ifndef MATERIALX_LIGHTSHADERGLSL_H
#define MATERIALX_LIGHTSHADERGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCode.h>

namespace MaterialX
{

/// Implementation of light shaders for GLSL.
/// Used for all light shaders implemented in source code.
class LightShaderGlsl : public SourceCode
{
public:
    static SgImplementationPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    vector<Shader::Variable> _lightUniforms;
};

} // namespace MaterialX

#endif
