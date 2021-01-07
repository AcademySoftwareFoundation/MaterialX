//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_UTIL
#define MATERIALX_UTIL

/// @file
/// Utility methods

#include <MaterialXCore/Library.h>

namespace MaterialX
{

extern const string EMPTY_STRING;

/// Return the version of the MaterialX library as a string.
string getVersionString();

/// Return the major, minor, and build versions of the MaterialX library
/// as an integer tuple.
std::tuple<int, int, int> getVersionIntegers();

/// Create a valid MaterialX name from the given string.
string createValidName(string name, char replaceChar = '_');

/// Return true if the given string is a valid MaterialX name.
bool isValidName(const string& name);

/// Increment the numeric suffix of a name
string incrementName(const string& name);

/// Split a string into a vector of substrings using the given set of
/// separator characters.
StringVec splitString(const string& str, const string& sep);

/// Apply the given substring substitutions to the input string.
string replaceSubstrings(string str, const StringMap& stringMap);

/// Return a copy of the given string with letters converted to lower case.
string stringToLower(string str);

/// Return true if the given string ends with the given suffix.
bool stringEndsWith(const string& str, const string& suffix);

/// Trim leading and trailing spaces from a string.
string trimSpaces(const string& str);

/// Combine the hash of a value with an existing seed.
template<typename T> void hashCombine(size_t& seed, const T& value)
{
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
} 

/// Split a name path into string vector
StringVec splitNamePath(const string& namePath);

/// Create a name path from a string vector
string createNamePath(const StringVec& nameVec);

/// Given a name path, return the parent name path
string parentNamePath(const string& namePath);

} // namespace MaterialX

#endif
