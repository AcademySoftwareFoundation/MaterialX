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

class RtPrimSpec;

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

    /// Return the name of the attached prim.
    /// Shorthand for calling getPrim().getName().
    const RtToken& getName() const
    {
        return getPrim().getName();
    }

    /// Return the path of the attached prim.
    /// Shorthand for calling getPrim().getPath().
    RtPath getPath() const
    {
        return getPrim().getPath();
    }

    /// Create a new attribute on the attached prim.
    /// Shorthand for calling getPrim().createAttribute().
    RtTypedValue* createAttribute(const RtToken& name, const RtToken& type)
    {
        return getPrim().createAttribute(name, type);
    }

    /// Remove an attribute from the attached prim.
    /// Shorthand for calling getPrim().removeAttribute().
    void removeAttribute(const RtToken& name)
    {
        return getPrim().removeAttribute(name);
    }

    /// Return an attribute from the attached prim.
    /// Shorthand for calling getPrim().getAttribute(name).
    RtTypedValue* getAttribute(const RtToken& name)
    {
        return getPrim().getAttribute(name);
    }

    /// Return an attribute from the attached prim.
    /// Shorthand for calling getPrim().getAttribute(name).
    const RtTypedValue* getAttribute(const RtToken& name) const
    {
        return getPrim().getAttribute(name);
    }

    /// Return an attribute from the attached prim, including a type check.
    /// Shorthand for calling getPrim().getAttribute(name, type).
    RtTypedValue* getAttribute(const RtToken& name, const RtToken& type)
    {
        return getPrim().getAttribute(name, type);
    }

    /// Return an attribute from the attached prim, including a type check.
    /// Shorthand for calling getPrim().getAttribute(name, type).
    const RtTypedValue* getAttribute(const RtToken& name, const RtToken& type) const
    {
        return getPrim().getAttribute(name, type);
    }

    // Accessors.
    PvtPrim* prim() const;
    PvtRelationship* rel(const RtToken& name) const;

protected:
    /// Constructor attaching a prim to the API.
    explicit RtSchemaBase(const RtPrim& prim);

    /// Copy constructor.
    explicit RtSchemaBase(const RtSchemaBase& other);

    /// Return the handle set for this API.
    PvtObjHandle& hnd() { return _hnd; }

    /// Return the handle set for this API.
    const PvtObjHandle& hnd() const { return _hnd; }

protected:
    // Handle for the prim attached to the API.
    PvtObjHandle _hnd;
};


/// @class RtTypedSchema
/// Base class for all typed prim schemas.
class RtTypedSchema : public RtSchemaBase
{
public:
    /// Return the type info for the prim defined by this schema.
    virtual const RtTypeInfo& getTypeInfo() const = 0;

    /// Return a prim spec for the prim type defined by this schema.
    virtual const RtPrimSpec& getPrimSpec() const = 0;

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
    const RtPrimSpec& getPrimSpec() const override;                                         \
    static const RtToken& typeName() { return _typeInfo.getShortTypeName(); }               \
    static const RtTypeInfo& typeInfo() { return _typeInfo; }                               \
    static RtPrim createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent);  \

/// Macro defining required methods and mambers on typed schemas.
#define DEFINE_TYPED_SCHEMA(T, typeNameHierachy)                                            \
const RtTypeInfo T::_typeInfo(typeNameHierachy);                                            \

}

#endif
