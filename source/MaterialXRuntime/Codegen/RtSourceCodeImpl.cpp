//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Codegen/RtSourceCodeImpl.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXFormat/Util.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtSourceCodeImpl, "nodeimpl:sourcecodeimpl");

RtPrim RtSourceCodeImpl::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("sourcecodeimpl1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    return primH;
}

void RtSourceCodeImpl::setFile(const string& file)
{
    PvtPrim* p = prim();
    RtTypedValue* v = p->addMetadata(Tokens::FILE, RtType::STRING);
    v->getValue().asString() = file;

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
    RtTypedValue* v = prim()->getMetadata(Tokens::FILE, RtType::STRING);
    return v ? v->getValue().asString() : EMPTY_STRING;
}

void RtSourceCodeImpl::setSourceCode(const string& source)
{
    PvtPrim* p = prim();
    RtTypedValue* v = p->addMetadata(Tokens::SOURCECODE, RtType::STRING);
    v->getValue().asString() = source;
}

const string& RtSourceCodeImpl::getSourceCode() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::SOURCECODE, RtType::STRING);
    return v ? v->getValue().asString() : EMPTY_STRING;
}

void RtSourceCodeImpl::setFormat(const RtToken& format)
{
    PvtPrim* p = prim();
    RtTypedValue* v = p->addMetadata(Tokens::FORMAT, RtType::TOKEN);
    v->getValue().asToken() = format;
}

const RtToken& RtSourceCodeImpl::getFormat() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::FORMAT, RtType::TOKEN);
    return v ? v->getValue().asToken() : Tokens::SHADER;
}

void RtSourceCodeImpl::setFunction(const string& source)
{
    PvtPrim* p = prim();
    RtTypedValue* v = p->addMetadata(Tokens::FUNCTION, RtType::STRING);
    v->getValue().asString() = source;
}

const string& RtSourceCodeImpl::getFunction() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::FUNCTION, RtType::STRING);
    return v ? v->getValue().asString() : EMPTY_STRING;
}

void RtSourceCodeImpl::emitFunctionDefinition(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

void RtSourceCodeImpl::emitFunctionCall(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

}
