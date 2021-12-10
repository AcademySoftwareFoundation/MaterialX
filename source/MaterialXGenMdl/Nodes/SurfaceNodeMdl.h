//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SURFACENODEMDL_H
#define MATERIALX_SURFACENODEMDL_H

#include <MaterialXGenMdl/Export.h>

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

/// Surface node implementation for MDL
class MX_GENMDL_API SurfaceNodeMdl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
