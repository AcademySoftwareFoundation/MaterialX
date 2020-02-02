//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTOBJECT_H
#define MATERIALX_RTOBJECT_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtPointer.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtPrim;
class PvtAttribute;
class PvtRelationship;
class RtStage;

// A handle to private object data
class PvtObject;
DECLARE_REF_PTR_TYPE(PvtObject, PvtDataHandle)

/// Shared pointer to a stage.
using RtStagePtr = RtSharedPtr<RtStage>;

/// Weak pointer to a stage.
using RtStageWeakPtr = RtWeakPtr<RtStage>;

/// Identifiers for object types.
enum class RtObjType
{
    // NOTE: If this enum is changed in any way
    // make sure to update RtObject::_canCast()
    OBJECT,
    PRIM,
    ATTRIBUTE,
    INPUT,
    OUTPUT,
    RELATIONSHIP,
    NUM_TYPES
};

/// @class RtObject
/// Base class for all runtime objects.
class RtObject
{
public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtObject();

    /// Construct from a data handle.
    RtObject(PvtDataHandle hnd);

    /// Copy constructor.
    RtObject(const RtObject& other);

    /// Destructor.
    ~RtObject();

    /// Return the type of this object instance.
    RtObjType getObjType() const;

    /// Return the type of this object class.
    static RtObjType objType()
    {
        return RtObjType::OBJECT;
    }

    /// Return true if this object can be cast to the templated object class.
    template<class T>
    bool isA() const
    {
        static_assert(std::is_base_of<RtObject, T>::value,
            "Templated type must be an RtObject or a subclass of RtObject");
        return _canCast(getObjType(), T::objType());
    }

    /// Cast to the templated object class. Returns and invalid object if
    /// the classes are not compatible.
    template<class T>
    T asA() const
    {
        static_assert(std::is_base_of<RtObject, T>::value,
            "Templated type must be an RtObject or a subclass of RtObject");
        return _canCast(getObjType(), T::objType()) ? T(_hnd) : T();
    }

    /// Return true if the object is valid.
    bool isValid() const
    {
        return _hnd != nullptr;
    }

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

    /// Return the name of the object.
    const RtToken& getName() const;

    /// Return the full path to the object.
    RtPath getPath() const;

    /// Return the parent to the object.
    RtPrim getParent() const;

    /// Return the root prim for the object.
    RtPrim getRoot() const;

    /// Return the stage that owns the object.
    RtStageWeakPtr getStage() const;

    /// Add new metadata to the object.
    RtTypedValue* addMetadata(const RtToken& name, const RtToken& type);

    /// Remove metadata from the object.
    void removeMetadata(const RtToken& name);

    /// Return metadata from the object.
    RtTypedValue* getMetadata(const RtToken& name);

protected:
    /// Return the data handle.
    const PvtDataHandle& hnd() const
    {
        return _hnd;
    }

    /// Return true if given type casting can be done.
    static bool _canCast(RtObjType fromType, RtObjType toType);

    /// Internal data handle.
    PvtDataHandle _hnd;

    friend class PvtObject;
    friend class PvtStage;
    friend class RtSchemaBase;
};

}

#endif
