//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_THINFILMNODE_H
#define MATERIALX_THINFILMNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>

namespace MaterialX
{

namespace Type
{
    // Type declaration for thinfilm data
    extern MX_GENSHADER_API const TypeDesc* THINFILM;
}

/// Thin-Film node.
class MX_GENSHADER_API ThinFilmNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    static void addThinFilmSupport(ShaderNode& node);

    /// String constants
    static const string THICKNESS;
    static const string IOR;
    static const string THINFILM_INPUT;
};

/// BSDF with thin-film support.
class MX_GENSHADER_API BsdfWithThinFilm : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create()
    {
        return std::make_shared<BsdfWithThinFilm>();
    }

    void addInputs(ShaderNode& node, GenContext&) const override
    {
        // Add thin-film support.
        ThinFilmNode::addThinFilmSupport(node);
    }
};

/// BSDF with thin-film support specifically for HW.
class MX_GENSHADER_API HwBsdfWithThinFilm : public HwSourceCodeNode
{
public:
    static ShaderNodeImplPtr create()
    {
        return std::make_shared<HwBsdfWithThinFilm>();
    }

    void addInputs(ShaderNode& node, GenContext&) const override
    {
        // Add thin-film support.
        ThinFilmNode::addThinFilmSupport(node);
    }
};


} // namespace MaterialX

#endif
