//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSTRING_H
#define MATERIALX_RTSTRING_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>

namespace MaterialX
{

/// @class RtString
/// Interned string class. Holds a unique reference to a string.
/// All string instances are kept in an internal registry and a hash of the
/// string content is used in comparisons making such operations very efficient.
///
/// NOTE: Creation of an RtString is more expensive than an ordinary mx::string,
/// so only use this class for strings that are constant or changes rarely,
/// and where fast compare operations are more important and outweights the
/// expensive creation. Otherwise use mx::string instead.
///
class RtString
{
public:
    /// Construct an empty RtString.
    RtString() : _entry(EMPTY._entry) {}

    /// Copy constructor.
    RtString(const RtString& other) : _entry(other._entry) {}

    /// Constructor creating an RtString from a raw string.
    explicit RtString(const char* s);

    /// Constructor creating an RtString from an std::string.
    explicit RtString(const string& s);

    /// Assingment from another RtString
    const RtString& assign(const RtString& other)
    {
        _entry = other._entry;
        return *this;
    }

    /// Assingment from a std::string.
    const RtString& assign(const string& other)
    {
        assign(RtString(other));
        return *this;
    }

    /// Assingment from a raw string.
    const RtString& assign(const char* other)
    {
        assign(RtString(other));
        return *this;
    }

    /// Assignment operator from other RtString.
    const RtString& operator=(const RtString& other)
    {
        assign(other);
        return *this;
    }

    /// Equality operator
    /// Fast compare of the RtString pointers.
    bool operator==(const RtString& other) const
    {
        return _entry == other._entry;
    }

    /// Inequality operator
    /// Fast compare of the RtString pointers.
    bool operator!=(const RtString& other) const
    {
        return _entry != other._entry;
    }

    /// Equality operator comparing an RtString with std::string.
    /// Performs lexicographic compares of the internal string.
    bool operator==(const std::string& other) const
    {
        return _entry->_str == other;
    }

    /// Equality operator comparing an RtString with a raw string.
    /// Performs lexicographic compares of the internal string.
    bool operator==(const char* other) const
    {
        return _entry->_str == other;
    }

    /// Equality operator comparing an RtString with std::string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator==(const std::string& s, const RtString& t)
    {
        return t == s;
    }

    /// Equality operator comparing an RtString with a raw string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator==(const char* s, const RtString& t)
    {
        return t == s;
    }

    /// Inequality operator comparing RtString with std::string.
    /// Performs lexicographic compares of the internal string.
    bool operator!=(const std::string& other) const
    {
        return _entry->_str != other;
    }

    /// Inequality operator comparing an RtString with raw string.
    /// Performs lexicographic compares of the internal string.
    bool operator!=(const char* other) const
    {
        return _entry->_str != other;
    }

    /// Inequality operator comparing an RtString with std::string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator!=(const std::string& s, const RtString& t)
    {
        return t != s;
    }

    /// Inequality operator comparing an RtString with raw string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator!=(const char* s, const RtString& t)
    {
        return t != s;
    }

    /// Less-than operator comparing an RtString lexicographically.
    inline bool operator<(const RtString& other) const
    {
        return _entry->_str < other._entry->_str;
    }

    /// Return the internal string as a std::string.
    const string& str() const
    {
        return _entry->_str;
    }

    /// Return the internal string as a raw string.
    const char* c_str() const
    {
        return _entry->_str.c_str();
    }

    /// Returning true if the RtString is empty.
    bool empty() const
    {
        return _entry->_str.empty();
    }

    /// Explicit conversion to bool.
    /// Returning false if the RtString is empty.
    explicit operator bool() const
    {
        return !empty();
    }

    /// Return a hash key for this RtString.
    size_t hash() const
    {
        return _entry->_hash;
    }

    /// Fast hash operator returning the hash already stored on the RtString.
    struct FastHash
    {
        size_t operator()(const RtString& t) const
        {
            return t.hash();
        }
    };

    /// Fast less operator that only compares the internal
    /// pointers and does no lexicographic compares.
    struct FastLess
    {
        bool operator()(const RtString& lhs, const RtString& rhs) const
        {
            return lhs._entry < rhs._entry;
        }
    };

private:
    struct Entry
    {
        explicit Entry(const char* s, size_t hash) : _str(s), _hash(hash) {}
        explicit Entry(const string& s, size_t hash) : _str(s), _hash(hash) {}
        string _str;
        size_t _hash;
    };
    const Entry* _entry;

    friend struct RtStringRegistry;

public:
    /// Special case, defining the empty string ""
    /// and not the string "empty".
    static const RtString EMPTY;

    /// Declaration of commonly used string constants.
    static const RtString DOC;
    static const RtString NAME;
    static const RtString TYPE;
    static const RtString VALUE;
    static const RtString UNIFORM;
    static const RtString DEFAULTGEOMPROP;
    static const RtString INTERNALGEOMPROPS;
    static const RtString ENUM;
    static const RtString ENUMVALUES;
    static const RtString COLORSPACE;
    static const RtString FILEPREFIX;
    static const RtString UINAME;
    static const RtString UICOLOR;
    static const RtString UIFOLDER;
    static const RtString UIMIN;
    static const RtString UIMAX;
    static const RtString UISOFTMIN;
    static const RtString UISOFTMAX;
    static const RtString UISTEP;
    static const RtString UIADVANCED;
    static const RtString UIVISIBLE;
    static const RtString XPOS;
    static const RtString YPOS;
    static const RtString UNIT;
    static const RtString UNITTYPE;
    static const RtString MEMBER;
    static const RtString CHANNELS;
    static const RtString MATERIAL;
    static const RtString COLLECTION;
    static const RtString GEOM;
    static const RtString EXCLUSIVE;
    static const RtString MATERIALASSIGN;
    static const RtString ENABLEDLOOKS;
    static const RtString FORMAT;
    static const RtString FILE;
    static const RtString FRAGMENT;
    static const RtString FUNCTION;
    static const RtString LOOK;
    static const RtString LOOKS;
    static const RtString INCLUDEGEOM;
    static const RtString EXCLUDEGEOM;
    static const RtString INCLUDECOLLECTION;
    static const RtString CONTAINS;
    static const RtString MINIMIZED;
    static const RtString WIDTH;
    static const RtString HEIGHT;
    static const RtString BITDEPTH;
    static const RtString IMPLNAME;
    static const RtString INHERIT;
    static const RtString KIND;
    static const RtString LANGUAGE;
    static const RtString NAMESPACE;
    static const RtString NODE;
    static const RtString NODEDEF;
    static const RtString NODEGROUP;
    static const RtString NODEIMPL;
    static const RtString NOTE;
    static const RtString SHADER;
    static const RtString SOURCECODE;
    static const RtString TARGET;
    static const RtString VERSION;
    static const RtString ISDEFAULTVERSION;
    static const RtString DEFAULT;
    static const RtString DEFAULTINPUT;
    static const RtString UNKNOWN;
};

/// Class representing an unordered map with RtString keys and templated value type.
template<typename T>
using RtStringMap = std::unordered_map<RtString, T, RtString::FastHash>;

/// Class representing an unordered set of RtStrings.
using RtStringSet = std::unordered_set<RtString, RtString::FastHash>;

/// Class representing a vector of RtStrings.
using RtStringVec = vector<RtString>;

}

#endif
