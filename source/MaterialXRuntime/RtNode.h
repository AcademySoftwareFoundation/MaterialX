//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODE_H
#define MATERIALX_RTNODE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtAttribute.h>

namespace MaterialX
{

class RtInput;
class RtOutput;

/// @class RtInput
/// API for accessing connections on an input attribute.
class RtInput : public RtAttribute
{
public:
    /// Constructor attaching an object to the API.
    /// Object must be an input attribute on a node instance.
    RtInput(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return true if this input is uniform.
    bool isUniform() const;

    /// Return true if this input is connected.
    bool isConnected() const;

    /// Connect to a source output.
    void connect(RtOutput& source);

    /// Disconnect from a source output.
    void disconnect(RtOutput& source);

    /// Break any connections.
    void clearConnections();

    /// Return the output connected to this input.
    RtObject getConnection() const;
};

/// @class RtOutput
/// API for accessing connections on an output attribute.
class RtOutput : public RtAttribute
{
public:
    /// Constructor attaching an object to the API.
    /// Object must be an input attribute on a node instance.
    RtOutput(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return true if this output is connected.
    bool isConnected() const;

    /// Connect to a destination input.
    void connect(RtInput& dest);

    /// Disconnect from a destination input.
    void disconnect(RtInput& dest);

    /// Break any connections.
    void clearConnections();

    /// Return an iterator for the connections downstream from this output.
    RtConnectionIterator getConnections() const;
};


/// @class RtNode
/// API for accessing a node instance. This API can be
/// attached to objects of type NODE and NODEGRAPH.
class RtNode : public RtPrim
{
public:
    /// Constructor attaching an object to the API.
    RtNode(const RtObject& obj);

    /// Return the type name for nodes.
    static const RtToken& typeName();

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the nodedef of this node.
    RtObject getNodeDef() const;

    /// Make a new connection between two attributes.
    static void connect(RtOutput& source, RtInput& dest);

    /// Break a connection between two attributes.
    static void disconnect(RtOutput& source, RtInput& dest);
};

}

#endif
