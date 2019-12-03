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
    return RtObject(PvtStage::createNew(name));
}

void RtStage::addReference(RtObject stage)
{
    data()->asA<PvtStage>()->addReference(stage.data());
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
    return data()->asA<PvtStage>()->getReference(index);
}

RtObject RtStage::findReference(const RtToken& name) const
{
    return RtObject(data()->asA<PvtStage>()->findReference(name));
}

void RtStage::addElement(RtObject elem)
{
    if (elem.hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("A stage cannot be added as direct child of another stage. Use addReference() instead to reference the stage.");
    }
    data()->asA<PvtStage>()->addChild(elem.data());
}

void RtStage::removeElement(const RtToken& name)
{
    data()->asA<PvtStage>()->removeChild(name);
}

RtObject RtStage::findElementByName(const RtToken& name) const
{
    PvtObjectHandle elem = data()->asA<PvtStage>()->findChildByName(name);
    return RtObject(elem);
}

RtObject RtStage::findElementByPath(const string& path) const
{
    PvtObjectHandle elem = data()->asA<PvtStage>()->findChildByPath(path);
    return RtObject(elem);
}

RtStageIterator RtStage::traverseStage(RtTraversalFilter filter)
{
    return RtStageIterator(getObject(), filter);
}

}
