//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTATTRIBUTE_H
#define MATERIALX_RTATTRIBUTE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

/// @class RtAttrFlag
/// Flags for tagging attributes.
class RtAttrFlag
{
public:
    /// Attribute is internal and hidden from UI or file output.
    static const uint32_t INTERNAL = 0x00000001;
};

/// @class RtAttribute
/// Class representing an attribute on an element. An attribute
/// holds a name, a type and a value and is used to store data,
/// or metadata, on an element. Any data that is not explicitly
/// expressed by elements and sub-elements are stored as attributes.
class RtAttribute
{
public:
    /// Get attribute name.
    const RtToken& getName() const
    {
        return _name;
    }

    /// Get attribute type.
    const RtToken& getType() const
    {
        return _type;
    }

    /// Get attribute value.
    const RtValue& getValue() const
    {
        return _value;
    }

    /// Get attribute value.
    RtValue& getValue()
    {
        return _value;
    }

    /// Set attribute value.
    void setValue(const RtValue& value)
    {
        _value = value;
    }

    /// Return a string representation for the value of this attribute.
    string getValueString() const;

    /// Set attribute value from a string representation.
    void setValueString(const string& v);

    /// Return the flags set for this attribute.
    int32_t getFlags() const
    {
        return _flags;
    }

    /// Return true if the given flag is set for this attribute.
    bool hasFlag(uint32_t flag) const
    {
        return (_flags & flag) != 0;
    }

private:
    /// Private constructor.
    RtAttribute(const RtToken& name, const RtToken& type, RtObject parent, uint32_t flags = 0);

    RtToken _name;
    RtToken _type;
    RtValue _value;
    uint32_t _flags;
    friend class PrvElement;
};

}

#endif
