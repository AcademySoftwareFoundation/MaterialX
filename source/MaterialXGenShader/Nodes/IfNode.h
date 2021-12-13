//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_IFNODE_H
#define MATERIALX_IFNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

/// @class IfNode
/// Abstract base class for implementions which handle if conditions.
class MX_GENSHADER_API IfNode : public ShaderNodeImpl
{
  public:
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  private:
    /// Provides the shader code equality operator string to use
    virtual const string& equalityString() const = 0;

    static const StringVec INPUT_NAMES;
};

/// @class IfGreaterNode
/// "ifgreater" node implementation
class MX_GENSHADER_API IfGreaterNode : public IfNode
{
  public:
    static ShaderNodeImplPtr create();
  private:
    const string& equalityString() const override
    {
        return EQUALITY_STRING;
    }
    static string EQUALITY_STRING;
};

/// @class IfGreaterEqNode 
/// "ifgreatereq" node implementation
class MX_GENSHADER_API IfGreaterEqNode : public IfNode
{
  public:
    static ShaderNodeImplPtr create();
  private:
    const string& equalityString() const override
    {
        return EQUALITY_STRING;
    }
    static string EQUALITY_STRING;
};

/// @class IfEqualNode 
/// "ifequal" node implementation
class MX_GENSHADER_API IfEqualNode : public IfNode
{
  public:
    static ShaderNodeImplPtr create();
  private:
    const string& equalityString() const override
    {
        return EQUALITY_STRING;
    }
    static string EQUALITY_STRING;
};

MATERIALX_NAMESPACE_END

#endif
