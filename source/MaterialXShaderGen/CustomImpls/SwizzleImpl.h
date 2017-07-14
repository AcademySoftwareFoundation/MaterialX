#ifndef MATERIALX_SWIZZLEIMPL_H
#define MATERIALX_SWIZZLEIMPL_H

#include <MaterialXShaderGen/CustomImpl.h>

namespace MaterialX
{

/// Implementation of swizzle node
class SwizzleImpl : public CustomImpl
{
    DECLARE_IMPLEMENTATION(SwizzleImpl)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
