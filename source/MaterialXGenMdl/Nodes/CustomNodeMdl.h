//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_CUSTOMNODEMDL_H
#define MATERIALX_CUSTOMNODEMDL_H

#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>

MATERIALX_NAMESPACE_BEGIN

class MdlSyntax;
class NodeDef;

class MX_GENMDL_API CustomCodeNodeMdl : public SourceCodeNodeMdl
{
  public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    const string& getQualifiedModuleName() const;

  protected:
    void initializeForInlineSourceCode(const InterfaceElement& element, GenContext& context);
    void initializeForExternalSourceCode(const InterfaceElement& element, GenContext& context);
    void initializeFunctionCallTemplateString(const MdlSyntax& syntax, const NodeDef& node);
    void initializeOutputDefaults(const MdlSyntax& syntax, const NodeDef& node);

    std::vector<ValuePtr> _outputDefaults;

    bool _useExternalSourceCode;
    string _inlineFunctionName;
    string _inlineSourceCode;
    string _qualifiedModuleName;
};

MATERIALX_NAMESPACE_END

#endif
