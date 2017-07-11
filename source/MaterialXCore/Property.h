//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PROPERTY_H
#define MATERIALX_PROPERTY_H

/// @file
/// Property element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Geom.h>

namespace MaterialX
{

/// A shared pointer to a Property
using PropertyPtr = shared_ptr<class Property>;
/// A shared pointer to a PropertyAssign
using PropertyAssignPtr = shared_ptr<class PropertyAssign>;
/// A shared pointer to a PropertySet
using PropertySetPtr = shared_ptr<class PropertySet>;
/// A shared pointer to a PropertySetAssign
using PropertySetAssignPtr = shared_ptr<class PropertySetAssign>;

/// @class Property
/// A property element within a PropertySet.
class Property : public ValueElement
{
  public:
    Property(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~Property() { }

  public:
    static const string CATEGORY;
};

/// @class PropertyAssign
/// A property assignment element within a Look.
class PropertyAssign : public ValueElement
{
  public:
    PropertyAssign(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~PropertyAssign() { }

    /// @name Geometry
    /// @{

    /// Set the geom string of the element.
    void setGeom(const string& name)
    {
        setAttribute(GEOM_ATTRIBUTE, name);
    }

    /// Return the geom string of the element.
    const string& getGeom() const
    {
        return getAttribute(GEOM_ATTRIBUTE);
    }

    /// @}
    /// @name Collection
    /// @{

    /// Set the collection string of the element.
    void setCollection(const string& name)
    {
        setAttribute(COLLECTION_ATTRIBUTE, name);
    }

    /// Return the collection string of the element.
    const string& getCollection() const
    {
        return getAttribute(COLLECTION_ATTRIBUTE);
    }

    /// @}

  public:
    static const string CATEGORY;
    static const string GEOM_ATTRIBUTE;
    static const string COLLECTION_ATTRIBUTE;
};

/// @class PropertySet
/// A property set element within a Document.
class PropertySet : public Element
{
  public:
    PropertySet(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~PropertySet() { }

    /// Add a Property to the set.
    /// @param name The name of the new Property.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new Property.
    PropertyPtr addProperty(const string& name)
    {
        PropertyPtr assignPtr = getChildOfType<Property>(name);
        if (assignPtr)
            return assignPtr;
        return addChild<Property>(name);
    }

    /// Return the Property, if any, with the given name.
    PropertyPtr getProperty(const string& name) const
    {
        return getChildOfType<Property>(name);
    }

    /// Return a vector of all Property elements in the set.
    vector<PropertyPtr> getProperties() const
    {
        return getChildrenOfType<Property>();
    }

    /// Remove the Property with the given name, if present.
    void removeProperty(const string& name)
    {
        removeChildOfType<Property>(name);
    }

  public:
    static const string CATEGORY;
};

/// @class PropertySetAssign
/// A property set assignment element within a Look.
class PropertySetAssign : public GeomElement
{
  public:
    PropertySetAssign(ElementPtr parent, const string& name) :
        GeomElement(parent, CATEGORY, name)
    {
    }
    virtual ~PropertySetAssign() { }

  public:
    static const string CATEGORY;
};

} // namespace MaterialX

#endif
