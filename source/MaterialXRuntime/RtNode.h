//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODE_H
#define MATERIALX_RTNODE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtElement.h>

namespace MaterialX
{

/// @class RtPort
/// Class representing a port on a node instance.
class RtPort
{
public:
    /// Default constructor.
    RtPort();

    /// Construct a port from a node and a portdef.
    RtPort(const RtObject& node, const RtObject& portdef);

    /// Return true if the port is valid.
    bool isValid() const;

    /// Returns true if the port is invalid.
    bool operator!() const
    {
        return !isValid();
    }

    /// Explicit bool conversion operator.
    /// Return true if the port is valid.
    explicit operator bool() const
    {
        return isValid();
    }

    /// Return the name of the port.
    const RtToken& getName() const;

    /// Return the type of the port.
    const RtToken& getType() const;

    /// Return the node the port belongs to.
    RtObject getNode() const;

    /// Return the attribute flags for this port.
    int32_t getFlags() const;

    /// Return true if this is an input port.
    bool isInput() const;

    /// Return true if this is an output port.
    bool isOutput() const;

    /// Return true if this port is connectable.
    bool isConnectable() const;

    /// Return true if this port is an internal
    /// socket on a nodegraph.
    bool isSocket() const;

    /// Return the value for this port.
    const RtValue& getValue() const;

    /// Return the value for this port.
    RtValue& getValue();

    /// Set a new value on the port.
    /// The given value must be of the same type as this ports value.
    void setValue(const RtValue& v);

    /// Return a string representation for the value of this port.
    string getValueString() const;

    /// Set the port value from a string representation.
    void setValueString(const string& v);

    /// Get the color space for this value.
    const RtToken& getColorSpace() const;

    /// Set the color space for this value.
    void setColorSpace(const RtToken& colorspace);

    /// Get the color space for this value.
    const RtToken& getUnit() const;

    /// Set the unit for this value.
    void setUnit(const RtToken& unit);

    /// Return true if this port is connected.
    bool isConnected() const;

    /// Return true if this port can be connected to the other port.
    bool canConnectTo(const RtPort& other) const;

    /// Connect this port to a destination input port.
    void connectTo(const RtPort& dest);

    /// Disconnect this port from a destination input port.
    void disconnectFrom(const RtPort& dest);

    /// Return the source port connected upstream.
    RtPort getSourcePort() const;

    /// Return the number of destination ports connected downstream.
    size_t numDestinationPorts() const;

    /// Return a destination port connected downstream.
    RtPort getDestinationPort(size_t index) const;

    /// Traverse the node network upstream starting from this port.
    RtGraphIterator traverseUpstream(RtTraversalFilter filter = nullptr) const;

    /// Equality operator
    bool operator==(const RtPort& other) const
    {
        return _data == other._data && _index == other._index;
    }

    /// Inequality operator
    bool operator!=(const RtPort& other) const
    {
        return _data != other._data || _index != other._index;
    }

    /// Less-than operator
    bool operator<(const RtPort& other) const
    {
        return _data != other._data ? 
            _data < other._data : _index < other._index;
    }

    /// Return the data handle.
    PvtObjectHandle data() const
    {
        return _data;
    }

private:
    RtPort(PvtObjectHandle data, size_t index);

    PvtObjectHandle _data;
    size_t _index;
    friend class PvtNode;
    friend class PvtNodeGraph;
};

/// @class RtNode
/// API for accessing a node instance. This API can be
/// attached to objects of type NODE and NODEGRAPH.
class RtNode : public RtElement
{
public:
    /// Constructor attaching and object to the API.
    RtNode(const RtObject& obj);

    /// Create a new node instance of the given nodedef type
    /// and add it to the parent object if specified.
    /// The parent must be a stage or a nodegraph object.
    static RtObject createNew(RtObject parent, RtObject nodedef, const RtToken& name = EMPTY_TOKEN);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the nodedef of this node.
    RtObject getNodeDef() const;

    /// Return the node name for this node.
    const RtToken& getNodeName() const;

    /// Return the port count.
    size_t numPorts() const;

    /// Return the output port count.
    size_t numOutputs() const;

    /// Return the input port count.
    size_t numInputs() const;

    /// Return a port corresponding to the given portdef object,
    /// or a null object if no such port exists.
    RtPort getPort(RtObject portdef) const;

    /// Return a port by index, or a null object 
    /// if no such port exists.
    RtPort getPort(size_t index) const;

    /// Get the index offset for outputs.
    /// This index points to the first output.
    size_t getOutputsOffset() const;

    /// Get the index offset for inputs.
    /// This index points to the first input.
    size_t getInputsOffset() const;

    /// Get the i:th output port, or a null object
    /// if no such port exists.
    RtPort getOutput(size_t index) const
    {
        return getPort(getOutputsOffset() + index);
    }

    /// Get the i:th input port, or a null object
    /// if no such port exists.
    RtPort getInput(size_t index) const
    {
        return getPort(getInputsOffset() + index);
    }

    /// Find a port by name, or a null object 
    /// if no such port is found.
    RtPort findPort(const RtToken& name) const;

    /// Make a new connection between two ports.
    static void connect(const RtPort& source, const RtPort& dest);

    /// Break a connection between two ports.
    static void disconnect(const RtPort& source, const RtPort& dest);
};

}

#endif
