//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SOURCECODECLOSURENODEGLSL_H
#define MATERIALX_SOURCECODECLOSURENODEGLSL_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

class SourceCodeClosureNodeGlsl : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    const string& getTarget() const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
