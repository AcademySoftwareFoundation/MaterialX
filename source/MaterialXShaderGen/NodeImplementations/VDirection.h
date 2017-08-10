#ifndef MATERIALX_VDIRECTION_H
#define MATERIALX_VDIRECTION_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// vdirection flip in OSL
class VDirectionFlipOsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionFlipOsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection no-op in OSL
class VDirectionNoOpOsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionNoOpOsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection flip in GLSL
class VDirectionFlipGlsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionFlipGlsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection no-op in GLSL
class VDirectionNoOpGlsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionNoOpGlsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

}

#endif
