//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPORTDEF_H
#define MATERIALX_RTPORTDEF_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtElement.h>

namespace MaterialX
{

class RtValue;

/// @class RtPortFlag
/// Flags for tagging port definitions.
class RtPortFlag
{
public:
    /// Port is an output.
    static const uint32_t OUTPUT        = 0x00000001;

    /// Port is not connectable.
    static const uint32_t UNCONNECTABLE = 0x00000002;

    /// Port is a uniform.
    static const uint32_t UNIFORM       = 0x00000004;
};

/// @class RtPortDef
/// API for accessing a port definition. This API can only be
/// attached to objects of type PORTDEF.
class RtPortDef : public RtElement
{
public:
    /// Constructor attaching and object to the API.
    RtPortDef(const RtObject& obj);

    /// Create a new portdef and add it to a parent if specified.
    /// The parent must be a nodedef or a nodegraph.
    static RtObject createNew(RtObject parent, const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the data type for this port.
    const RtToken& getType() const;

    /// Return the default value for this port.
    const RtValue& getValue() const;

    /// Return the default value for this port.
    RtValue& getValue();

    /// Set a new default value on the port.
    void setValue(const RtValue& v);

    /// Return the default color space for this port.
    const RtToken& getColorSpace() const;

    /// Set the default color space for this port.
    void setColorSpace(const RtToken& colorspace);

    /// Return the default unit for this port.
    const RtToken& getUnit() const;

    /// Set the default unit for this port.
    void setUnit(const RtToken& unit);

    /// Return the flags set for this port.
    int32_t getFlags() const;

    /// Return true if the given flag is set for this port.
    bool hasFlag(uint32_t flag) const;

    /// Return true if this is an input port.
    bool isInput() const;

    /// Return true if this is an output port.
    bool isOutput() const;

    /// Return true if this port is unconnectable.
    bool isConnectable() const;

    /// Return true if this port is uniform.
    bool isUniform() const;
};

}

#endif
