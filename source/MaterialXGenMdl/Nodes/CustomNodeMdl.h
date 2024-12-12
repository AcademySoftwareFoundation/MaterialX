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

/// Node to handle user defined implementations in external MDL files or using the inline `sourcecode` attribute.
class MX_GENMDL_API CustomCodeNodeMdl : public SourceCodeNodeMdl
{
  public:
    static ShaderNodeImplPtr create();
    void initialize(const InterfaceElement& element, GenContext& context) override;
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// Get the MDL qualified name of the externally references user module.
    /// It's used for import statements and functions calls in the generated target code.
    const string& getQualifiedModuleName() const;

    /// To avoid collisions with reserved names in MDL, input and output names are prefixed.
    /// In the `sourcecode` case all inputs and outputs are prefixed so authors don't need knowledge about reserved words in MDL.
    /// In the `file` and `function` case, only reserved names are prefixed to support existing MDL implementations without changes.
    string modifyPortName(const string& name, const MdlSyntax& syntax) const;

  protected:
    /// Initialize function for nodes that use the inline `sourcecode` attribute.
    void initializeForInlineSourceCode(const InterfaceElement& element, GenContext& context);

    /// Initialize function for nodes that use the `file` and `function` attribute. 
    void initializeForExternalSourceCode(const InterfaceElement& element, GenContext& context);

    /// Computes the function call string with replacement markers use by base class.
    void initializeFunctionCallTemplateString(const MdlSyntax& syntax, const NodeDef& node);

    /// Keep track of the default values needed for the inline `sourcecode` case.
    void initializeOutputDefaults(const MdlSyntax& syntax, const NodeDef& node);

    std::vector<ValuePtr> _outputDefaults; ///< store default values of the node definition

    bool _useExternalSourceCode; // Indicates that `file` and `function` are used by this node implementation
    string _inlineFunctionName; // Name of the functionDefinition to emit
    string _inlineSourceCode; // The actual inline source code
    string _qualifiedModuleName; // MDL qualified name derived from the `file` attribute
};

MATERIALX_NAMESPACE_END

#endif
