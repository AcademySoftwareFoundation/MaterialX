//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

RtNodeDef::RtNodeDef(const RtObject& obj) :
    RtElement(obj)
{
}

RtObject RtNodeDef::createNew(RtObject stage, const RtToken& name, const RtToken& category)
{
    if (!stage.hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("Given object is not a valid stage");
    }

    PvtDataHandle nodedef = PvtNodeDef::createNew(PvtObject::ptr<PvtElement>(stage), name, category);

    return PvtObject::object(nodedef);
}

RtApiType RtNodeDef::getApiType() const
{
    return RtApiType::NODEDEF;
}

const RtToken& RtNodeDef::getNodeName() const
{
    return data()->asA<PvtNodeDef>()->getNodeName();
}

void RtNodeDef::removePort(RtObject portdef)
{
    if (!portdef.hasApi(RtApiType::PORTDEF))
    {
        throw ExceptionRuntimeError("Given object is not a portdef");
    }
    PvtPortDef* p = PvtObject::ptr<PvtPortDef>(portdef);
    return data()->asA<PvtNodeDef>()->removePort(p->getName());
}

size_t RtNodeDef::numPorts() const
{
    return data()->asA<PvtNodeDef>()->numChildren();
}

size_t RtNodeDef::numOutputs() const
{
    return data()->asA<PvtNodeDef>()->numOutputs();
}

size_t RtNodeDef::getOutputsOffset() const
{
    return data()->asA<PvtNodeDef>()->getOutputsOffset();
}

size_t RtNodeDef::getInputsOffset() const
{
    return data()->asA<PvtNodeDef>()->getInputsOffset();
}

RtObject RtNodeDef::getPort(size_t index) const
{
    PvtDataHandle portdef = data()->asA<PvtNodeDef>()->getChild(index);
    return PvtObject::object(portdef);
}

RtObject RtNodeDef::findPort(const RtToken& name) const
{
    PvtDataHandle portdef = data()->asA<PvtNodeDef>()->findChildByName(name);
    return PvtObject::object(portdef);
}

}
