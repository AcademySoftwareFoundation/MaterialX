//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_THINFILMNODE_H
#define MATERIALX_THINFILMNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

namespace Type
{
    // Type declaration for thinfilm data
    extern const TypeDesc* THINFILM;
}

/// Thin-Film node.
class ThinFilmNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void addInputs(ShaderNode& node, GenContext&) const override;

    static void addThinFilmSupport(ShaderNode& node);

    /// String constants
    static const string THICKNESS;
    static const string IOR;
    static const string THINFILM_INPUT;
};

} // namespace MaterialX

#endif
