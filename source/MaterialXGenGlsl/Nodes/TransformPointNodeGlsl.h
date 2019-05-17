//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TRANSFORMPOINTNODEGLSL_H
#define MATERIALX_TRANSFORMPOINTNODEGLSL_H

#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>

namespace MaterialX
{

/// Implementation of transformspoint node for GLSL
class TransformPointNodeGlsl : public TransformVectorNodeGlsl
{
public:
    static ShaderNodeImplPtr create();

protected:
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const;
};

} // namespace MaterialX

#endif
