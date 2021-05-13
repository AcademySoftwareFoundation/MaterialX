//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTypeDef.h>

#include <MaterialXRuntime/Private/PvtTypeDef.h>

namespace MaterialX
{

const RtString RtTypeDef::BASETYPE_NONE("none");
const RtString RtTypeDef::BASETYPE_BOOLEAN("boolean");
const RtString RtTypeDef::BASETYPE_FLOAT("float");
const RtString RtTypeDef::BASETYPE_INTEGER("integer");
const RtString RtTypeDef::BASETYPE_STRING("string");
const RtString RtTypeDef::BASETYPE_STRUCT("struct");

const RtString RtTypeDef::SEMANTIC_NONE("none");
const RtString RtTypeDef::SEMANTIC_COLOR("color");
const RtString RtTypeDef::SEMANTIC_VECTOR("vector");
const RtString RtTypeDef::SEMANTIC_MATRIX("matrix");
const RtString RtTypeDef::SEMANTIC_FILENAME("filename");
const RtString RtTypeDef::SEMANTIC_CLOSURE("closure");
const RtString RtTypeDef::SEMANTIC_SHADER("shader");
const RtString RtTypeDef::SEMANTIC_MATERIAL("material");

RtTypeDef::RtTypeDef(const RtString& name, const RtString& basetype, const RtValueFuncs& funcs, const RtString& semantic, size_t size) :
    _ptr(new PvtTypeDef(name, basetype, funcs, semantic, size))
{
}

RtTypeDef::~RtTypeDef()
{
    delete static_cast<PvtTypeDef*>(_ptr);
}

RtValue RtTypeDef::createValue(RtPrim& owner) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().create(owner);
}

void RtTypeDef::copyValue(const RtValue& src, RtValue& dest) const
{
    static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().copy(src, dest);
}

bool RtTypeDef::compareValue(const RtValue& a, const RtValue& b) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().compare(a, b);
}

void RtTypeDef::toStringValue(const RtValue& src, string& dest) const
{
    static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().toString(src, dest);
}

void RtTypeDef::fromStringValue(const string& src, RtValue& dest) const
{
    static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().fromString(src, dest);
}

const RtString& RtTypeDef::getName() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getName();
}

const RtString& RtTypeDef::getBaseType() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getBaseType();
}

const RtString& RtTypeDef::getSemantic() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getSemantic();
}

size_t RtTypeDef::getSize() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getSize();
}

void RtTypeDef::setComponent(size_t index, const RtString& name, const RtString& basetype)
{
    static_cast<PvtTypeDef*>(_ptr)->setComponent(index, name, basetype);
}

size_t RtTypeDef::getComponentIndex(const RtString& name) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentIndex(name);
}

const RtString& RtTypeDef::getComponentName(size_t index) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentName(index);
}

const RtString& RtTypeDef::getComponentBaseType(size_t index) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentBaseType(index);
}

const RtStringSet& RtTypeDef::getValidConnectionTypes() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValidConnectionTypes();
}

RtTypeDef* RtTypeDef::registerType(const RtString& name, const RtString& basetype, const RtValueFuncs& funcs,
                                   const RtString& semantic, size_t size)
{
    if (PvtTypeDefRegistry::get().findType(name))
    {
        throw ExceptionRuntimeError("A type named '" + name.str() + "' is already registered");
    }
    return PvtTypeDefRegistry::get().newType(name, basetype, funcs, semantic, size);
}

size_t RtTypeDef::numTypes()
{
    return PvtTypeDefRegistry::get().numTypes();
}

const RtTypeDef* RtTypeDef::getType(size_t index)
{
    return PvtTypeDefRegistry::get().getType(index);
}

const RtTypeDef* RtTypeDef::findType(const RtString& name)
{
    return PvtTypeDefRegistry::get().findType(name);
}

}
