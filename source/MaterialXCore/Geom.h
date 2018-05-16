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

/// A shared pointer to a GeomElement
using GeomElementPtr = shared_ptr<class GeomElement>;
/// A shared pointer to a const GeomElement
using ConstGeomElementPtr = shared_ptr<const class GeomElement>;

/// A shared pointer to a GeomAttr
using GeomAttrPtr = shared_ptr<class GeomAttr>;
/// A shared pointer to a const GeomAttr
using ConstGeomAttrPtr = shared_ptr<const class GeomAttr>;

/// A shared pointer to a GeomInfo
using GeomInfoPtr = shared_ptr<class GeomInfo>;
/// A shared pointer to a const GeomInfo
using ConstGeomInfoPtr = shared_ptr<const class GeomInfo>;

/// A shared pointer to a Collection
using CollectionPtr = shared_ptr<class Collection>;
/// A shared pointer to a const Collection
using ConstCollectionPtr = shared_ptr<const class Collection>;

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

    /// Return a vector of all GeomAttr elements.
    vector<GeomAttrPtr> getGeomAttrs() const
    {
        return getChildrenOfType<GeomAttr>();
    }

    /// Remove the GeomAttr, if any, with the given name.
    void removeGeomAttr(const string& name)
    {
        removeChildOfType<GeomAttr>(name);
    }

    /// @}
    /// @name Tokens
    /// @{

    /// Add a Token to this element.
    /// @param name The name of the new Token.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new Token.
    TokenPtr addToken(const string& name = EMPTY_STRING)
    {
        return addChild<Token>(name);
    }

    /// Return the Token, if any, with the given name.
    TokenPtr getToken(const string& name) const
    {
        return getChildOfType<Token>(name);
    }

    /// Return a vector of all Token elements.
    vector<TokenPtr> getTokens() const
    {
        return getChildrenOfType<Token>();
    }

    /// Remove the Token, if any, with the given name.
    void removeToken(const string& name)
    {
        removeChildOfType<Token>(name);
    }

    /// @}
    /// @name Values
    /// @{

    /// Set the value of a GeomAttr by its name, creating a child element
    /// to hold the GeomAttr if needed.
    template<class T> GeomAttrPtr setGeomAttrValue(const string& name,
                                                   const T& value,
                                                   const string& type = EMPTY_STRING);

    /// Set the string value of a Token by its name, creating a child element
    /// to hold the Token if needed.
    TokenPtr setTokenValue(const string& name, const string& value)
    {
        TokenPtr token = getToken(name);
        if (!token)
            token = addToken(name);
        token->setValue<std::string>(value);
        return token;
    }

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

    /// @name Include Geometry
    /// @{

    /// Set the include geometry string of this element.
    void setIncludeGeom(const string& geom)
    {
        setAttribute(INCLUDE_GEOM_ATTRIBUTE, geom);
    }

    /// Return true if this element has an include geometry string.
    bool hasIncludeGeom()
    {
        return hasAttribute(INCLUDE_GEOM_ATTRIBUTE);
    }

    /// Return the include geometry string of this element.
    const string& getIncludeGeom() const
    {
        return getAttribute(INCLUDE_GEOM_ATTRIBUTE);
    }

    /// @}
    /// @name Include Collection
    /// @{

    /// Set the include collection string of this element.
    void setIncludeCollection(const string& collection)
    {
        setAttribute(INCLUDE_COLLECTION_ATTRIBUTE, collection);
    }

    /// Return true if this element has an include collection string.
    bool hasIncludeCollection()
    {
        return hasAttribute(INCLUDE_COLLECTION_ATTRIBUTE);
    }

    /// Return the include collection string of this element.
    const string& getIncludeCollection() const
    {
        return getAttribute(INCLUDE_COLLECTION_ATTRIBUTE);
    }

    /// @}
    /// @name Exclude Geometry
    /// @{

    /// Set the exclude geometry string of this element.
    void setExcludeGeom(const string& geom)
    {
        setAttribute(EXCLUDE_GEOM_ATTRIBUTE, geom);
    }

    /// Return true if this element has an exclude geometry string.
    bool hasExcludeGeom()
    {
        return hasAttribute(EXCLUDE_GEOM_ATTRIBUTE);
    }

    /// Return the exclude geometry string of this element.
    const string& getExcludeGeom() const
    {
        return getAttribute(EXCLUDE_GEOM_ATTRIBUTE);
    }

    /// @}
    /// @name Exclude Collection
    /// @{

    /// Set the exclude collection string of this element.
    void setExcludeCollection(const string& collection)
    {
        setAttribute(EXCLUDE_COLLECTION_ATTRIBUTE, collection);
    }

    /// Return true if this element has an exclude collection string.
    bool hasExcludeCollection()
    {
        return hasAttribute(EXCLUDE_COLLECTION_ATTRIBUTE);
    }

    /// Return the exclude collection string of this element.
    const string& getExcludeCollection() const
    {
        return getAttribute(EXCLUDE_COLLECTION_ATTRIBUTE);
    }

    /// @}

  public:
    static const string CATEGORY;
    static const string INCLUDE_GEOM_ATTRIBUTE;
    static const string INCLUDE_COLLECTION_ATTRIBUTE;
    static const string EXCLUDE_GEOM_ATTRIBUTE;
    static const string EXCLUDE_COLLECTION_ATTRIBUTE;
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
