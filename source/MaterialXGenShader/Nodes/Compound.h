#ifndef MATERIALX_COMPOUND_H
#define MATERIALX_COMPOUND_H

#include <MaterialXGenShader/GenImplementation.h>
#include <MaterialXGenShader/Dag.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

class Compound : public GenImplementation
{
public:
    static GenImplementationPtr create();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

    Dag* getDag() const override { return _dag.get(); }

protected:
    DagPtr _dag;
    string _functionName;
};

} // namespace MaterialX

#endif
