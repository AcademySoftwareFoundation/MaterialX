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

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen, const GenOptions& options) override;

    void createVariables(ShaderStage& stage, const ShaderNode& node) const override;

    void emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen) const override;

    ShaderGraph* getGraph() const override { return _rootGraph.get(); }

protected:
    ShaderGraphPtr _rootGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
