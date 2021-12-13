//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_COMBINENODEMDL_H
#define MATERIALX_COMBINENODEMDL_H

#include <MaterialXGenMdl/Export.h>

#include <MaterialXGenShader/Nodes/CombineNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Custom combine node implementation for MDL
class MX_GENMDL_API CombineNodeMdl : public CombineNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
