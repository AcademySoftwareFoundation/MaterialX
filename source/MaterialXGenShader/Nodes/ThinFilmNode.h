//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_THINFILMNODE_H
#define MATERIALX_THINFILMNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>

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

    /// String constants
    static const string THICKNESS;
    static const string IOR;
};

/// Base class for microfacet BSDF nodes that support layering with thin-film.
/// Thin-film data is added as an extra input to BSDF nodes that derive from
/// this class.
class ThinFilmSupport : public HwSourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;

    /// String constants
    static const string THINFILM_INPUT;
};

} // namespace MaterialX

#endif
