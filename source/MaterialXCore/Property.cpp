//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Property.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string PropertyAssign::GEOM_ATTRIBUTE = "geom";
const string PropertyAssign::COLLECTION_ATTRIBUTE = "collection";

void PropertyAssign::setCollection(ConstCollectionPtr collection)
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

CollectionPtr PropertyAssign::getCollection() const
{
    return resolveRootNameReference<Collection>(getCollectionString());
}

} // namespace MaterialX
