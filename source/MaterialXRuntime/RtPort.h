//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPORT_H
#define MATERIALX_RTPORT_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

class RtValue;
class RtOutput;
class RtInputIterator;

/// @class RtPortFlag
/// Flags for tagging ports.
class RtPortFlag
{
public:
    /// Port holds uniform values.
    static const uint32_t UNIFORM     = 0x00000001;

    /// Port is a nodegraph internal socket.
    static const uint32_t SOCKET      = 0x00000002;
};

/// @class RtPort
/// Base class for prim ports.
class RtPort : public RtObject
{
    RT_DECLARE_RUNTIME_OBJECT(RtInput)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtPort() {}

    /// Construct from a handle.
    RtPort(PvtObjHandle hnd);

    /// Return the data type for this port.
    const RtToken& getType() const;

    /// Return the value for this port.
    const RtValue& getValue() const;

    /// Return the value for this port.
    RtValue& getValue();

    /// Set a new value on the port.
    void setValue(const RtValue& v);

    /// Return a string representation for the value of this port.
    string getValueString() const;

    /// Set the port value from a string representation.
    void setValueString(const string& v);

    /// Return the color space for this port.
    const RtToken& getColorSpace() const;

    /// Set the color space for this port.
    void setColorSpace(const RtToken& colorspace);

    /// Return the unit for this port.
    const RtToken& getUnit() const;

    /// Set the unit for this port.
    void setUnit(const RtToken& unit);

    /// Return the unit type for this port.
    const RtToken& getUnitType() const;

    /// Set the unit type for this port.
    void setUnitType(const RtToken& unit);
};


/// @class RtInput
/// Object holding an input port on a prim.
class RtInput : public RtPort
{
    RT_DECLARE_RUNTIME_OBJECT(RtInput)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtInput() {}

    /// Construct from a handle.
    RtInput(PvtObjHandle hnd);

    /// Return true if this input is uniform.
    bool isUniform() const;

    /// Sets the input to be a uniform or not a uniform
    void setUniform(bool uniform);

    /// Return true if this input is connected.
    bool isConnected() const;

    /// Return true if this input is an internal nodegraph socket.
    bool isSocket() const;

    /// Return true if this input is connectable
    /// to the given output.
    bool isConnectable(const RtOutput& source) const;

    /// Connect to a source output.
    void connect(const RtOutput& source);

    /// Disconnect from a source output.
    void disconnect(const RtOutput& source);

    /// Break any connections.
    void clearConnection();

    /// Return the output connected to this input.
    RtOutput getConnection() const;

    friend class RtOutput;
};

/// @class RtOutput
/// Object holding an output port on a prim.
class RtOutput : public RtPort
{
    RT_DECLARE_RUNTIME_OBJECT(RtOutput)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtOutput() {}

    /// Construct from a handle.
    RtOutput(PvtObjHandle hnd);

    /// Return true if this output is connected.
    bool isConnected() const;

    /// Return true if this output is an internal nodegraph socket.
    bool isSocket() const;

    /// Return true if this input is connectable
    /// to the given output.
    bool isConnectable(const RtInput& input) const;

    /// Connect to a destination input.
    void connect(const RtInput& input);

    /// Disconnect from a destination input.
    void disconnect(const RtInput& input);

    /// Break any connections.
    void clearConnections();

    /// Return an iterator for the connections downstream from this output.
    RtInputIterator getConnections() const;

    friend class RtInput;
};

}

#endif
