//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSCHEMA_H
#define MATERIALX_RTSCHEMA_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTypeInfo.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

/// Traversal predicate for schemas.
template<class T>
struct RtSchemaPredicate
{
    bool operator()(const RtObject& obj)
    {
        return obj.isA<RtPrim>() && obj.asA<RtPrim>().hasApi<T>();
    }
};

/// @class RtSchemaBase
/// Base class for all prim schemas.
class RtSchemaBase
{
public:
    /// Destructor.
    virtual ~RtSchemaBase() {};

    /// Return true if the given prim is compatible with this schema.
    virtual bool isCompatible(const RtPrim& prim) const = 0;

    /// Return true if the attached prim is valid
    /// and compatible with this schema.
    bool isValid() const
    {
        return _hnd && isCompatible(_hnd);
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
    /// Shorthand for calling getPrim().getMetadata(name).
    RtTypedValue* getMetadata(const RtToken& name)
    {
        return getPrim().getMetadata(name);
    }

    /// Return metadata from the prim.
    /// Shorthand for calling getPrim().getMetadata(name).
    const RtTypedValue* getMetadata(const RtToken& name) const
    {
        return getPrim().getMetadata(name);
    }

    /// Return metadata from the prim, including a type check.
    /// Shorthand for calling getPrim().getMetadata(name, type).
    RtTypedValue* getMetadata(const RtToken& name, const RtToken& type)
    {
        return getPrim().getMetadata(name, type);
    }

    /// Return metadata from the prim, including a type check.
    /// Shorthand for calling getPrim().getMetadata(name, type).
    const RtTypedValue* getMetadata(const RtToken& name, const RtToken& type) const
    {
        return getPrim().getMetadata(name, type);
    }

    /// Returns a vector of public metadata names for the schema.
    virtual const RtTokenVec& getPublicMetadataNames() const;

    /// Returns a vector of public port metatdata names for the schema.
    virtual const RtTokenVec& getPublicPortMetadataNames(const RtToken& name) const;


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

protected:
    // Handle for the prim attached to the API.
    PvtDataHandle _hnd;
};


/// @class RtTypedSchema
/// Base class for all typed prim schemas.
class RtTypedSchema : public RtSchemaBase
{
public:
    /// Return the type info for the prim defined by this schema.
    virtual const RtTypeInfo& getTypeInfo() const = 0;

    /// Return true if the given prim is compatible with this schema.
    bool isCompatible(const RtPrim& prim) const override;

protected:
    /// Constructor attaching a prim to the API.
    explicit RtTypedSchema(const RtPrim& prim) :
        RtSchemaBase(prim)
    {
    }
};


/// Macro declaring required methods and mambers on typed schemas.
#define DECLARE_TYPED_SCHEMA(T)                                                             \
private:                                                                                    \
    static const RtTypeInfo _typeInfo;                                                      \
public:                                                                                     \
    const RtTypeInfo& getTypeInfo() const override { return _typeInfo; }                    \
    static const RtToken& typeName() { return _typeInfo.getShortTypeName(); }               \
    static const RtTypeInfo& typeInfo() { return _typeInfo; }                               \
    static RtPrim createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent);  \

/// Macro defining required methods and mambers on typed schemas.
#define DEFINE_TYPED_SCHEMA(T, typeNameHierachy)                                            \
const RtTypeInfo T::_typeInfo(typeNameHierachy);                                            \

}

#endif
