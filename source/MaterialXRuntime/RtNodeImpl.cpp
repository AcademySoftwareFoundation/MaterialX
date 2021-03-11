//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{
namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodeImplPrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodeImplPrimSpec()
        {
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::NODEDEF, RtType::TOKEN);
            addPrimAttribute(Tokens::TARGET, RtType::TOKEN);
            addPrimAttribute(Tokens::FILE, RtType::STRING);
            addPrimAttribute(Tokens::SOURCECODE, RtType::STRING);
            addPrimAttribute(Tokens::FUNCTION, RtType::STRING);
            addPrimAttribute(Tokens::FORMAT, RtType::TOKEN);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeImpl, "nodeimpl");

RtPrim RtNodeImpl::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("nodeimpl1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeImpl::getPrimSpec() const
{
    static const PvtNodeImplPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeImpl::setTarget(const RtToken& target)
{
    RtTypedValue* attr = createAttribute(Tokens::TARGET, RtType::TOKEN);
    attr->getValue().asToken() = target;
}

const RtToken& RtNodeImpl::getTarget() const
{
    const RtTypedValue* attr = getAttribute(Tokens::TARGET, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeImpl::setNodeDef(const RtToken& language)
{
    RtTypedValue* attr = createAttribute(Tokens::NODEDEF, RtType::TOKEN);
    attr->asToken() = language;
}

const RtToken& RtNodeImpl::getNodeDef() const
{
    const RtTypedValue* attr = getAttribute(Tokens::NODEDEF, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeImpl::setImplName(const RtToken& implname)
{
    RtTypedValue* attr = createAttribute(Tokens::IMPLNAME, RtType::TOKEN);
    attr->asToken() = implname;
}

const RtToken& RtNodeImpl::getImplName() const
{
    const RtTypedValue* attr = getAttribute(Tokens::IMPLNAME, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

}
