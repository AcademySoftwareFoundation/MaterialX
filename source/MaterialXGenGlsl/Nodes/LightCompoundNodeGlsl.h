#ifndef MATERIALX_LIGHTCOMPOUNDNODEGLSL_H
#define MATERIALX_LIGHTCOMPOUNDNODEGLSL_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

class GlslShaderGenerator;
class HwGenContext;
class HwClosureContext;

/// Implementation of 'light' node for GLSL
class LightCompoundNodeGlsl : public CompoundNode
{
public:
    LightCompoundNodeGlsl();

    static ShaderNodeImplPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen, GenContext& context) override;

    void createVariables(Shader& shader, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;

protected:
    void emitFunctionDefinition(ShaderStage& stage, GlslShaderGenerator& shadergen,
                                GenContext& context, const HwClosureContext* ccx) const;

    VariableBlock _lightUniforms;
};

} // namespace MaterialX

#endif
