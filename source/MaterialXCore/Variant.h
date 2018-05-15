//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_VARIANT_H
#define MATERIALX_VARIANT_H

/// @file
/// Variant element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Interface.h>

namespace MaterialX
{

/// A shared pointer to a Variant
using VariantPtr = shared_ptr<class Variant>;
/// A shared pointer to a const Variant
using ConstVariantPtr = shared_ptr<const class Variant>;

/// A shared pointer to a VariantSet
using VariantSetPtr = shared_ptr<class VariantSet>;
/// A shared pointer to a const VariantSet
using ConstVariantSetPtr = shared_ptr<const class VariantSet>;

/// @class Variant
/// A variant element within a VariantSet
class Variant : public InterfaceElement
{
  public:
    Variant(ElementPtr parent, const string& name) :
        InterfaceElement(parent, CATEGORY, name)
    {
    }
    virtual ~Variant() { }

public:
    static const string CATEGORY;
};

/// @class VariantSet
/// A variant set element within a Document.
class VariantSet : public Element
{
  public:
    VariantSet(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~VariantSet() { }

    /// @}
    /// @name Varient Elements
    /// @{

    /// Add a Variant to the variant set.
    /// @param name The name of the new Variant.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new Variant.
    VariantPtr addVariant(const string& name = EMPTY_STRING)
    {
        return addChild<Variant>(name);
    }

    /// Return the Variant, if any, with the given name.
    VariantPtr getVariant(const string& name) const
    {
        return getChildOfType<Variant>(name);
    }

    /// Return a vector of all Variant elements in the look.
    vector<VariantPtr> getVariants() const
    {
        return getChildrenOfType<Variant>();
    }

    /// Remove the Variant, if any, with the given name.
    void removeVariant(const string& name)
    {
        removeChildOfType<Variant>(name);
    }

    /// @}

  public:
    static const string CATEGORY;
};

} // namespace MaterialX

#endif
