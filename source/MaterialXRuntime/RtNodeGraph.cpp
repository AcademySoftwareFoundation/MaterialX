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

RtObject RtNodeGraph::createNew(const RtObject& parent, const RtToken& name)
{
    if (!(parent.hasApi(RtApiType::STAGE) || parent.hasApi(RtApiType::NODEGRAPH)))
    {
        throw ExceptionRuntimeError("Parent object must be a stage or a nodegraph");
    }
    PvtDataHandle data = PvtNodeGraph::createNew(PvtObject::ptr<PvtElement>(parent), name);
    return PvtObject::object(data);
}

RtApiType RtNodeGraph::getApiType() const
{
    return RtApiType::NODEGRAPH;
}

void RtNodeGraph::addNode(const RtObject& node)
{
    return data()->asA<PvtNodeGraph>()->addNode(PvtObject::data(node));
}

void RtNodeGraph::removeNode(const RtObject& node)
{
    if (!node.hasApi(RtApiType::NODE))
    {
        throw ExceptionRuntimeError("Given object is not a node");
    }
    PvtNode* n = PvtObject::ptr<PvtNode>(node);
    return data()->asA<PvtNodeGraph>()->removeNode(n->getName());
}

void RtNodeGraph::removePort(const RtObject& portdef)
{
    if (!portdef.hasApi(RtApiType::PORTDEF))
    {
        throw ExceptionRuntimeError("Given object is not a portdef");
    }
    PvtPortDef* p = PvtObject::ptr<PvtPortDef>(portdef);
    return data()->asA<PvtNodeGraph>()->removePort(p->getName());
}

size_t RtNodeGraph::numNodes() const
{
    return data()->asA<PvtNodeGraph>()->numChildren();
}

RtObject RtNodeGraph::getNode(size_t index) const
{
    PvtDataHandle node = data()->asA<PvtNodeGraph>()->getChild(index);
    return PvtObject::object(node);
}

RtObject RtNodeGraph::findNode(const RtToken& name) const
{
    PvtDataHandle node = data()->asA<PvtNodeGraph>()->findChildByName(name);
    return PvtObject::object(node);
}

RtPort RtNodeGraph::getOutputSocket(size_t index) const
{
    return data()->asA<PvtNodeGraph>()->getOutputSocket(index);
}

RtPort RtNodeGraph::getInputSocket(size_t index) const
{
    return data()->asA<PvtNodeGraph>()->getInputSocket(index);
}

RtPort RtNodeGraph::findOutputSocket(const RtToken& name) const
{
    return data()->asA<PvtNodeGraph>()->findOutputSocket(name);
}

RtPort RtNodeGraph::findInputSocket(const RtToken& name) const
{
    return data()->asA<PvtNodeGraph>()->findInputSocket(name);
}

string RtNodeGraph::asStringDot() const
{
    return data()->asA<PvtNodeGraph>()->asStringDot();
}

}
