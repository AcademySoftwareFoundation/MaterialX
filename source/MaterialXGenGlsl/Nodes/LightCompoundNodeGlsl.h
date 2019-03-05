#ifndef MATERIALX_LIGHTCOMPOUNDNODEGLSL_H
#define MATERIALX_LIGHTCOMPOUNDNODEGLSL_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

class GlslShaderGenerator;

/// Implementation of 'light' node for GLSL
class LightCompoundNodeGlsl : public CompoundNode
{
public:
    LightCompoundNodeGlsl();

    static ShaderNodeImplPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(ElementPtr implementation, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    void emitFunctionDefinition(HwClosureContextPtr ccx, GenContext& context, ShaderStage& stage) const;

    VariableBlock _lightUniforms;
};

} // namespace MaterialX

#endif
