//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeImpl.h>

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
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::NODEDEF, RtType::INTERNSTRING);
            addPrimAttribute(RtString::TARGET, RtType::INTERNSTRING);
            addPrimAttribute(RtString::FILE, RtType::STRING);
            addPrimAttribute(RtString::SOURCECODE, RtType::STRING);
            addPrimAttribute(RtString::FUNCTION, RtType::STRING);
            addPrimAttribute(RtString::FORMAT, RtType::INTERNSTRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeImpl, "nodeimpl");

RtPrim RtNodeImpl::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtString DEFAULT_NAME("nodeimpl1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeImpl::getPrimSpec() const
{
    static const PvtNodeImplPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeImpl::setTarget(const RtString& target)
{
    RtTypedValue* attr = createAttribute(RtString::TARGET, RtType::INTERNSTRING);
    attr->getValue().asInternString() = target;
}

const RtString& RtNodeImpl::getTarget() const
{
    const RtTypedValue* attr = getAttribute(RtString::TARGET, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeImpl::setNodeDef(const RtString& language)
{
    RtTypedValue* attr = createAttribute(RtString::NODEDEF, RtType::INTERNSTRING);
    attr->asInternString() = language;
}

const RtString& RtNodeImpl::getNodeDef() const
{
    const RtTypedValue* attr = getAttribute(RtString::NODEDEF, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeImpl::setImplName(const RtString& implname)
{
    RtTypedValue* attr = createAttribute(RtString::IMPLNAME, RtType::INTERNSTRING);
    attr->asInternString() = implname;
}

const RtString& RtNodeImpl::getImplName() const
{
    const RtTypedValue* attr = getAttribute(RtString::IMPLNAME, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

}
