#ifndef MATERIALX_VDIRECTIONIMPL_H
#define MATERIALX_VDIRECTIONIMPL_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// vdirection flip in OSL
class VDirectionImplFlipOsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionImplFlipOsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection no-op in OSL
class VDirectionImplNoOpOsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionImplNoOpOsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection flip in GLSL
class VDirectionImplFlipGlsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionImplFlipGlsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection no-op in GLSL
class VDirectionImplNoOpGlsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(VDirectionImplNoOpGlsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

}

#endif
