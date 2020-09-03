//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_COMBINENODEMDL_H
#define MATERIALX_COMBINENODEMDL_H

#include <MaterialXGenShader/Nodes/CombineNode.h>

namespace MaterialX
{

/// Custom combine node implementation for MDL
class CombineNodeMdl : public CombineNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
