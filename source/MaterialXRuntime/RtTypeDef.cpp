//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTypeDef.h>

#include <MaterialXRuntime/Private/PvtTypeDef.h>

namespace MaterialX
{

const RtToken RtType::BOOLEAN("boolean");
const RtToken RtType::INTEGER("integer");
const RtToken RtType::FLOAT("float");
const RtToken RtType::VECTOR2("vector2");
const RtToken RtType::VECTOR3("vector3");
const RtToken RtType::VECTOR4("vector4");
const RtToken RtType::COLOR3("color3");
const RtToken RtType::COLOR4("color4");
const RtToken RtType::MATRIX33("matrix33");
const RtToken RtType::MATRIX44("matrix44");
const RtToken RtType::TOKEN("token");
const RtToken RtType::STRING("string");
const RtToken RtType::FILENAME("filename");
const RtToken RtType::INTEGERARRAY("integerarray");
const RtToken RtType::FLOATARRAY("floatarray");
const RtToken RtType::COLOR3ARRAY("color3array");
const RtToken RtType::COLOR4ARRAY("color4array");
const RtToken RtType::VECTOR2ARRAY("vector2array");
const RtToken RtType::VECTOR3ARRAY("vector3array");
const RtToken RtType::VECTOR4ARRAY("vector4array");
const RtToken RtType::STRINGARRAY("stringarray");
const RtToken RtType::BSDF("BSDF");
const RtToken RtType::EDF("EDF");
const RtToken RtType::VDF("VDF");
const RtToken RtType::SURFACESHADER("surfaceshader");
const RtToken RtType::VOLUMESHADER("volumeshader");
const RtToken RtType::DISPLACEMENTSHADER("displacementshader");
const RtToken RtType::LIGHTSHADER("lightshader");
const RtToken RtType::MATERIAL("material");
const RtToken RtType::AUTO("auto");

const RtToken RtTypeDef::BASETYPE_NONE("none");
const RtToken RtTypeDef::BASETYPE_BOOLEAN("boolean");
const RtToken RtTypeDef::BASETYPE_FLOAT("float");
const RtToken RtTypeDef::BASETYPE_INTEGER("integer");
const RtToken RtTypeDef::BASETYPE_STRING("string");
const RtToken RtTypeDef::BASETYPE_STRUCT("struct");

const RtToken RtTypeDef::SEMANTIC_NONE("none");
const RtToken RtTypeDef::SEMANTIC_COLOR("color");
const RtToken RtTypeDef::SEMANTIC_VECTOR("vector");
const RtToken RtTypeDef::SEMANTIC_MATRIX("matrix");
const RtToken RtTypeDef::SEMANTIC_FILENAME("filename");
const RtToken RtTypeDef::SEMANTIC_CLOSURE("closure");
const RtToken RtTypeDef::SEMANTIC_SHADER("shader");
const RtToken RtTypeDef::SEMANTIC_MATERIAL("material");

RtTypeDef::RtTypeDef(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs, const RtToken& semantic, size_t size) :
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

const RtToken& RtTypeDef::getName() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getName();
}

const RtToken& RtTypeDef::getBaseType() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getBaseType();
}

const RtToken& RtTypeDef::getSemantic() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getSemantic();
}

size_t RtTypeDef::getSize() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getSize();
}

void RtTypeDef::setComponent(size_t index, const RtToken& name, const RtToken& basetype)
{
    static_cast<PvtTypeDef*>(_ptr)->setComponent(index, name, basetype);
}

size_t RtTypeDef::getComponentIndex(const RtToken& name) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentIndex(name);
}

const RtToken& RtTypeDef::getComponentName(size_t index) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentName(index);
}

const RtToken& RtTypeDef::getComponentBaseType(size_t index) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentBaseType(index);
}

const RtTokenSet& RtTypeDef::getValidConnectionTypes() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValidConnectionTypes();
}

RtTypeDef* RtTypeDef::registerType(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs,
                                   const RtToken& semantic, size_t size)
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

const RtTypeDef* RtTypeDef::findType(const RtToken& name)
{
    return PvtTypeDefRegistry::get().findType(name);
}

}
