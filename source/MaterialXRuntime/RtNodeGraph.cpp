//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PvtNodeGraph.h>
#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

RtNodeGraph::RtNodeGraph(const RtObject& obj) :
    RtNode(obj)
{
}

const RtToken& RtNodeGraph::typeName()
{
    return PvtNodeGraph::typeName();
}

RtApiType RtNodeGraph::getApiType() const
{
    return RtApiType::NODEGRAPH;
}

RtObject RtNodeGraph::createAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return hnd()->asA<PvtNodeGraph>()->createAttribute(name, type, flags)->obj();
}

void RtNodeGraph::removeAttribute(const RtToken& name)
{
    return hnd()->asA<PvtNodeGraph>()->removeAttribute(name);
}

RtOutput RtNodeGraph::getInputSocket(const RtToken& name) const
{
    PvtAttribute* attr = hnd()->asA<PvtNodeGraph>()->getInputSocket(name);
    return attr ? RtOutput(attr->obj()) : RtObject();
}

RtInput RtNodeGraph::getOutputSocket(const RtToken& name) const
{
    PvtAttribute* attr = hnd()->asA<PvtNodeGraph>()->getOutputSocket(name);
    return attr ? RtInput(attr->obj()) : RtObject();
}

string RtNodeGraph::asStringDot() const
{
    return hnd()->asA<PvtNodeGraph>()->asStringDot();
}

}
