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

RtObject RtNodeGraph::createNew(RtObject parent, const RtToken& name)
{
    if (!(parent.hasApi(RtApiType::STAGE) || parent.hasApi(RtApiType::NODEGRAPH)))
    {
        throw ExceptionRuntimeError("Parent object must be a stage or a nodegraph");
    }
    return RtObject(PvtNodeGraph::createNew(parent.data()->asA<PvtElement>(), name));
}

RtApiType RtNodeGraph::getApiType() const
{
    return RtApiType::NODEGRAPH;
}

void RtNodeGraph::addNode(RtObject node)
{
    return data()->asA<PvtNodeGraph>()->addNode(node.data());
}

void RtNodeGraph::removeNode(RtObject node)
{
    if (!node.hasApi(RtApiType::NODE))
    {
        throw ExceptionRuntimeError("Given object is not a node");
    }
    PvtNode* n = node.data()->asA<PvtNode>();
    return data()->asA<PvtNodeGraph>()->removeNode(n->getName());
}

void RtNodeGraph::removePort(RtObject portdef)
{
    if (!portdef.hasApi(RtApiType::PORTDEF))
    {
        throw ExceptionRuntimeError("Given object is not a portdef");
    }
    PvtPortDef* p = portdef.data()->asA<PvtPortDef>();
    return data()->asA<PvtNodeGraph>()->removePort(p->getName());
}

size_t RtNodeGraph::numNodes() const
{
    return data()->asA<PvtNodeGraph>()->numChildren();
}

RtObject RtNodeGraph::getNode(size_t index) const
{
    PvtObjectHandle node = data()->asA<PvtNodeGraph>()->getChild(index);
    return RtObject(node);
}

RtObject RtNodeGraph::findNode(const RtToken& name) const
{
    PvtObjectHandle node = data()->asA<PvtNodeGraph>()->findChildByName(name);
    return RtObject(node);
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
