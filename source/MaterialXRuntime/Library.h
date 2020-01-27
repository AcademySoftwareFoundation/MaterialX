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
#include <memory>

namespace MaterialX
{

/// Number to match if an index is valid.
const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

/// Reference counted shared pointer class.
/// Objects uing this class should derive from RtRefBase.
template<typename T>
using RtRefPtr = std::shared_ptr<T>; // Using STL pointers for now.

/// Base class for reference counted objects.
template<typename T>
using RtRefBase = std::enable_shared_from_this<T>; // Using STL pointers for now.

/// Weak reference pointer class.
template<typename T>
using RtWeakPtr = std::weak_ptr<T>; // Using STL pointers for now.

// Forward delcarations
class RtApi;
class RtObject;
class RtPath;
class RtStage;
class RtPrim;
class RtAttribute;
class RtInput;
class RtOutput;
class RtRelationship;

/// Shared pointer to the API instance.
using RtApiPtr = RtRefPtr<RtApi>;

/// Shared pointer to a stage.
using RtStagePtr = RtRefPtr<RtStage>;

/// Weak pointer to a stage.
using RtStageWeakPtr = RtWeakPtr<RtStage>;

/// Predicate used for filtering objects during traversal.
using RtObjectPredicate = std::function<bool(const RtObject& obj)>;

/// @class ExceptionRuntimeError
/// An exception that is thrown when a runtime operation fails.
class ExceptionRuntimeError : public Exception
{
public:
    using Exception::Exception;
};

} // namespace MaterialX

#endif
