//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TRANSFORMNORMALNODEGLSL_H
#define MATERIALX_TRANSFORMNORMALNODEGLSL_H

#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>

namespace MaterialX
{

/// TransformNormal node implementation for GLSL
class TransformNormalNodeGlsl : public TransformVectorNodeGlsl
{
public:
    static ShaderNodeImplPtr create();

protected:
    virtual const string& getMatrix(const string& fromSpace, const string& toSpace) const;
};

} // namespace MaterialX

#endif
