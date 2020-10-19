//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTOKEN_H
#define MATERIALX_RTTOKEN_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>

namespace MaterialX
{

class RtToken;

/// Token representing an empty string.
extern const RtToken EMPTY_TOKEN;

/// @class RtToken
/// Interned string class. Holds a unique reference to a string.
/// To be used for strings that changes rarely and where fast
/// compare operations are more important.
/// All string instances are kept in an internal registry and
/// a hash of the string content is used in comparisons making
/// such operations very efficient.
class RtToken
{
public:
    /// Construct an empty token.
    RtToken() : _entry(&NULL_ENTRY) {}

    /// Copy constructor.
    RtToken(const RtToken& other) : _entry(other._entry) {}

    /// Constructor creating a token from a raw string.
    explicit RtToken(const char* s);

    /// Constructor creating a token from an std::string.
    explicit RtToken(const string& s);

    /// Assingment from another token.
    const RtToken& assign(const RtToken& other)
    {
        _entry = other._entry;
        return *this;
    }

    /// Assingment from a std::string.
    const RtToken& assign(const string& other)
    {
        assign(RtToken(other));
        return *this;
    }

    /// Assingment from a raw string.
    const RtToken& assign(const char* other)
    {
        assign(RtToken(other));
        return *this;
    }

    /// Assignment operator from other token.
    const RtToken& operator=(const RtToken& other)
    {
        assign(other);
        return *this;
    }

    /// Equality operator
    /// Fast compare of the token pointers.
    bool operator==(const RtToken& other) const
    {
        return _entry == other._entry;
    }

    /// Inequality operator
    /// Fast compare of the token pointers.
    bool operator!=(const RtToken& other) const
    {
        return _entry != other._entry;
    }

    /// Equality operator comparing token with std::string.
    /// Performs lexicographic compares of the internal string.
    bool operator==(const std::string& other) const
    {
        return _entry->_str == other;
    }

    /// Equality operator comparing token with raw string.
    /// Performs lexicographic compares of the internal string.
    bool operator==(const char* other) const
    {
        return _entry->_str == other;
    }

    /// Equality operator comparing token with std::string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator==(const std::string& s, const RtToken& t)
    {
        return t == s;
    }

    /// Equality operator comparing token with raw string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator==(const char* s, const RtToken& t)
    {
        return t == s;
    }

    /// Inequality operator comparing token with std::string.
    /// Performs lexicographic compares of the internal string.
    bool operator!=(const std::string& other) const
    {
        return _entry->_str != other;
    }

    /// Inequality operator comparing token with raw string.
    /// Performs lexicographic compares of the internal string.
    bool operator!=(const char* other) const
    {
        return _entry->_str != other;
    }

    /// Inequality operator comparing token with std::string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator!=(const std::string& s, const RtToken& t)
    {
        return t != s;
    }

    /// Inequality operator comparing token with raw string.
    /// Performs lexicographic compares of the internal string.
    friend bool operator!=(const char* s, const RtToken& t)
    {
        return t != s;
    }

    /// Less-than operator comparing tokens lexicographically.
    inline bool operator<(const RtToken& other) const
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

    /// Conversion to std::string.
    operator const string& () const
    {
        return _entry->_str;
    }

    /// Explicit conversion to bool.
    /// Returning false if the token is empty.
    explicit operator bool() const
    {
        return _entry != EMPTY_TOKEN._entry;
    }

    /// Return a hash key for this token.
    size_t hash() const
    {
        return _entry->_hash;
    }

    /// Fast hash operator returning the hash already stored on the token.
    struct FastHash
    {
        size_t operator()(const RtToken& t) const
        {
            return t.hash();
        }
    };

    /// Fast less operator that only compares the internal
    /// pointers and does no lexicographic compares.
    struct FastLess
    {
        bool operator()(const RtToken& lhs, const RtToken& rhs) const
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

    friend struct RtTokenRegistry;
};

/// Class representing an unordered map with token keys and templated value type.
template<typename T>
using RtTokenMap = std::unordered_map<RtToken, T, RtToken::FastHash>;

/// Class representing an unordered set of tokens.
using RtTokenSet = std::unordered_set<RtToken, RtToken::FastHash>;

/// Class representing a vector of tokens
using RtTokenVec = std::vector<RtToken>;

}

#endif
