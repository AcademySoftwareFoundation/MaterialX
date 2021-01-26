//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LAYERNODE_H
#define MATERIALX_LAYERNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Layer node implementation
class LayerNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// Adding a base BSDF input to a node.
    static void addLayerSupport(ShaderNode& node);

    /// String constants
    static const string TOP;
    static const string BASE;
};

} // namespace MaterialX

#endif
