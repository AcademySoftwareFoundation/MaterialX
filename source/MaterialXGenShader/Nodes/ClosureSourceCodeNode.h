//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURESOURCECODENODE_H
#define MATERIALX_CLOSURESOURCECODENODE_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

class MX_GENSHADER_API ClosureSourceCodeNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
