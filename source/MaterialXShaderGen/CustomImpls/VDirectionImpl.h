#ifndef MATERIALX_VDIRECTIONIMPL_H
#define MATERIALX_VDIRECTIONIMPL_H

#include <MaterialXShaderGen/CustomImpl.h>

namespace MaterialX
{

/// vdirection flip in OSL
class VDirectionImplFlipOsl : public CustomImpl
{
    DECLARE_IMPLEMENTATION(VDirectionImplFlipOsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection no-op in OSL
class VDirectionImplNoOpOsl : public CustomImpl
{
    DECLARE_IMPLEMENTATION(VDirectionImplNoOpOsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection flip in GLSL
class VDirectionImplFlipGlsl : public CustomImpl
{
    DECLARE_IMPLEMENTATION(VDirectionImplFlipGlsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

/// vdirection no-op in GLSL
class VDirectionImplNoOpGlsl : public CustomImpl
{
    DECLARE_IMPLEMENTATION(VDirectionImplNoOpGlsl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

}

#endif
