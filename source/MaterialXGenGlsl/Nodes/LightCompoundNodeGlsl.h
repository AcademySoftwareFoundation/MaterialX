//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LIGHTCOMPOUNDNODEGLSL_H
#define MATERIALX_LIGHTCOMPOUNDNODEGLSL_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

class GlslShaderGenerator;

/// LightCompound node implementation for GLSL
class LightCompoundNodeGlsl : public CompoundNode
{
public:
    LightCompoundNodeGlsl();

    static ShaderNodeImplPtr create();

    const string& getTarget() const override;

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    void emitFunctionDefinition(HwClosureContextPtr ccx, GenContext& context, ShaderStage& stage) const;

    VariableBlock _lightUniforms;
};

} // namespace MaterialX

#endif
