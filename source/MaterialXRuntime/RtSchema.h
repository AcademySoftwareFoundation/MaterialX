//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSCHEMA_H
#define MATERIALX_RTSCHEMA_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

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
    /// Shorthand for calling getPrim().getName().
    const RtToken& getName() const
    {
        return getPrim().getName();
    }

    /// Return the path of the prim.
    /// Shorthand for calling getPrim().getPath().
    RtPath getPath() const
    {
        return getPrim().getPath();
    }

    /// Add new metadata to the prim.
    /// Shorthand for calling getPrim().addMetadata().
    RtTypedValue* addMetadata(const RtToken& name, const RtToken& type)
    {
        return getPrim().addMetadata(name, type);
    }

    /// Remove metadata from the prim.
    /// Shorthand for calling getPrim().removeMetadata().
    void removeMetadata(const RtToken& name)
    {
        return getPrim().removeMetadata(name);
    }

    /// Return metadata from the prim.
    /// Shorthand for calling getPrim().getMetadata().
    RtTypedValue* getMetadata(const RtToken& name)
    {
        return getPrim().getMetadata(name);
    }

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
    /// Return the typename for the prim defined by this schema.
    virtual const RtToken& getTypeName() const;

protected:
    /// Constructor attaching a prim to the API.
    explicit RtTypedSchema(const RtPrim& prim) :
        RtSchemaBase(prim)
    {
    }

    /// Return true if the given prim handle is supported by this API.
    bool isSupported(const PvtDataHandle& hnd) const override;
};

/// Macro declaring required methods and mambers on typed schemas.
#define DECLARE_TYPED_SCHEMA(T)                                                             \
private:                                                                                    \
    static const RtToken _typeName;                                                         \
public:                                                                                     \
    T(const RtPrim& prim) : RtTypedSchema(prim) {}                                          \
    const RtToken& getTypeName() const override { return _typeName; }                       \
    static const RtToken& typeName() { return _typeName; }                                  \
    static RtPrim createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent);  \

/// Macro defining required methods and mambers on typed schemas.
#define DEFINE_TYPED_SCHEMA(T, typeNameStr)                                                 \
const RtToken T::_typeName(typeNameStr);                                                    \

}

#endif
