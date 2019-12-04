//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTLIBRARY_H
#define MATERIALX_RTLIBRARY_H

/// @file
/// Library-wide includes and types.  This file should be the first include for
/// any public header in the MaterialXRuntime library.

#include <MaterialXCore/Library.h>

#include <unordered_set>
#include <limits>

namespace MaterialX
{

/// Number to match if an index is valid.
const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

/// @class ExceptionRuntimeError
/// An exception that is thrown when a runtime operation fails.
class ExceptionRuntimeError : public Exception
{
public:
    using Exception::Exception;
};

} // namespace MaterialX

#endif
