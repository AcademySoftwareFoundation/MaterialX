//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTLIBRARY_H
#define MATERIALX_RTLIBRARY_H

/// @file
/// Library-wide includes and types.  This file should be the first include for
/// any public header in the MaterialXRuntime library.

#include <MaterialXCore/Exception.h>
#include <MaterialXCore/Library.h>

#include <unordered_set>
#include <limits>
#include <cstring>

namespace MaterialX
{

/// Number to match if an index is valid.
const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

// Forward delcarations
class RtApi;
class RtObject;
class RtPath;
class RtStage;
class RtPrim;
class RtPort;
class RtInput;
class RtOutput;
class RtRelationship;

/// Predicate for filtering objects during traversal.
using RtObjectPredicate = std::function<bool(const RtObject& obj)>;

/// Connection change type
enum class ConnectionChange
{
    MAKE_CONNECTION,
    BREAK_CONNECTION
};

/// @class ExceptionRuntimeError
/// An exception that is thrown when a runtime operation fails.
class ExceptionRuntimeError : public Exception
{
public:
    using Exception::Exception;
};

/// @class ExceptionRuntimeTypeError
/// An exception that is thrown when a type incompatibility error occur.
class ExceptionRuntimeTypeError : public ExceptionRuntimeError
{
public:
    using ExceptionRuntimeError::ExceptionRuntimeError;
};

/// A custom reinterpret cast function. To be used when casting between
/// different interpretations of the same bits. This is a safer way to
/// re-interpret the bits. The standard method of casting the address may
/// cause pointer aliasing, and the results are undefined by the language.
/// Some compilers warn about this, so by using this custom cast function
/// we avoid the warnings and the resulting code is safer.
template <typename T_TO, typename T_FROM>
inline T_TO _reinterpret_cast(const T_FROM v)
{
    T_TO tmp;
    std::memcpy(&tmp, &v, sizeof(T_TO));
    return tmp;
}

} // namespace MaterialX

#endif
