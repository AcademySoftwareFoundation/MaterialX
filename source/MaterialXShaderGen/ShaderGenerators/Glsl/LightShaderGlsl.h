#ifndef MATERIALX_LIGHTSHADERGLSL_H
#define MATERIALX_LIGHTSHADERGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/SourceCode.h>

namespace MaterialX
{

/// Implementation of light shaders for GLSL.
/// Used for all light shaders implemented in source code.
class LightShaderGlsl : public SourceCode
{
public:
    static SgImplementationPtr creator();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    vector<Shader::Variable> _lightUniforms;
};

} // namespace MaterialX

#endif
