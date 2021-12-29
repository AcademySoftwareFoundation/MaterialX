//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TRANSFORMNORMALNODEGLSL_H
#define MATERIALX_TRANSFORMNORMALNODEGLSL_H

#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformNormal node implementation for GLSL
class MX_GENGLSL_API TransformNormalNodeGlsl : public TransformVectorNodeGlsl
{
public:
    static ShaderNodeImplPtr create();

protected:
    virtual const string& getMatrix(const string& fromSpace, const string& toSpace) const;
};

MATERIALX_NAMESPACE_END

#endif
