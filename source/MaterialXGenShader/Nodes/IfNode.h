//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_IFNODE_H
#define MATERIALX_IFNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// @class IfNode
/// Abstract base class for implementions which handle if conditions.
class IfNode : public ShaderNodeImpl
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
class IfGreaterNode : public IfNode
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
class IfGreaterEqNode : public IfNode
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
class IfEqualNode : public IfNode
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

} // namespace MaterialX

#endif
