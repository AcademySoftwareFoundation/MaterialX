//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GEOM_H
#define MATERIALX_GEOM_H

/// @file
/// Geometric element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Element.h>

namespace MaterialX
{

extern const string UNIVERSAL_GEOM_NAME;
extern const string UDIM_TOKEN;
extern const string UV_TILE_TOKEN;

class GeomElement;
class GeomAttr;
class GeomInfo;
class Collection;
class CollectionAdd;
class CollectionRemove;

/// A shared pointer to a GeomElement
using GeomElementPtr = shared_ptr<GeomElement>;
/// A shared pointer to a const GeomElement
using ConstGeomElementPtr = shared_ptr<const GeomElement>;

/// A shared pointer to a GeomAttr
using GeomAttrPtr = shared_ptr<GeomAttr>;
/// A shared pointer to a const GeomAttr
using ConstGeomAttrPtr = shared_ptr<const GeomAttr>;

/// A shared pointer to a GeomInfo
using GeomInfoPtr = shared_ptr<GeomInfo>;
/// A shared pointer to a const GeomInfo
using ConstGeomInfoPtr = shared_ptr<const GeomInfo>;

/// A shared pointer to a Collection
using CollectionPtr = shared_ptr<Collection>;
/// A shared pointer to a const Collection
using ConstCollectionPtr = shared_ptr<const Collection>;

/// A shared pointer to a CollectionAdd
using CollectionAddPtr = shared_ptr<CollectionAdd>;
/// A shared pointer to a const CollectionAdd
using ConstCollectionAddPtr = shared_ptr<const CollectionAdd>;

/// A shared pointer to a CollectionRemove
using CollectionRemovePtr = shared_ptr<CollectionRemove>;
/// A shared pointer to a const CollectionRemove
using ConstCollectionRemovePtr = shared_ptr<const CollectionRemove>;

/// @class GeomElement
/// The base class for geometric elements, which support bindings to geometries
/// and geometric collections.
class GeomElement : public Element
{
  protected:
    GeomElement(ElementPtr parent, const string& category, const string& name) :
        Element(parent, category, name)
    {
    }
  public:
    virtual ~GeomElement() { }

    /// @name Geometry
    /// @{

    /// Set the geometry string of this element.
    void setGeom(const string& geom)
    {
        setAttribute(GEOM_ATTRIBUTE, geom);
    }

    /// Return true if this element has a geometry string.
    bool hasGeom()
    {
        return hasAttribute(GEOM_ATTRIBUTE);
    }

    /// Return the geometry string of this element.
    const string& getGeom() const
    {
        return getAttribute(GEOM_ATTRIBUTE);
    }

    /// @}
    /// @name Collection
    /// @{

    /// Set the collection string of this element.
    void setCollectionString(const string& collection)
    {
        setAttribute(COLLECTION_ATTRIBUTE, collection);
    }

    /// Return true if this element has a collection string.
    bool hasCollectionString()
    {
        return hasAttribute(COLLECTION_ATTRIBUTE);
    }

    /// Return the collection string of this element.
    const string& getCollectionString() const
    {
        return getAttribute(COLLECTION_ATTRIBUTE);
    }

    /// Assign a Collection to this element.
    void setCollection(ConstCollectionPtr collection);

    /// Return the Collection that is assigned to this element.
    CollectionPtr getCollection() const;

    /// @}

  public:
    static const string GEOM_ATTRIBUTE;
    static const string COLLECTION_ATTRIBUTE;
};

/// @class GeomInfo
/// A geometry info element within a Document.
class GeomInfo : public GeomElement
{
  public:
    GeomInfo(ElementPtr parent, const string& name) :
        GeomElement(parent, CATEGORY, name)
    {
    }
    virtual ~GeomInfo() { }

    /// @name GeomAttr Elements
    /// @{

    /// Add a GeomAttr to this element.
    /// @param name The name of the new GeomAttr.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new GeomAttr.
    GeomAttrPtr addGeomAttr(const string& name = EMPTY_STRING)
    {
        return addChild<GeomAttr>(name);
    }

    /// Return the GeomAttr, if any, with the given name.
    GeomAttrPtr getGeomAttr(const string& name) const
    {
        return getChildOfType<GeomAttr>(name);
    }

