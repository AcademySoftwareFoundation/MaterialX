//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPOINTER_H
#define MATERIALX_RTPOINTER_H

/// @file
/// Smart pointer types used in the runtime library.

#include <MaterialXRuntime/External/boost/smart_ptr/intrusive_ptr.hpp>

#include <atomic>
#include <memory>

namespace MaterialX
{

/// Base class for classes using the shared/weak pointer.
template<typename T>
using RtSharedBase = std::enable_shared_from_this<T>;

/// Class for non-intrusive shared pointer.
template<typename T>
using RtSharedPtr = std::shared_ptr<T>;

/// Class for non-intrusive weak pointer.
template<typename T>
using RtWeakPtr = std::weak_ptr<T>;


/// Class for pointer to reference counted objects.
template<typename T>
using RtRefPtr = boost::intrusive_ptr<T>;

/// Base class for reference counted objects.
template<typename T>
class RtRefCounted
{
public:
    /// Constructor.
    RtRefCounted() :
        _refCount(0)
    {}

    /// Copy constructor.
    RtRefCounted(const RtRefCounted&) :
        _refCount(0)
    {}

    /// Assignment operator.
    RtRefCounted& operator=(const RtRefCounted&)
    {
        return *this;
    }

    /// Return the reference count for the object.
    int64_t refCount() const
    {
        return _refCount;
    }

protected:
    ~RtRefCounted() = default;

    mutable std::atomic<int64_t> _refCount;
};

// Macro for declaring a ref pointer type and the accompanying
// reference counting functions.
#define RT_DECLARE_REF_PTR_TYPE(T, name)                            \
using name = RtRefPtr<T>;                                           \
void intrusive_ptr_add_ref(T* obj);                                 \
void intrusive_ptr_release(T* obj);                                 \

// Macro for defining the reference counting functions for a type.
#define RT_DEFINE_REF_PTR_FUNCTIONS(T)                              \
void intrusive_ptr_add_ref(T* p)                                    \
{                                                                   \
    p->_refCount.fetch_add(1, std::memory_order_relaxed);           \
}                                                                   \
void intrusive_ptr_release(T* p)                                    \
{                                                                   \
    if (p->_refCount.fetch_sub(1, std::memory_order_release) == 1)  \
    {                                                               \
        delete p;                                                   \
    }                                                               \
}                                                                   \

// Macro for friending the reference counting functions for a class.
#define RT_FRIEND_REF_PTR_FUNCTIONS(T)                              \
friend void intrusive_ptr_add_ref(T* obj);                          \
friend void intrusive_ptr_release(T* obj);                          \

} // namespace MaterialX

#endif
