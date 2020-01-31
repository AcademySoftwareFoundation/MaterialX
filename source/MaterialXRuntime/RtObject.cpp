//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RtObject::RtObject() :
    _hnd(nullptr)
{
}

RtObject::RtObject(const RtObject& other) :
    _hnd(other._hnd)
{
}

RtObject::RtObject(PvtDataHandle data) :
    _hnd(data)
{
}

RtObject::~RtObject()
{
}

RtObjType RtObject::getObjType() const
{
    return _hnd ? _hnd->getObjType() : RtObjType::OBJECT;
}

const RtToken& RtObject::getName() const
{
    return hnd()->asA<PvtObject>()->getName();
}

RtPath RtObject::getPath() const
{
    return RtPath(hnd()->obj());
}

RtPrim RtObject::getParent() const
{
    PvtPrim* parent = hnd()->asA<PvtObject>()->getParent();
    return parent ? parent->hnd() : RtPrim();
}

RtPrim RtObject::getRoot() const
{
    PvtPrim* root = hnd()->asA<PvtObject>()->getRoot();
    return root ? root->hnd() : RtPrim();
}

RtStageWeakPtr RtObject::getStage() const
{
    return hnd()->asA<PvtObject>()->getStage();
}

RtTypedValue* RtObject::addMetadata(const RtToken& name, const RtToken& type)
{
    return hnd()->asA<PvtObject>()->addMetadata(name, type);
}

void RtObject::removeMetadata(const RtToken& name)
{
    hnd()->asA<PvtObject>()->removeMetadata(name);
}

RtTypedValue* RtObject::getMetadata(const RtToken& name)
{
    return hnd()->asA<PvtObject>()->getMetadata(name);
}

bool RtObject::_canCast(RtObjType fromType, RtObjType toType)
{
    // Hard coded registery as the RtObject class hierarchy
    // will not change, or change very rarely.
    static const std::set<RtObjType> s_objTypeRegistry[size_t(RtObjType::NUM_TYPES)] =
    {
        {RtObjType::OBJECT                                          },
        {RtObjType::OBJECT, RtObjType::PRIM                         },
        {RtObjType::OBJECT, RtObjType::ATTRIBUTE                    },
        {RtObjType::OBJECT, RtObjType::ATTRIBUTE, RtObjType::INPUT  },
        {RtObjType::OBJECT, RtObjType::ATTRIBUTE, RtObjType::OUTPUT },
        {RtObjType::OBJECT, RtObjType::RELATIONSHIP                 }
    };

    return s_objTypeRegistry[size_t(fromType)].count(toType);
}

}
