//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PrvNode.h>
#include <MaterialXRuntime/Private/PrvNodeGraph.h>
#include <MaterialXRuntime/Private/PrvStage.h>

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
        _data = node.data();
        PrvNode* n = _data->asA<PrvNode>();
        RtPortDef pd(portdef);
        _index = n->nodeDef()->findPortIndex(pd.getName());
    }
}

RtPort::RtPort(PrvObjectHandle data, size_t index) :
    _data(data),
    _index(index)
{
}

bool RtPort::isValid() const
{
    if (_data)
    {
        PrvNode* node = _data->asA<PrvNode>();
        return node->nodeDef()->getPort(_index) != nullptr;
    }
    return false;
}

const RtToken& RtPort::getName() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->nodeDef()->getPort(_index)->getName();
}

const RtToken& RtPort::getType() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->nodeDef()->getPort(_index)->getType();
}

RtObject RtPort::getNode() const
{
    return RtObject(_data);
}

int32_t RtPort::getFlags() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->nodeDef()->getPort(_index)->getFlags();
}

bool RtPort::isInput() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->nodeDef()->getPort(_index)->isInput();
}

bool RtPort::isOutput() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->nodeDef()->getPort(_index)->isOutput();
}

bool RtPort::isConnectable() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->nodeDef()->getPort(_index)->isConnectable();
}

bool RtPort::isSocket() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->getNodeName() == PrvNodeGraph::SOCKETS_NODE_NAME;
}

const RtValue& RtPort::getValue() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->_ports[_index].value;
}

RtValue& RtPort::getValue()
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->_ports[_index].value;
}

void RtPort::setValue(const RtValue& v)
{
    PrvNode* node = _data->asA<PrvNode>();
    node->_ports[_index].value = v;
}

string RtPort::getValueString() const
{
    PrvNode* node = _data->asA<PrvNode>();
    string dest;
    RtValue::toString(getType(), node->_ports[_index].value, dest);
    return dest;
}

void RtPort::setValueString(const string& v)
{
    PrvNode* node = _data->asA<PrvNode>();
    RtValue::fromString(getType(), v, node->_ports[_index].value);
}

const RtToken& RtPort::getColorSpace() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->_ports[_index].colorspace;
}

void RtPort::setColorSpace(const RtToken& colorspace)
{
    PrvNode* node = _data->asA<PrvNode>();
    node->_ports[_index].colorspace = colorspace;
}

const RtToken& RtPort::getUnit() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->_ports[_index].unit;
}

void RtPort::setUnit(const RtToken& unit)
{
    PrvNode* node = _data->asA<PrvNode>();
    node->_ports[_index].unit = unit;
}

bool RtPort::isConnected() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return !node->_ports[_index].connections.empty();
}

bool RtPort::canConnectTo(const RtPort& other) const
{
    const PrvNode* node = _data->asA<PrvNode>();
    const PrvPortDef* port = node->nodeDef()->getPort(_index);

    const PrvNode* otherNode = other.data()->asA<PrvNode>();
    const PrvPortDef* otherPort = otherNode->nodeDef()->getPort(other._index);

    return node != otherNode && port->canConnectTo(otherPort);
}

void RtPort::connectTo(const RtPort& dest)
{
    PrvNode::connect(*this, dest);
}

void RtPort::disconnectFrom(const RtPort& dest)
{
    PrvNode::disconnect(*this, dest);
}

RtPort RtPort::getSourcePort() const
{
    PrvNode* node = _data->asA<PrvNode>();
    const RtPortVec& connections = node->_ports[_index].connections;
    return connections.size() ? connections[0] : RtPort();
}

size_t RtPort::numDestinationPorts() const
{
    PrvNode* node = _data->asA<PrvNode>();
    return node->_ports[_index].connections.size();
}

RtPort RtPort::getDestinationPort(size_t index) const
{
    PrvNode* node = _data->asA<PrvNode>();
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

    PrvElement* parentElem = parent.data()->asA<PrvElement>();
    return RtObject(PrvNode::createNew(parentElem, nodedef.data(), name));
}

RtApiType RtNode::getApiType() const
{
    return RtApiType::NODE;
}

RtObject RtNode::getNodeDef() const
{
    return RtObject(data()->asA<PrvNode>()->getNodeDef());
}

const RtToken& RtNode::getNodeName() const
{
    return data()->asA<PrvNode>()->getNodeName();
}

size_t RtNode::numPorts() const
{
    return data()->asA<PrvNode>()->numPorts();
}

size_t RtNode::numOutputs() const
{
    return data()->asA<PrvNode>()->numOutputs();
}

size_t RtNode::numInputs() const
{
    return numPorts() - numOutputs();
}

RtPort RtNode::getPort(size_t index) const
{
    return data()->asA<PrvNode>()->getPort(index);
}

size_t RtNode::getOutputsOffset() const
{
    return data()->asA<PrvNode>()->getOutputsOffset();
}

size_t RtNode::getInputsOffset() const
{
    return data()->asA<PrvNode>()->getInputsOffset();
}

RtPort RtNode::findPort(const RtToken& name) const
{
    return data()->asA<PrvNode>()->findPort(name);
}

RtPort RtNode::getPort(RtObject portdef) const
{
    if (portdef.hasApi(RtApiType::PORTDEF))
    {
        PrvPortDef* pd = portdef.data()->asA<PrvPortDef>();
        return data()->asA<PrvNode>()->findPort(pd->getName());
    }
    return RtPort();
}

void RtNode::connect(const RtPort& source, const RtPort& dest)
{
    PrvNode::connect(source, dest);
}

void RtNode::disconnect(const RtPort& source, const RtPort& dest)
{
    PrvNode::disconnect(source, dest);
}

}
