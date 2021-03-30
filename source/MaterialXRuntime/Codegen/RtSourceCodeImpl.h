//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSOURCECODEIMPL_H
#define MATERIALX_RTSOURCECODEIMPL_H

/// @file RtSourceCodeImpl.h
/// TODO: Docs

#include <MaterialXRuntime/Codegen/RtCodegenImpl.h>

namespace MaterialX
{

/// @class RtSourceCodeImpl
/// Node implementation using shader source code.
class RtSourceCodeImpl : public RtCodegenImpl
{
    DECLARE_TYPED_SCHEMA(RtSourceCodeImpl)

public:
    /// Constructor.
    RtSourceCodeImpl(const RtPrim& prim) : RtCodegenImpl(prim) {}

    /// Set a file containing the source code to use for this implementation.
    void setFile(const string& file);

    /// Return a file containing the source code used by this implementation.
    const string& getFile() const;

    /// Set a string containing the source code to use for this implementation.
    void setSourceCode(const string& source);

    /// Return a string containing the source code used by this implementation.
    const string& getSourceCode() const;

    /// Set the format used by the source code in this implementation.
    void setFormat(const RtIdentifier& format);

    /// Return the format used by the source code in this implementation.
    const RtIdentifier& getFormat() const;

    /// Set the function name to use for this implementation.
    void setFunction(const string& function);

    /// Return the function name to use for this implementation.
    const string& getFunction() const;

    /// Emit function definition for the given node instance in the given context.
    void emitFunctionDefinition(const RtNode& node, GenContext& context, ShaderStage& stage) const override;

    /// Emit the function call or inline source code for given node instance in the given context.
    void emitFunctionCall(const RtNode& node, GenContext& context, ShaderStage& stage) const override;
};

}

#endif
