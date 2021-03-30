//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RtIdentifier_H
#define MATERIALX_RtIdentifier_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>

namespace MaterialX
{

class RtIdentifier;

/// Identifier representing an empty string.
extern const RtIdentifier EMPTY_IDENTIFIER;

/// @class RtIdentifier
/// Interned string class. Holds a unique reference to a string.
/// To be used for strings that changes rarely and where fast
/// compare operations are more important.
/// All string instances are kept in an internal registry and
/// a hash of the string content is used in comparisons making
/// such operations very efficient.
class RtIdentifier
{
public:
    /// Construct an empty idenfifier.
    RtIdentifier() : _entry(&NULL_ENTRY) {}

    /// Copy constructor.
    RtIdentifier(const RtIdentifier& other) : _entry(other._entry) {}

    /// Constructor creating an idenfifier from a raw string.
    explicit RtIdentifier(const char* s);

    /// Constructor creating an idenfifierfrom an std::string.
    explicit RtIdentifier(const string& s);

    /// Assingment from another idenfifier
    const RtIdentifier& assign(const RtIdentifier& other)
    {
        _entry = other._entry;
        return *this;
    }

    /// Assingment from a std::string.
    const RtIdentifier& assign(const string& other)
    {
        assign(RtIdentifier(other));
        return *this;
    }

    /// Assingment from a raw string.
    const RtIdentifier& assign(const char* other)
    {
        assign(RtIdentifier(other));
        return *this;
    }

    /// Assignment operator from other idenfifier.
    const RtIdentifier& operator=(const RtIdentifier& other)
    {
        assign(other);
        return *this;
    }

    /// Equality operator
    /// Fast compare of the idenfifier pointers.
    bool operator==(const RtIdentifier& other) const
    {
        return _entry == other._entry;
    }

    /// Inequality operator
    /// Fast compare of the idenfifier pointers.
    bool operator!=(const RtIdentifier& other) const
    {
        return _entry != other._entry;
    }

    /// Equality operator comparing an idenfifier with std::string.
    /// Performs lexicographic compares of the internal string.
    bool operator==(const std::string& other) const
    {
        return _entry->_str == other;
    }

    /// Equality operator comparing an idenfifier with a raw string.
    /// Performs lexicographic compares of the internal string.
    bool operator==(const char* other) const
    {
        return _entry->_str == other;
    }

    /// Equality operator comparing an idenfifier with std::string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator==(const std::string& s, const RtIdentifier& t)
    {
        return t == s;
    }

    /// Equality operator comparing an idenfifier with a raw string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator==(const char* s, const RtIdentifier& t)
    {
        return t == s;
    }

    /// Inequality operator comparing idenfifier with std::string.
    /// Performs lexicographic compares of the internal string.
    bool operator!=(const std::string& other) const
    {
        return _entry->_str != other;
    }

    /// Inequality operator comparing an idenfifier with raw string.
    /// Performs lexicographic compares of the internal string.
    bool operator!=(const char* other) const
    {
        return _entry->_str != other;
    }

    /// Inequality operator comparing an idenfifier with std::string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator!=(const std::string& s, const RtIdentifier& t)
    {
        return t != s;
    }

    /// Inequality operator comparing an idenfifier with raw string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator!=(const char* s, const RtIdentifier& t)
    {
        return t != s;
    }

    /// Less-than operator comparing an idenfifier lexicographically.
    inline bool operator<(const RtIdentifier& other) const
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

    /// Explicit conversion to bool.
    /// Returning false if the idenfifier is empty.
    explicit operator bool() const
    {
        return _entry != EMPTY_IDENTIFIER._entry;
    }

    /// Return a hash key for this idenfifier .
    size_t hash() const
    {
        return _entry->_hash;
    }

    /// Fast hash operator returning the hash already stored on the idenfifier .
    struct FastHash
    {
        size_t operator()(const RtIdentifier& t) const
        {
            return t.hash();
        }
    };

    /// Fast less operator that only compares the internal
    /// pointers and does no lexicographic compares.
    struct FastLess
    {
        bool operator()(const RtIdentifier& lhs, const RtIdentifier& rhs) const
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
    static const Entry NULL_ENTRY;

    friend struct RtIdentifierRegistry;
};

/// Class representing an unordered map with idenfifier keys and templated value type.
template<typename T>
using RtIdentifierMap = std::unordered_map<RtIdentifier, T, RtIdentifier::FastHash>;

/// Class representing an unordered set of idenfifier.
using RtIdentifierSet = std::unordered_set<RtIdentifier, RtIdentifier::FastHash>;

/// Class representing a vector of idenfifiers
using RtIdentifierVec = std::vector<RtIdentifier>;

}

#endif
