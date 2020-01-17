//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>
#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

RtInput::RtInput(const RtObject& obj) :
    RtAttribute(obj)
{
    if (isValid() && !(isInput() && isConnectable()))
    {
        throw ExceptionRuntimeError("Given object is not a connectable input attribute");
    }
}

RtApiType RtInput::getApiType() const
{
    return RtApiType::INPUT;
}

bool RtInput::isUniform() const
{
    return hnd()->asA<PvtAttribute>()->isUniform();
}

bool RtInput::isConnected() const
{
    return hnd()->asA<PvtAttribute>()->isConnected();
}

void RtInput::connect(RtOutput source)
{
    RtNode::connect(source, *this);
}

void RtInput::disconnect(RtOutput source)
{
    RtNode::disconnect(source, *this);
}

void RtInput::clearConnections()
{
    return hnd()->asA<PvtAttribute>()->clearConnections();
}

RtOutput RtInput::getConnection() const
{
    PvtAttribute* attr = hnd()->asA<PvtAttribute>()->getConnection();
    return attr ? RtOutput(attr->obj()) : RtObject();
}


RtOutput::RtOutput(const RtObject& obj) :
    RtAttribute(obj)
{
    if (isValid() && !(isOutput() && isConnectable()))
    {
        throw ExceptionRuntimeError("Given object is not a connectable output attribute");
    }
}

RtApiType RtOutput::getApiType() const
{
    return RtApiType::OUTPUT;
}

bool RtOutput::isConnected() const
{
    return hnd()->asA<PvtAttribute>()->isConnected();
}

void RtOutput::connect(RtInput dest)
{
    RtNode::connect(*this, dest);
}

void RtOutput::disconnect(RtInput dest)
{
    RtNode::disconnect(*this, dest);
}

void RtOutput::clearConnections()
{
    return hnd()->asA<PvtAttribute>()->clearConnections();
}

RtConnectionIterator RtOutput::getConnections() const
{
    return hnd()->asA<PvtAttribute>()->getConnections();
}


RtNode::RtNode(const RtObject& obj) :
    RtPrim(obj)
{
}

const RtToken& RtNode::typeName()
{
    return PvtNode::typeName();
}

RtApiType RtNode::getApiType() const
{
    return RtApiType::NODE;
}

RtObject RtNode::getNodeDef() const
{
    return hnd()->asA<PvtNode>()->getNodeDef()->obj();
}

RtInput RtNode::getInput(const RtToken& name) const
{
    PvtAttribute* attr = hnd()->asA<PvtNode>()->getInput(name);
    return attr ? RtInput(attr->obj()) : RtObject();
}

RtOutput RtNode::getOutput(const RtToken& name) const
{
    PvtAttribute* attr = hnd()->asA<PvtNode>()->getOutput(name);
    return attr ? RtOutput(attr->obj()) : RtObject();
}

void RtNode::connect(RtOutput source, RtInput dest)
{
    PvtAttribute* sourceAttr = PvtObject::ptr<PvtAttribute>(source.getObject());
    PvtAttribute* destAttr = PvtObject::ptr<PvtAttribute>(dest.getObject());
    PvtAttribute::connect(sourceAttr, destAttr);
}

void RtNode::disconnect(RtOutput source, RtInput dest)
{
    PvtAttribute* sourceAttr = PvtObject::ptr<PvtAttribute>(source.getObject());
    PvtAttribute* destAttr = PvtObject::ptr<PvtAttribute>(dest.getObject());
    PvtAttribute::disconnect(sourceAttr, destAttr);
}

}
