//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Geom.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string GEOM_PATH_SEPARATOR = "/";
const string UNIVERSAL_GEOM_NAME = GEOM_PATH_SEPARATOR;
const string UDIM_TOKEN = "%UDIM";
const string UV_TILE_TOKEN = "%UVTILE";

const string GeomElement::GEOM_ATTRIBUTE = "geom";
const string GeomElement::COLLECTION_ATTRIBUTE = "collection";
const string Collection::INCLUDE_GEOM_ATTRIBUTE = "includegeom";
const string Collection::EXCLUDE_GEOM_ATTRIBUTE = "excludegeom";
const string Collection::INCLUDE_COLLECTION_ATTRIBUTE = "includecollection";

bool geomStringsMatch(const string& geom1, const string& geom2, bool contains)
{
    vector<GeomPath> paths1;
    for (const string& name1 : splitString(geom1, ARRAY_VALID_SEPARATORS))
    {
        paths1.push_back(GeomPath(name1));
    }
    for (const string& name2 : splitString(geom2, ARRAY_VALID_SEPARATORS))
    {
        GeomPath path2(name2);
        for (const GeomPath& path1 : paths1)
        {
            if (path1.isMatching(path2, contains))
            {
                return true;
            }
        }
    }
    return false;
}

//
// GeomElement methods
//

void GeomElement::setCollection(ConstCollectionPtr collection)
{
    if (collection)
    {
        setCollectionString(collection->getName());
    }
    else
    {
        removeAttribute(COLLECTION_ATTRIBUTE);
    }
}

CollectionPtr GeomElement::getCollection() const
{
    return resolveRootNameReference<Collection>(getCollectionString());
}

bool GeomElement::validate(string* message) const
{
    bool res = true;
    if (hasCollectionString())
    {
        validateRequire(getCollection() != nullptr, res, message, "Invalid collection string");
    }
    return Element::validate(message) && res;
}

//
// Collection methods
//

void Collection::setIncludeCollection(ConstCollectionPtr collection)
{
    if (collection)
    {
        setIncludeCollectionString(collection->getName());
    }
    else
    {
        removeAttribute(INCLUDE_COLLECTION_ATTRIBUTE);
    }
}

CollectionPtr Collection::getIncludeCollection() const
{
    return resolveRootNameReference<Collection>(getIncludeCollectionString());
}

bool Collection::hasIncludeCycle() const
{
    try
    {
        matchesGeomString(UNIVERSAL_GEOM_NAME);
    }
    catch (ExceptionFoundCycle&)
    {
        return true;
    }
    return false;
}

bool Collection::matchesGeomString(const string& geom) const
{
    if (geomStringsMatch(getActiveExcludeGeom(), geom, true))
    {
        return false;
    }
    if (geomStringsMatch(getActiveIncludeGeom(), geom))
    {
        return true;
    }

    std::set<ConstCollectionPtr> includedSet;
    ConstCollectionPtr included = getIncludeCollection();
    while (included)
    {
        if (includedSet.count(included))
        {
            throw ExceptionFoundCycle("Encountered a cycle in collection: " + getName());
        }
        includedSet.insert(included);
        included = included->getIncludeCollection();
    }
    for (ConstCollectionPtr collection : includedSet)
    {
        if (collection->matchesGeomString(geom))
        {
            return true;
        }
    }

    return false;
}

bool Collection::validate(string* message) const
{
    bool res = true;
    validateRequire(!hasIncludeCycle(), res, message, "Cycle in collection include chain");
    return Element::validate(message) && res;
}

} // namespace MaterialX
