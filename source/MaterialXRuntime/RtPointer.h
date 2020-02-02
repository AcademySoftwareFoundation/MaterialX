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
/// The object pointed to owns the reference counter and must implemented 
/// the functionts to add references and release the object.
/// Use macros below for a default implementations of this.
template<typename T>
using RtRefPtr = boost::intrusive_ptr<T>;

// Macro for declaring ref pointer type for a class.
#define DECLARE_REF_PTR_TYPE(T, name)                               \
using name = RtRefPtr<T>;                                           \
void intrusive_ptr_add_ref(const T* obj);                           \
void intrusive_ptr_release(const T* obj);                           \

// Macro for adding reference counter for a class and declaring
// functions to support the intrusive ref pointer type.
#define DECLARE_REF_COUNTED_CLASS(T)                                \
public:                                                             \
    int64_t refCount() const { return _refCount; }                  \
private:                                                            \
    mutable std::atomic<int64_t> _refCount = 0;                     \
    friend void intrusive_ptr_add_ref(const T* obj);                \
    friend void intrusive_ptr_release(const T* obj);                \

// Macro for defining the reference counting functions for a class
#define DEFINE_REF_COUNTED_CLASS(T)                                 \
void intrusive_ptr_add_ref(const T* p)                              \
{                                                                   \
    p->_refCount.fetch_add(1, std::memory_order_relaxed);           \
}                                                                   \
void intrusive_ptr_release(const T* p)                              \
{                                                                   \
    if (p->_refCount.fetch_sub(1, std::memory_order_release) == 1)  \
    {                                                               \
        delete p;                                                   \
    }                                                               \
}                                                                   \

} // namespace MaterialX

#endif
