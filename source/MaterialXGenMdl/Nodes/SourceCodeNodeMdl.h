//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SOURCECODEMDL_H
#define MATERIALX_SOURCECODEMDL_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

/// Node implementation using data-driven static source code.
/// This is the default implementation used for all nodes that 
/// do not have a custom ShaderNodeImpl class.
class SourceCodeNodeMdl : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;
    void emitFunctionDefinition(const ShaderNode&, GenContext&, ShaderStage&) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    string _returnStruct;
};

} // namespace MaterialX

#endif
