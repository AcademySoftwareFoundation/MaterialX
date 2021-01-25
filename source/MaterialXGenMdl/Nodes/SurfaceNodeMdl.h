//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SURFACENODEMDL_H
#define MATERIALX_SURFACENODEMDL_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Surface node implementation for MDL
class SurfaceNodeMdl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
