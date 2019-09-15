//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SOURCECODENODE_H
#define MATERIALX_SOURCECODENODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Node implementation using data-driven static source code.
/// This is the default implementation used for all nodes that 
/// do not have a custom ShaderNodeImpl class.
class SourceCodeNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    bool _inlined;
    string _functionName;
    string _functionSource;
};

} // namespace MaterialX

#endif
