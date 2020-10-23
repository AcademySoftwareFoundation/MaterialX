//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtPrim.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

#include <sstream>

namespace MaterialX
{

RtValue::RtValue(const Matrix33& v, RtPrim& prim)
{
    // Allocate storage for the value.
    PvtAllocator& allocator = PvtObject::ptr<PvtPrim>(prim)->getAllocator();
    *_reinterpret_cast<Matrix33**>(&_data) = allocator.allocType<Matrix33>();

    // Copy the value.
    asMatrix33() = v;
}

RtValue::RtValue(const Matrix44& v, RtPrim& prim)
{
    // Allocate storage for the value.
    PvtAllocator& allocator = PvtObject::ptr<PvtPrim>(prim)->getAllocator();
    *_reinterpret_cast<Matrix44**>(&_data) = allocator.allocType<Matrix44>();

    // Copy the value.
    asMatrix44() = v;
}

RtValue::RtValue(const string& v, RtPrim& prim)
{
    // Allocate storage for the value.
    PvtAllocator& allocator = PvtObject::ptr<PvtPrim>(prim)->getAllocator();
    *_reinterpret_cast<string**>(&_data) = allocator.allocType<string>();

    // Copy the value.
    asString() = v;
}

RtValue RtValue::createNew(const RtToken& type, RtPrim owner)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    return typeDef->createValue(owner);
}

RtValue RtValue::clone(const RtToken& type, const RtValue& value, RtPrim owner)
{
    RtValue clonedValue = createNew(type, owner);
    copy(type, value, clonedValue);
    return clonedValue;
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

bool RtValue::compare(const RtToken& type, const RtValue& a, const RtValue& b)
{
    const RtTypeDef* typeDef = RtTypeDef::findType(type);
    if (!typeDef)
    {
        throw ExceptionRuntimeError("Type '" + type.str() + "' is not a registered type");
    }
    return typeDef->compareValue(a, b);
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

}
