//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Codegen/RtSourceCodeImpl.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtSourceCodeImpl, "nodeimpl:sourcecodeimpl");

RtPrim RtSourceCodeImpl::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtString DEFAULT_NAME("sourcecodeimpl1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtSourceCodeImpl::getPrimSpec() const
{
    static const PvtPrimSpec s_primSpec;
    return s_primSpec;
}

void RtSourceCodeImpl::setFile(const string& file)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::FILE, RtType::STRING);
    attr->asString() = file;

    const FilePath path = RtApi::get().getSearchPath().find(file);
    string source = readFile(path);
    if (source.empty())
    {
        throw ExceptionShaderGenError("Failed to get source code from file '" + path.asString() +
            "' used by implementation '" + getName().str() + "'");
    }
    setSourceCode(source);
}

const string& RtSourceCodeImpl::getFile() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::FILE, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtSourceCodeImpl::setSourceCode(const string& source)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::SOURCECODE, RtType::STRING);
    attr->asString() = source;
}

const string& RtSourceCodeImpl::getSourceCode() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::SOURCECODE, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtSourceCodeImpl::setFormat(const RtString& format)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::FORMAT, RtType::INTERNSTRING);
    attr->asInternString() = format;
}

const RtString& RtSourceCodeImpl::getFormat() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::FORMAT, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::SHADER;
}

void RtSourceCodeImpl::setFunction(const string& source)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::FUNCTION, RtType::STRING);
    attr->asString() = source;
}

const string& RtSourceCodeImpl::getFunction() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::FUNCTION, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtSourceCodeImpl::emitFunctionDefinition(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

void RtSourceCodeImpl::emitFunctionCall(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

}
