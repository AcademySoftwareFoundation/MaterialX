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

    /// Return the type of object contained.
    RtObjType getObjType() const;

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
    friend class RtInput;
    friend class RtOutput;
};


/// @class RtSchemaBase
/// Base class for all prim schemas.
class RtSchemaBase
{
public:
    /// Destructor.
    virtual ~RtSchemaBase() {};

    /// Return true if the given prim is supported by this API.
    bool isSupported(const RtPrim& prim) const;

    /// Return true if the attached prim is valid
    /// and supported by this API.
    bool isValid() const
    {
        return _hnd && isSupported(_hnd);
    }

    /// Return true if the attached prim is valid
    /// and supported by this API.
    bool operator!() const
    {
        return !isValid();
    }

    /// Explicit bool conversion operator.
    /// Return true if the attached prim is valid
    /// and supported by this API.
    explicit operator bool() const
    {
        return isValid();
    }

    /// Return true if the attached prims are equal.
    bool operator==(const RtSchemaBase& other) const
    {
        return _hnd == other._hnd;
    }

    /// Return the prim attached to this API.
    RtPrim getPrim() const;

    /// Return the name of the prim.
    const RtToken& getName() const;

    // Accessors.
    PvtPrim* prim() const;
    PvtAttribute* attr(const RtToken& name) const;
    PvtRelationship* rel(const RtToken& name) const;

protected:
    /// Constructor attaching a prim to the API.
    explicit RtSchemaBase(const RtPrim& prim);

    /// Copy constructor.
    explicit RtSchemaBase(const RtSchemaBase& other);

    /// Return the handle set for this API.
    PvtDataHandle& hnd() { return _hnd; }

    /// Return the handle set for this API.
    const PvtDataHandle& hnd() const { return _hnd; }

    /// Return true if the given prim handle is supported by this API.
    /// Derived classes should override this method.
    virtual bool isSupported(const PvtDataHandle& hnd) const;


protected:
    // Handle for the prim attached to the API.
    PvtDataHandle _hnd;
};

/// @class RtTypedSchema
/// Base class for all typed prim schemas.
class RtTypedSchema : public RtSchemaBase
{
public:
    /// Constructor attaching a prim to the API.
    explicit RtTypedSchema(const RtPrim& prim) :
        RtSchemaBase(prim)
    {
    }

    virtual const RtToken& getTypeName() const;

    static const string TYPE_NAME_SEPARATOR;

protected:
    /// Return true if the given prim handle is supported by this API.
    bool isSupported(const PvtDataHandle& hnd) const override;
};

/// Macro declaring required methods and mambers on typed schemas.
#define DECLARE_TYPED_SCHEMA(T)                                                             \
private:                                                                                    \
    static const RtToken _typeName;                                                         \
public:                                                                                     \
    explicit T(const RtPrim& prim) : RtTypedSchema(prim) {}                                 \
    const RtToken& getTypeName() const override { return _typeName; }                       \
    static const RtToken& typeName() { return _typeName; }                                  \
    static RtPrim createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent);  \

/// Macro defining required methods and mambers on typed schemas.
#define DEFINE_TYPED_SCHEMA(T, typeNameStr)                                                 \
const RtToken T::_typeName(typeNameStr);                                                    \

}

#endif
