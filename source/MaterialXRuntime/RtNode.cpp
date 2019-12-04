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

namespace
{
    static const RtPortVec EMPTY_PORT_VEC;
}

RtPort::RtPort() :
    _data(nullptr),
    _index(INVALID_INDEX)
{
}

RtPort::RtPort(const RtObject& node, const RtObject& portdef) :
    _data(nullptr),
    _index(INVALID_INDEX)
{
    if (node.hasApi(RtApiType::NODE) && portdef.hasApi(RtApiType::PORTDEF))
    {
        _data = PvtObject::data(node);
        PvtNode* n = _data->asA<PvtNode>();
        RtPortDef pd(portdef);
        _index = n->nodeDef()->findPortIndex(pd.getName());
    }
}

RtPort::RtPort(PvtDataHandle data, size_t index) :
    _data(data),
    _index(index)
{
}

bool RtPort::isValid() const
{
    if (_data)
    {
        PvtNode* node = _data->asA<PvtNode>();
        return node->nodeDef()->getPort(_index) != nullptr;
    }
    return false;
}

const RtToken& RtPort::getName() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->nodeDef()->getPort(_index)->getName();
}

const RtToken& RtPort::getType() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->nodeDef()->getPort(_index)->getType();
}

RtObject RtPort::getNode() const
{
    return PvtObject::object(_data);
}

int32_t RtPort::getFlags() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->nodeDef()->getPort(_index)->getFlags();
}

bool RtPort::isInput() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->nodeDef()->getPort(_index)->isInput();
}

bool RtPort::isOutput() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->nodeDef()->getPort(_index)->isOutput();
}

bool RtPort::isConnectable() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->nodeDef()->getPort(_index)->isConnectable();
}

bool RtPort::isSocket() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->getNodeName() == PvtNodeGraph::SOCKETS_NODE_NAME;
}

const RtValue& RtPort::getValue() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->_ports[_index].value;
}

RtValue& RtPort::getValue()
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->_ports[_index].value;
}

void RtPort::setValue(const RtValue& v)
{
    PvtNode* node = _data->asA<PvtNode>();
    node->_ports[_index].value = v;
}

string RtPort::getValueString() const
{
    PvtNode* node = _data->asA<PvtNode>();
    string dest;
    RtValue::toString(getType(), node->_ports[_index].value, dest);
    return dest;
}

void RtPort::setValueString(const string& v)
{
    PvtNode* node = _data->asA<PvtNode>();
    RtValue::fromString(getType(), v, node->_ports[_index].value);
}

RtAttribute* RtPort::addAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->addPortAttribute(_index, name, type, flags);
}

void RtPort::removeAttribute(const RtToken& name)
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->removePortAttribute(_index, name);
}

const RtAttribute* RtPort::getAttribute(const RtToken& name) const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->getPortAttribute(_index, name);
}

RtAttribute* RtPort::getAttribute(const RtToken& name)
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->getPortAttribute(_index, name);
}

const RtToken& RtPort::getColorSpace() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->getPortColorSpace(_index);
}

void RtPort::setColorSpace(const RtToken& colorspace)
{
    PvtNode* node = _data->asA<PvtNode>();
    node->setPortColorSpace(_index, colorspace);
}

const RtToken& RtPort::getUnit() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->getPortUnit(_index);
}

void RtPort::setUnit(const RtToken& unit)
{
    PvtNode* node = _data->asA<PvtNode>();
    node->setPortUnit(_index, unit);
}

bool RtPort::isConnected() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return !node->_ports[_index].connections.empty();
}

bool RtPort::canConnectTo(const RtPort& other) const
{
    const PvtNode* node = _data->asA<PvtNode>();
    const PvtPortDef* port = node->nodeDef()->getPort(_index);

    const PvtNode* otherNode = other.data()->asA<PvtNode>();
    const PvtPortDef* otherPort = otherNode->nodeDef()->getPort(other._index);

    return node != otherNode && port->canConnectTo(otherPort);
}

void RtPort::connectTo(const RtPort& dest)
{
    PvtNode::connect(*this, dest);
}

void RtPort::disconnectFrom(const RtPort& dest)
{
    PvtNode::disconnect(*this, dest);
}

RtPort RtPort::getSourcePort() const
{
    PvtNode* node = _data->asA<PvtNode>();
    const RtPortVec& connections = node->_ports[_index].connections;
    return connections.size() ? connections[0] : RtPort();
}

size_t RtPort::numDestinationPorts() const
{
    PvtNode* node = _data->asA<PvtNode>();
    return node->_ports[_index].connections.size();
}

RtPort RtPort::getDestinationPort(size_t index) const
{
    PvtNode* node = _data->asA<PvtNode>();
    RtPortVec& connections = node->_ports[_index].connections;
    return index < connections.size() ? connections[index] : RtPort();
}

RtGraphIterator RtPort::traverseUpstream(RtTraversalFilter filter) const
{
    return RtGraphIterator(*this, filter);
}


RtNode::RtNode(const RtObject& obj) :
    RtElement(obj)
{
}

RtObject RtNode::createNew(RtObject parent, RtObject nodedef, const RtToken& name)
{
    if (!(parent.hasApi(RtApiType::STAGE) || parent.hasApi(RtApiType::NODEGRAPH)))
    {
        throw ExceptionRuntimeError("Parent object must be a stage or a nodegraph");
    }

    if (!nodedef.hasApi(RtApiType::NODEDEF))
    {
        throw ExceptionRuntimeError("Given nodedef object is not a valid nodedef");
    }

    PvtElement* parentElem = PvtObject::ptr<PvtElement>(parent);
    PvtDataHandle data = PvtNode::createNew(parentElem, PvtObject::data(nodedef), name);
    return PvtObject::object(data);
}

RtApiType RtNode::getApiType() const
{
    return RtApiType::NODE;
}

RtObject RtNode::getNodeDef() const
{
    return PvtObject::object(data()->asA<PvtNode>()->getNodeDef());
}

const RtToken& RtNode::getNodeName() const
{
    return data()->asA<PvtNode>()->getNodeName();
}

size_t RtNode::numPorts() const
{
    return data()->asA<PvtNode>()->numPorts();
}

size_t RtNode::numOutputs() const
{
    return data()->asA<PvtNode>()->numOutputs();
}

size_t RtNode::numInputs() const
{
    return numPorts() - numOutputs();
}

RtPort RtNode::getPort(size_t index) const
{
    return data()->asA<PvtNode>()->getPort(index);
}

size_t RtNode::getOutputsOffset() const
{
    return data()->asA<PvtNode>()->getOutputsOffset();
}

size_t RtNode::getInputsOffset() const
{
    return data()->asA<PvtNode>()->getInputsOffset();
}

RtPort RtNode::findPort(const RtToken& name) const
{
    return data()->asA<PvtNode>()->findPort(name);
}

RtPort RtNode::getPort(RtObject portdef) const
{
    if (portdef.hasApi(RtApiType::PORTDEF))
    {
        PvtPortDef* pd = PvtObject::ptr<PvtPortDef>(portdef);
        return data()->asA<PvtNode>()->findPort(pd->getName());
    }
    return RtPort();
}

void RtNode::connect(const RtPort& source, const RtPort& dest)
{
    PvtNode::connect(source, dest);
}

void RtNode::disconnect(const RtPort& source, const RtPort& dest)
{
    PvtNode::disconnect(source, dest);
}

}
