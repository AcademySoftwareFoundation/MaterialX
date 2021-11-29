//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_IMAGENODEOSL_H
#define MATERIALX_IMAGENODEOSL_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

/// Extending the SourceCodeNode with requirements for OSL image nodes.
class MX_GENSHADER_API ImageNodeOsl: public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
