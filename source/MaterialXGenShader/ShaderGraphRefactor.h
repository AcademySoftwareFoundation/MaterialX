//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHREFACTOR_H
#define MATERIALX_SHADERGRAPHREFACTOR_H

/// @file
/// Shader graph refactoring passes

#include <MaterialXGenShader/Export.h>

MATERIALX_NAMESPACE_BEGIN

class GenContext;
class ShaderGraph;

/// A shared pointer to a shader graph refactor
using ShaderGraphRefactorPtr = shared_ptr<class ShaderGraphRefactor>;

/// @class ShaderGraphRefactor
/// Base class for shader graph refactoring passes.
/// Each pass identifies a structural pattern in the shader graph and
/// rewrites it into a mathematically equivalent form better suited
/// to the target backend.
class MX_GENSHADER_API ShaderGraphRefactor
{
  public:
    virtual ~ShaderGraphRefactor() { }

    /// Return the name of this refactoring pass.
    virtual const string& getName() const = 0;

    /// Execute the pass on the given graph.
    /// Return the number of graph edits made, or zero if
    /// the pass is not applicable to the current context.
    virtual size_t execute(ShaderGraph& graph, GenContext& context) = 0;
};

/// @class NodeElisionRefactor
/// Removes constant and dot nodes by bypassing them.
/// Constant nodes have their values moved downstream, and
/// dot nodes with filename-typed inputs are elided to prevent
/// extra samplers.
class MX_GENSHADER_API NodeElisionRefactor : public ShaderGraphRefactor
{
  public:
    const string& getName() const override;
    size_t execute(ShaderGraph& graph, GenContext& context) override;
};

/// @class PremultipliedBsdfAddRefactor
/// Replaces BSDF mix nodes with premultiplied add nodes.
/// Transforms mix(A, B, w) into add(A*w, B*(1-w)) by folding
/// the mix weight into each BSDF's weight input, enabling
/// hardware shading languages to skip BSDF evaluation when
/// the weight is zero.
class MX_GENSHADER_API PremultipliedBsdfAddRefactor : public ShaderGraphRefactor
{
  public:
    const string& getName() const override;
    size_t execute(ShaderGraph& graph, GenContext& context) override;
};

/// @class DistributeLayerOverMixRefactor
/// Distributes layer operations over mix nodes.
/// Transforms layer(mix(A, B, w), C) into mix(layer(A, C), layer(B, C), w)
/// to satisfy backends that cannot handle a mixed BSDF as the top
/// operand of a layer node.
class MX_GENSHADER_API DistributeLayerOverMixRefactor : public ShaderGraphRefactor
{
  public:
    const string& getName() const override;
    size_t execute(ShaderGraph& graph, GenContext& context) override;
};

MATERIALX_NAMESPACE_END

#endif
