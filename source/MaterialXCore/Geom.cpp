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
const string Collection::INCLUDE_COLLECTION_ATTRIBUTE = "includecollection";
const string Collection::EXCLUDE_GEOM_ATTRIBUTE = "excludegeom";

bool geomStringsMatch(const string& geom1, const string& geom2)
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
            if (path2.isMatching(path1))
            {
                return true;
            }
        }
    }
    return false;
}

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
    return getDocument()->getCollection(getCollectionString());
}

} // namespace MaterialX
