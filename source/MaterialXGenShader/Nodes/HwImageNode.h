//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HWIMAGECODENODE_H
#define MATERIALX_HWIMAGECODENODE_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Extending the SourceCodeNode with requirements for image nodes.
class MX_GENSHADER_API HwImageNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext& context) const override;
    void setValues(const Node& node, ShaderNode& shaderNode, GenContext& context) const override;
};

MATERIALX_NAMESPACE_END

#endif
