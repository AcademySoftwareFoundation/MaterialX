//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTYPE_H
#define MATERIALX_RTTYPE_H

/// @file
/// Identifiers for runtime data types.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtIdentifier.h>

namespace MaterialX
{

/// Class holding identifiers for the built in data types.
class RtType
{
public:
    static const RtIdentifier BOOLEAN;
    static const RtIdentifier INTEGER;
    static const RtIdentifier FLOAT;
    static const RtIdentifier VECTOR2;
    static const RtIdentifier VECTOR3;
    static const RtIdentifier VECTOR4;
    static const RtIdentifier COLOR3;
    static const RtIdentifier COLOR4;
    static const RtIdentifier MATRIX33;
    static const RtIdentifier MATRIX44;
    static const RtIdentifier IDENTIFIER;
    static const RtIdentifier STRING;
    static const RtIdentifier FILENAME;
    static const RtIdentifier INTEGERARRAY;
    static const RtIdentifier FLOATARRAY;
    static const RtIdentifier COLOR3ARRAY;
    static const RtIdentifier COLOR4ARRAY;
    static const RtIdentifier VECTOR2ARRAY;
    static const RtIdentifier VECTOR3ARRAY;
    static const RtIdentifier VECTOR4ARRAY;
    static const RtIdentifier STRINGARRAY;
    static const RtIdentifier BSDF;
    static const RtIdentifier EDF;
    static const RtIdentifier VDF;
    static const RtIdentifier SURFACESHADER;
    static const RtIdentifier VOLUMESHADER;
    static const RtIdentifier DISPLACEMENTSHADER;
    static const RtIdentifier LIGHTSHADER;
    static const RtIdentifier MATERIAL;
    static const RtIdentifier AUTO;
};

} // namespace MaterialX

#endif
