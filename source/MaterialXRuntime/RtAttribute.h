//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTATTRIBUTE_H
#define MATERIALX_RTATTRIBUTE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

class RtValue;
class RtOutput;
class RtConnectionIterator;

/// @class RtAttrFlag
/// Flags for tagging attributes.
class RtAttrFlag
{
public:
    /// Attribute holds uniform values.
    static const uint32_t UNIFORM     = 0x00000001;

    /// Attribute is a nodegraph internal socket.
    static const uint32_t SOCKET      = 0x00000002;
};

/// @class RtAttribute
/// Object holding an attribute on a prim.
class RtAttribute : public RtObject
{
    RT_DECLARE_RUNTIME_OBJECT(RtAttribute)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtAttribute() {}

    /// Construct from a data handle.
    RtAttribute(PvtDataHandle hnd);

    /// Return the data type for this attribute.
    const RtToken& getType() const;

    /// Return the default value for this attribute.
    const RtValue& getValue() const;

    /// Return the default value for this attribute.
    RtValue& getValue();

    /// Set a new default value on the attribute.
    void setValue(const RtValue& v);

    /// Return a string representation for the value of this attribute.
    string getValueString() const;

    /// Set the attribute value from a string representation.
    void setValueString(const string& v);

    /// Return the default color space for this attribute.
    const RtToken& getColorSpace() const;

    /// Set the default color space for this attribute.
    void setColorSpace(const RtToken& colorspace);

    /// Return the default unit for this attribute.
    const RtToken& getUnit() const;

    /// Set the default unit for this attribute.
    void setUnit(const RtToken& unit);

    /// Return the default unit type for this attribute.
    const RtToken& getUnitType() const;

    /// Set the default unit type for this attribute.
    void setUnitType(const RtToken& unit);
};


/// @class RtInput
/// Object holding an input attribute on a prim.
class RtInput : public RtAttribute
{
    RT_DECLARE_RUNTIME_OBJECT(RtInput)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtInput() {}

    /// Construct from a data handle.
    RtInput(PvtDataHandle hnd);

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
/// Object holding an output attribute on a prim.
class RtOutput : public RtAttribute
{
    RT_DECLARE_RUNTIME_OBJECT(RtOutput)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtOutput() {}

    /// Construct from a data handle.
    RtOutput(PvtDataHandle hnd);

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
    RtConnectionIterator getConnections() const;

    friend class RtInput;
};

}

#endif