    /// Return a vector of all GeomAttr elements in the element.
    vector<GeomAttrPtr> getGeomAttrs() const
    {
        return getChildrenOfType<GeomAttr>();
    }

    /// Remove the GeomAttr, if any, with the given name.
    void removeGeomAttr(const string& name)
    {
        removeChildOfType<GeomAttr>(name);
    }

    /// Set the value of a geomattr by its name, creating a child element
    /// to hold the geomattr if needed.
    template<class T> GeomAttrPtr setGeomAttrValue(const string& name,
                                                   const T& value,
                                                   const string& type = EMPTY_STRING);

    /// @}

  public:
    static const string CATEGORY;
};

/// @class GeomAttr
/// A geometry attribute element within a GeomInfo.
class GeomAttr : public ValueElement
{
  public:
    GeomAttr(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~GeomAttr() { }

  public:
    static const string CATEGORY;
};

/// @class Collection
/// A collection element within a Document.
/// @todo Add a Collection::containsGeom method that computes whether the
///     given Collection contains the specified geometry.
class Collection : public Element
{
  public:
    Collection(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~Collection() { }

    /// @name CollectionAdd Elements
    /// @{

    /// Add a CollectionAdd to the collection.
    /// @param name The name of the new CollectionAdd.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new CollectionAdd.
    CollectionAddPtr addCollectionAdd(const string& name = EMPTY_STRING)
    {
        return addChild<CollectionAdd>(name);
    }

    /// Return the CollectionAdd, if any, with the given name.
    CollectionAddPtr getCollectionAdd(const string& name) const
    {
        return getChildOfType<CollectionAdd>(name);
    }

    /// Return a vector of all CollectionAdd elements in the collection
    vector<CollectionAddPtr> getCollectionAdds() const
    {
        return getChildrenOfType<CollectionAdd>();
    }

    /// Remove the CollectionAdd, if any, with the given name.
    void removeCollectionAdd(const string& name)
    {
        removeChildOfType<CollectionAdd>(name);
    }

    /// @}
    /// @name CollectionRemove Elements
    /// @{

    /// Add a CollectionRemove to the collection.
    /// @param name The name of the new CollectionRemove.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new CollectionRemove.
    CollectionRemovePtr addCollectionRemove(const string& name = EMPTY_STRING)
    {
        return addChild<CollectionRemove>(name);
    }

    /// Return the CollectionRemove, if any, with the given name.
    CollectionRemovePtr getCollectionRemove(const string& name) const
    {
        return getChildOfType<CollectionRemove>(name);
    }

    /// Return a vector of all CollectionRemove elements in the collection
    vector<CollectionRemovePtr> getCollectionRemoves() const
    {
        return getChildrenOfType<CollectionRemove>();
    }

    /// Remove the CollectionRemove, if any, with the given name.
    void removeCollectionRemove(const string& name)
    {
        removeChildOfType<CollectionRemove>(name);
    }

    /// @}

  public:
    static const string CATEGORY;
};

/// @class CollectionAdd
/// A collection add element within a Collection.
class CollectionAdd : public GeomElement
{
  public:
    CollectionAdd(ElementPtr parent, const string& name) :
        GeomElement(parent, CATEGORY, name)
    {
    }
    virtual ~CollectionAdd() { }

  public:
    static const string CATEGORY;
};

/// @class CollectionRemove
/// A collection remove element within a Collection.
class CollectionRemove : public GeomElement
{
  public:
    CollectionRemove(ElementPtr parent, const string& name) :
        GeomElement(parent, CATEGORY, name)
    {
    }
    virtual ~CollectionRemove() { }

  public:
    static const string CATEGORY;
};

template<class T> GeomAttrPtr GeomInfo::setGeomAttrValue(const string& name,
                                                         const T& value,
                                                         const string& type)
{
    GeomAttrPtr geomAttr = getChildOfType<GeomAttr>(name);
    if (!geomAttr)
        geomAttr = addGeomAttr(name);
    geomAttr->setValue(value, type);
    return geomAttr;
}

/// Given two geometry strings, each containing an array of geom names, return
/// true if they have any geometries in common.  The universal geom name "*"
/// matches all geometries.
/// @todo The full set of pattern matching rules in the specification is not
///    yet supported, and only the universal geom name is currently handled.
/// @relates GeomInfo
bool geomStringsMatch(const string& geom1, const string& geom2);

} // namespace MaterialX

#endif
