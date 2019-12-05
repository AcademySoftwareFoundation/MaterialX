//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

RtStage::RtStage(const RtObject& obj) :
    RtElement(obj)
{
}

RtApiType RtStage::getApiType() const
{
    return RtApiType::STAGE;
}

RtObject RtStage::createNew(const RtToken& name)
{
    return PvtObject::object(PvtStage::createNew(name));
}

void RtStage::addReference(const RtObject& stage)
{
    data()->asA<PvtStage>()->addReference(PvtObject::data(stage));
}

void RtStage::removeReference(const RtToken& name)
{
    data()->asA<PvtStage>()->removeReference(name);
}

void RtStage::removeReferences()
{
    data()->asA<PvtStage>()->removeReferences();
}

size_t RtStage::numReferences() const
{
    return data()->asA<PvtStage>()->numReferences();
}

RtObject RtStage::getReference(size_t index) const
{
    PvtDataHandle ref = data()->asA<PvtStage>()->getReference(index);
    return PvtObject::object(ref);
}

RtObject RtStage::findReference(const RtToken& name) const
{
    PvtDataHandle ref = data()->asA<PvtStage>()->findReference(name);
    return PvtObject::object(ref);
}

void RtStage::addElement(const RtObject& elem)
{
    if (elem.hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("A stage cannot be added as direct child of another stage. Use addReference() instead to reference the stage.");
    }
    data()->asA<PvtStage>()->addChild(PvtObject::data(elem));
}

void RtStage::removeElement(const RtObject& elem)
{
    data()->asA<PvtStage>()->removeChild(PvtObject::ptr<PvtElement>(elem)->getName());
}

void RtStage::removeElementByPath(const RtPath& path)
{
    data()->asA<PvtStage>()->removeChildByPath(*static_cast<const PvtPath*>(path._ptr));
}

RtObject RtStage::findElementByName(const RtToken& name) const
{
    PvtDataHandle elem = data()->asA<PvtStage>()->findChildByName(name);
    return PvtObject::object(elem);
}

RtObject RtStage::findElementByPath(const string& path) const
{
    PvtDataHandle elem = data()->asA<PvtStage>()->findChildByPath(path);
    return PvtObject::object(elem);
}

RtStageIterator RtStage::traverseStage(RtTraversalFilter filter)
{
    return RtStageIterator(getObject(), filter);
}

}
