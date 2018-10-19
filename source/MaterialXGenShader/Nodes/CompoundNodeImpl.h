#ifndef MATERIALX_COMPOUND_H
#define MATERIALX_COMPOUND_H

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

class CompoundNodeImpl : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

    ShaderGraph* getGraph() const override { return _rootGraph.get(); }

protected:
    ShaderGraphPtr _rootGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
