//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWHEIGHTTONORMALNODE_H
#define MATERIALX_HWHEIGHTTONORMALNODE_H

#include <MaterialXGenShader/HwShaderGenerator.h>

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

MATERIALX_NAMESPACE_BEGIN

/// HeightToNormal node implementation for Hw shader languages
class MX_GENSHADER_API HwHeightToNormalNode : public ConvolutionNode
{
  public:
    HwHeightToNormalNode(const string& samplingIncludeFilename) : _samplingIncludeFilename(samplingIncludeFilename) {}
    virtual ~HwHeightToNormalNode() {}

    static ShaderNodeImplPtr create(const string& samplingIncludeFilename);

    void createVariables(const ShaderNode&, GenContext&, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    /// Return if given type is an acceptable input
    bool acceptsInputType(TypeDesc type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString,
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;

  private:
    const string _samplingIncludeFilename;
};

MATERIALX_NAMESPACE_END

#endif
