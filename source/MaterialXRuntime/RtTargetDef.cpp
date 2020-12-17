//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTargetDef.h>
#include <MaterialXRuntime/Tokens.h>

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
        PvtTargetDefPrim(const RtTypeInfo* typeInfo, const RtToken& name, PvtPrim* parent) 
            : PvtPrim(typeInfo, name, parent)
        {}

        RtTokenSet matchingTargets;
    };
}

DEFINE_TYPED_SCHEMA(RtTargetDef, "targetdef");

RtPrim RtTargetDef::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("targetdef1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew<PvtTargetDefPrim>(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->asA<PvtTargetDefPrim>()->matchingTargets.insert(name);

    return primH;
}

void RtTargetDef::setInherit(const RtToken& target)
{
    RtTypedValue* data = addMetadata(Tokens::INHERIT, RtType::TOKEN);
    data->getValue().asToken() = target;

    prim()->asA<PvtTargetDefPrim>()->matchingTargets.insert(target);
}

const RtToken& RtTargetDef::getInherit() const
{
    const RtTypedValue* data = getMetadata(Tokens::INHERIT, RtType::TOKEN);
    return data ? data->getValue().asToken() : EMPTY_TOKEN;
}

bool RtTargetDef::isMatching(const RtToken& target) const
{
    return prim()->asA<PvtTargetDefPrim>()->matchingTargets.count(target) != 0;
}

}
