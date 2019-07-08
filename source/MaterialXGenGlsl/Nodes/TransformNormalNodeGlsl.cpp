//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/TransformNormalNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TransformNormalNodeGlsl::create()
{
    return std::make_shared<TransformNormalNodeGlsl>();
}

const string& TransformNormalNodeGlsl::getMatrix(const string& fromSpace, const string& toSpace) const
{
    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        return HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX;
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        return HW::T_WORLD_TRANSPOSE_MATRIX;
    }
    return EMPTY_STRING;
}

} // namespace MaterialX
