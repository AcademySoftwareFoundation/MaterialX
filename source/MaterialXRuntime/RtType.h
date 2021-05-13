//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTYPE_H
#define MATERIALX_RTTYPE_H

/// @file
/// Identifiers for runtime data types.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtString.h>

namespace MaterialX
{

/// Class holding identifiers for the built in data types.
class RtType
{
public:
    static const RtString BOOLEAN;
    static const RtString INTEGER;
    static const RtString FLOAT;
    static const RtString VECTOR2;
    static const RtString VECTOR3;
    static const RtString VECTOR4;
    static const RtString COLOR3;
    static const RtString COLOR4;
    static const RtString MATRIX33;
    static const RtString MATRIX44;
    static const RtString INTERNSTRING;
    static const RtString STRING;
    static const RtString FILENAME;
    static const RtString INTEGERARRAY;
    static const RtString FLOATARRAY;
    static const RtString COLOR3ARRAY;
    static const RtString COLOR4ARRAY;
    static const RtString VECTOR2ARRAY;
    static const RtString VECTOR3ARRAY;
    static const RtString VECTOR4ARRAY;
    static const RtString STRINGARRAY;
    static const RtString BSDF;
    static const RtString EDF;
    static const RtString VDF;
    static const RtString SURFACESHADER;
    static const RtString VOLUMESHADER;
    static const RtString DISPLACEMENTSHADER;
    static const RtString LIGHTSHADER;
    static const RtString MATERIAL;
    static const RtString AUTO;
};

} // namespace MaterialX

#endif
