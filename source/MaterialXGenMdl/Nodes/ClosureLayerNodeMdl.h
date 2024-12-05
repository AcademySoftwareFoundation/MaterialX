//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_CLOSURELAYERNODEMDL_H
#define MATERIALX_CLOSURELAYERNODEMDL_H

#include <MaterialXGenMdl/Export.h>
#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

/// Holds all constants required by the layering and its transformations.
class MX_GENMDL_API StringConstantsMdl
{
    StringConstantsMdl() = delete;

  public:
    /// String constants
    static const string TOP;  ///< layer parameter name of the top component
    static const string BASE; ///< layer parameter name of the base component
    static const string FG;   ///< mix parameter name of the foreground
    static const string BG;   ///< mix parameter name of the background
    static const string MIX;  ///< mix parameter name of the amount
    static const string TOP_WEIGHT; ///< mix amount forwarded into layer top component
};

/// Closure layer node implementation for MDL.
class MX_GENMDL_API ClosureLayerNodeMdl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

/// Layerable BSDF node.
/// Because MDL does not support vertical layering the nodes are transformed in a way that
/// the base node is passed as parameter to the top layer node.
/// Note, not all elemental bsdfs support this kind of transformation.
class MX_GENMDL_API LayerableNodeMdl : public SourceCodeNodeMdl
{
    using BASE = SourceCodeNodeMdl;

  public:
    virtual ~LayerableNodeMdl() = default;
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
    bool isEditable(const ShaderInput& input) const override;
};

MATERIALX_NAMESPACE_END

#endif
