#ifndef MATERIALX_COMPOUNDNODE_H
#define MATERIALX_COMPOUNDNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

class CompoundNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(GenContext& context, const ShaderGenerator& shadergen, ElementPtr implementation) override;

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionDefinition(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    ShaderGraph* getGraph() const override { return _rootGraph.get(); }

protected:
    ShaderGraphPtr _rootGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
