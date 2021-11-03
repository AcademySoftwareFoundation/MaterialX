//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURELAYERNODEOSL_H
#define MATERIALX_CLOSURELAYERNODEOSL_H

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

namespace MaterialX
{

/// Closure layer node implementation for OSL.
class MX_GENOSL_API ClosureLayerNodeOsl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string TOP;
    static const string BASE;
    static const string THICKNESS;
    static const string IOR;
};

} // namespace MaterialX

#endif
