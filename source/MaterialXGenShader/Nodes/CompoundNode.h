//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_COMPOUNDNODE_H
#define MATERIALX_COMPOUNDNODE_H

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/Shader.h>

#include <memory>

MATERIALX_NAMESPACE_BEGIN

class NodeGraphPermutation;

/// Compound node implementation
class MX_GENSHADER_API CompoundNode : public ShaderNodeImpl
{
  public:
    /// Create a CompoundNode with a permutation (may be nullptr).
    /// @param permutation The permutation for this instance (ownership transferred)
    static ShaderNodeImplPtr create(std::unique_ptr<NodeGraphPermutation> permutation);

    ~CompoundNode() override;

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void addClassification(ShaderNode& node) const override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    ShaderGraph* getGraph() const override { return _rootGraph.get(); }

    /// Return the permutation (if any) for this compound node.
    const NodeGraphPermutation* getPermutation() const { return _permutation.get(); }

    explicit CompoundNode(std::unique_ptr<NodeGraphPermutation> permutation);

  protected:
    ShaderGraphPtr _rootGraph;
    string _functionName;
    std::unique_ptr<NodeGraphPermutation> _permutation;
};

MATERIALX_NAMESPACE_END

#endif
