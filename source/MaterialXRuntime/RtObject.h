//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTOBJECT_H
#define MATERIALX_RTOBJECT_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtString.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtPointer.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtObject;
class PvtPrim;
class PvtPort;
class PvtRelationship;
class RtStage;
class RtAttributeIterator;

// A handle to private object data
RT_DECLARE_REF_PTR_TYPE(PvtObject, PvtObjHandle)

/// Shared pointer to a stage.
using RtStagePtr = RtSharedPtr<RtStage>;

/// Weak pointer to a stage.
using RtStageWeakPtr = RtWeakPtr<RtStage>;

/// Identifiers for object types.
enum class RtObjType
{
    OBJECT          = 1 << 0,
    PRIM            = 1 << 1,
    PORT            = 1 << 2,
    INPUT           = 1 << 3,
    OUTPUT          = 1 << 4,
    RELATIONSHIP    = 1 << 5,
    DISPOSED        = 1 << 6
};

#define RT_DECLARE_RUNTIME_OBJECT(T)                         \
private:                                                     \
    static const RtObjType _classType;                       \
    static const RtString _className;                         \
public:                                                      \
    static RtObjType classType() { return _classType; }      \
    static const RtString& className() { return _className; } \

#define RT_DEFINE_RUNTIME_OBJECT(T, type, name)              \
const RtObjType T::_classType(type);                         \
const RtString T::_className(name);                           \

/// @class RtObject
/// Base class for all runtime objects.
class RtObject
{
    RT_DECLARE_RUNTIME_OBJECT(RtObject)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtObject();

    /// Construct from a handle.
    RtObject(PvtObjHandle hnd);

    /// Copy constructor.
    RtObject(const RtObject& other);

    /// Destructor.
    ~RtObject();

    /// Return true if this object can be cast to the templated object class.
    template<class T>
    bool isA() const
    {
        static_assert(std::is_base_of<RtObject, T>::value,
            "Templated type must be an RtObject or a subclass of RtObject");
        return isCompatible(T::classType());
    }

    /// Cast to the templated object class. Returns and invalid object if
    /// the classes are not compatible.
    template<class T>
    T asA() const
    {
        static_assert(std::is_base_of<RtObject, T>::value,
            "Templated type must be an RtObject or a subclass of RtObject");
        return isCompatible(T::classType()) ? T(_hnd) : T();
    }

    /// Return true if the object is valid.
    bool isValid() const;

    /// Returns true if the object is invalid.
    bool operator!() const
    {
        return !isValid();
    }

    /// Explicit bool conversion operator.
    /// Return true if the object is valid.
    explicit operator bool() const
    {
        return isValid();
    }

    /// Return true if the objects are equal.
    bool operator==(const RtObject& other) const
    {
        return _hnd == other._hnd;
    }

    /// Return true if the objects are not equal.
    bool operator!=(const RtObject& other) const
    {
        return _hnd != other._hnd;
    }

    /// Return the name of the object.
    const RtString& getName() const;

    /// Return the full path to the object.
    RtPath getPath() const;

    /// Return the parent to the object.
    RtPrim getParent() const;

    /// Return the root prim for the object.
    RtPrim getRoot() const;

    /// Return the stage that owns the object.
    RtStageWeakPtr getStage() const;

    /// Create a new attribute on the object.
    RtTypedValue* createAttribute(const RtString& name, const RtString& type);

    /// Remove an attribute from the object.
    void removeAttribute(const RtString& name);

    /// Return an attribute by name.
    /// No type check is performed.
    RtTypedValue* getAttribute(const RtString& name);

    /// Return an attribute by name.
    /// No type check is performed.
    const RtTypedValue* getAttribute(const RtString& name) const;

    /// Return an attribute by name.
    /// With a type check that throw exception if an attribute 
    /// exists but has a different deta type.
    RtTypedValue* getAttribute(const RtString& name, const RtString& type);

    /// Return an attribute by name.
    /// With a type check that throw exception if an attribute 
    /// exists but has a different deta type.
    const RtTypedValue* getAttribute(const RtString& name, const RtString& type) const;

    /// Return an iterator over all attributes on this object.
    RtAttributeIterator getAttributes() const;

protected:
#ifdef NDEBUG
    /// Return the handle.
    const PvtObjHandle& hnd() const
    {
        return _hnd;
    }
#else
    /// Return the handle.
    const PvtObjHandle& hnd() const;
#endif

    /// Return true if this object is compatible
    /// with an object of the given type id.
    bool isCompatible(RtObjType typeId) const;

    /// Internal handle.
    PvtObjHandle _hnd;

    friend class PvtObject;
    friend class PvtStage;
    friend class RtSchemaBase;
};

}

#endif
