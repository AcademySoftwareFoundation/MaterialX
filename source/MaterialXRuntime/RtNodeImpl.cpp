//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/Identifiers.h>

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
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::NODEDEF, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::FILE, RtType::STRING);
            addPrimAttribute(Identifiers::SOURCECODE, RtType::STRING);
            addPrimAttribute(Identifiers::FUNCTION, RtType::STRING);
            addPrimAttribute(Identifiers::FORMAT, RtType::IDENTIFIER);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeImpl, "nodeimpl");

RtPrim RtNodeImpl::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtIdentifier DEFAULT_NAME("nodeimpl1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeImpl::getPrimSpec() const
{
    static const PvtNodeImplPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeImpl::setTarget(const RtIdentifier& target)
{
    RtTypedValue* attr = createAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
    attr->getValue().asIdentifier() = target;
}

const RtIdentifier& RtNodeImpl::getTarget() const
{
    const RtTypedValue* attr = getAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeImpl::setNodeDef(const RtIdentifier& language)
{
    RtTypedValue* attr = createAttribute(Identifiers::NODEDEF, RtType::IDENTIFIER);
    attr->asIdentifier() = language;
}

const RtIdentifier& RtNodeImpl::getNodeDef() const
{
    const RtTypedValue* attr = getAttribute(Identifiers::NODEDEF, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeImpl::setImplName(const RtIdentifier& implname)
{
    RtTypedValue* attr = createAttribute(Identifiers::IMPLNAME, RtType::IDENTIFIER);
    attr->asIdentifier() = implname;
}

const RtIdentifier& RtNodeImpl::getImplName() const
{
    const RtTypedValue* attr = getAttribute(Identifiers::IMPLNAME, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

}
