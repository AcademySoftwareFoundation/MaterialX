//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTATTRIBUTE_H
#define MATERIALX_RTATTRIBUTE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPathItem.h>

namespace MaterialX
{

class RtValue;

/// @class RtAttrFlag
/// Flags for tagging attributes.
class RtAttrFlag
{
public:
    /// Attribute is an output.
    static const uint32_t OUTPUT      = 0x00000001;

    /// Attribute is connectable.
    static const uint32_t CONNECTABLE = 0x00000002;

    /// Attribute holds uniform values.
    static const uint32_t UNIFORM     = 0x00000004;

    /// Attribute is a nodegraph internal socket.
    static const uint32_t SOCKET      = 0x00000008;
};

/// @class RtAttribute
/// API for accessing an attribute on a prim.
class RtAttribute : public RtPathItem
{
public:
    /// Constructor attaching an object to the API.
    RtAttribute(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

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

    /// Return true if this is an input attribute.
    bool isInput() const;

    /// Return true if this is an output attribute.
    bool isOutput() const;

    /// Return true if this attribute is connectable.
    bool isConnectable() const;

    /// Return true if this attribute is connectable
    /// to the given other attribute.
    bool isConnectable(const RtAttribute& other) const;
};

}

#endif
