#ifndef MATERIALX_COMPARE_H
#define MATERIALX_COMPARE_H

#include <MaterialXGenShader/GenImplementation.h>

namespace MaterialX
{

/// Implementation of compare node
class Compare : public GenImplementation
{
public:
    static GenImplementationPtr create();

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
