//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTargetDef.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // Private implementation of targetdef prims.
    class PvtTargetDefPrim : public PvtPrim
    {
    public:
        PvtTargetDefPrim(const RtTypeInfo* typeInfo, const RtString& name, PvtPrim* parent) 
            : PvtPrim(typeInfo, name, parent)
        {}

        RtStringSet matchingTargets;
    };

    // TODO: We should derive this from a data driven XML schema.
    class PvtTargetDefPrimSpec : public PvtPrimSpec
    {
    public:
        PvtTargetDefPrimSpec()
        {
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::INHERIT, RtType::INTERNSTRING);

        }
    };
}

DEFINE_TYPED_SCHEMA(RtTargetDef, "targetdef");

RtPrim RtTargetDef::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtString DEFAULT_NAME("targetdef1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew<PvtTargetDefPrim>(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->asA<PvtTargetDefPrim>()->matchingTargets.insert(name);

    return primH;
}

const RtPrimSpec& RtTargetDef::getPrimSpec() const
{
    static const PvtTargetDefPrimSpec s_primSpec;
    return s_primSpec;
}

void RtTargetDef::setInherit(const RtString& target)
{
    RtTypedValue* attr = createAttribute(RtString::INHERIT, RtType::INTERNSTRING);
    attr->asInternString() = target;

    prim()->asA<PvtTargetDefPrim>()->matchingTargets.insert(target);
}

const RtString& RtTargetDef::getInherit() const
{
    const RtTypedValue* attr = getAttribute(RtString::INHERIT, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

bool RtTargetDef::isMatching(const RtString& target) const
{
    return prim()->asA<PvtTargetDefPrim>()->matchingTargets.count(target) != 0;
}

}
