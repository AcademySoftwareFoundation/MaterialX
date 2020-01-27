//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTOBJECT_H
#define MATERIALX_RTOBJECT_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtObject;
class PvtPrim;
class PvtAttribute;
class PvtRelationship;

// A handle to private data
using PvtDataHandle = RtRefPtr<PvtObject>;

/// Identifiers for object types.
enum class RtObjType
{
    NONE,
    PRIM,
    ATTRIBUTE,
    INPUT,
    OUTPUT,
    RELATIONSHIP,
    NUM_TYPES
};

/// Type identifiers for API's attachable to objects.
enum class RtApiType
{
    PRIM,
    ATTRIBUTE,
    RELATIONSHIP,
    INPUT,
    OUTPUT,
    NODEDEF,
    NODE,
    NODEGRAPH,
    BACKDROP,
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

    /// Copy constructor.
    RtObject(const RtObject& other);

    /// Destructor.
    ~RtObject();

    /// Return the type of this object instance.
    RtObjType getObjType() const;

    /// Return the type of this object class.
    static RtObjType objType() 
    {
        return RtObjType::NONE;
    }

    /// Return true if this object is of the templated type.
    template<class T>
    bool isA() const
    {
        return getObjType() == T::objType();
    }

    /// Cast the object to the templated type. Returns and invald object if
    /// the types are not compatible.
    template<class T>
    T asA() const
    {
        return isA<T>() ? static_cast<const T&>(*this) : T();
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
    /// Construct from a data handle.
    RtObject(PvtDataHandle hnd);

    /// Return the data handle.
    const PvtDataHandle& hnd() const
    {
        return _hnd;
    }

    /// Internal data handle.
    PvtDataHandle _hnd;

    friend class PvtObject;
    friend class PvtStage;
    friend class RtSchemaBase;
};

}

#endif
