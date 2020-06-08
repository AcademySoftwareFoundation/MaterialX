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

  protected:
    /// String constants
    static const string TOP_STRING;
    static const string BASE_STRING;
};

} // namespace MaterialX

#endif
