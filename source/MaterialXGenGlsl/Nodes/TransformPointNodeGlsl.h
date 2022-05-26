//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TRANSFORMPOINTNODEGLSL_H
#define MATERIALX_TRANSFORMPOINTNODEGLSL_H

#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformPoint node implementation for GLSL
class MX_GENGLSL_API TransformPointNodeGlsl : public TransformVectorNodeGlsl
{
public:
    static ShaderNodeImplPtr create();

protected:
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const override;
};

MATERIALX_NAMESPACE_END

#endif
