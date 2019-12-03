//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>

#include <MaterialXRuntime/Private/PvtElement.h>

#include <sstream>

#ifndef _WIN32
// Disable warnings for breaking strict aliasing.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace MaterialX
{

RtValue::RtValue(const Matrix33& v, RtObject& owner)
{
    // Allocate storage for the value.
    PvtAllocator& allocator = owner.data()->asA<PvtElement>()->getAllocator();
    *reinterpret_cast<Matrix33**>(&_data) = allocator.allocType<Matrix33>();

    // Copy the value.
    asMatrix33() = v;
}

RtValue::RtValue(const Matrix44& v, RtObject& owner)
{
    // Allocate storage for the value.
    PvtAllocator& allocator = owner.data()->asA<PvtElement>()->getAllocator();
    *reinterpret_cast<Matrix44**>(&_data) = allocator.allocType<Matrix44>();

    // Copy the value.
    asMatrix44() = v;
}

RtValue::RtValue(const string& v, RtObject& owner)
{
    // Allocate storage for the value.
    PvtAllocator& allocator = owner.data()->asA<PvtElement>()->getAllocator();
    *reinterpret_cast<string**>(&_data) = allocator.allocType<string>();

    // Copy the value.
    asString() = v;
}

RtValue RtValue::createNew(const RtToken& type, RtObject owner)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    return typeDef->createValue(owner);
}

void RtValue::toString(const RtToken& type, const RtValue& src, string& dest)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    typeDef->toStringValue(src, dest);
}

void RtValue::fromString(const RtToken& type, const string& src, RtValue& dest)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    typeDef->fromStringValue(src, dest);
}

void RtValue::copy(const RtToken& type, const RtValue& src, RtValue& dest)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    typeDef->copyValue(src, dest);
}

}

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
