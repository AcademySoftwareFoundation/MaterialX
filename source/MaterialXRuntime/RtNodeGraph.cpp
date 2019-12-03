//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PrvNodeGraph.h>
#include <MaterialXRuntime/Private/PrvStage.h>

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
    return RtObject(PrvNodeGraph::createNew(parent.data()->asA<PrvElement>(), name));
}

RtApiType RtNodeGraph::getApiType() const
{
    return RtApiType::NODEGRAPH;
}

void RtNodeGraph::addNode(RtObject node)
{
    return data()->asA<PrvNodeGraph>()->addNode(node.data());
}

void RtNodeGraph::removeNode(RtObject node)
{
    if (!node.hasApi(RtApiType::NODE))
    {
        throw ExceptionRuntimeError("Given object is not a node");
    }
    PrvNode* n = node.data()->asA<PrvNode>();
    return data()->asA<PrvNodeGraph>()->removeNode(n->getName());
}

void RtNodeGraph::removePort(RtObject portdef)
{
    if (!portdef.hasApi(RtApiType::PORTDEF))
    {
        throw ExceptionRuntimeError("Given object is not a portdef");
    }
    PrvPortDef* p = portdef.data()->asA<PrvPortDef>();
    return data()->asA<PrvNodeGraph>()->removePort(p->getName());
}

size_t RtNodeGraph::numNodes() const
{
    return data()->asA<PrvNodeGraph>()->numChildren();
}

RtObject RtNodeGraph::getNode(size_t index) const
{
    PrvObjectHandle node = data()->asA<PrvNodeGraph>()->getChild(index);
    return RtObject(node);
}

RtObject RtNodeGraph::findNode(const RtToken& name) const
{
    PrvObjectHandle node = data()->asA<PrvNodeGraph>()->findChildByName(name);
    return RtObject(node);
}

RtPort RtNodeGraph::getOutputSocket(size_t index) const
{
    return data()->asA<PrvNodeGraph>()->getOutputSocket(index);
}

RtPort RtNodeGraph::getInputSocket(size_t index) const
{
    return data()->asA<PrvNodeGraph>()->getInputSocket(index);
}

RtPort RtNodeGraph::findOutputSocket(const RtToken& name) const
{
    return data()->asA<PrvNodeGraph>()->findOutputSocket(name);
}

RtPort RtNodeGraph::findInputSocket(const RtToken& name) const
{
    return data()->asA<PrvNodeGraph>()->findInputSocket(name);
}

string RtNodeGraph::asStringDot() const
{
    return data()->asA<PrvNodeGraph>()->asStringDot();
}

}
