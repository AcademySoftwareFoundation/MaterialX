#ifndef MATERIALX_LIGHTSHADERGLSL_H
#define MATERIALX_LIGHTSHADERGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of light shaders for GLSL.
/// Currently supports light shaders implemented in shader code.
///
/// TODO: Add support for light shaders implemented as node graphs
/// and using EDFs to describe light distributions.
///
class LightGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr creator();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    string _functionName;
    string _functionSource;
    vector<Shader::Variable> _lightUniforms;
};

} // namespace MaterialX

#endif
