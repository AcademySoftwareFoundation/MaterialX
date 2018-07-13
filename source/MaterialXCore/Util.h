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

class Element;

using ElementPtr = shared_ptr<Element>;

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

/// Pretty print the given element tree, calling asString recursively on each
/// element in depth-first order.
string prettyPrint(ElementPtr elem);

} // namespace MaterialX

#endif
