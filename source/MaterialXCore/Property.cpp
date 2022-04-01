//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Property.h>

MATERIALX_NAMESPACE_BEGIN

const string PropertyAssign::PROPERTY_ATTRIBUTE = "property";
const string PropertyAssign::GEOM_ATTRIBUTE = "geom";
const string PropertyAssign::COLLECTION_ATTRIBUTE = "collection";
const string PropertySetAssign::PROPERTY_SET_ATTRIBUTE = "propertyset";

//
// PropertyAssign methods
//

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

//
// PropertySetAssign methods
//

void PropertySetAssign::setPropertySet(ConstPropertySetPtr propertySet)
{
    if (propertySet)
    {
        setPropertySetString(propertySet->getName());
    }
    else
    {
        removeAttribute(PROPERTY_SET_ATTRIBUTE);
    }
}

PropertySetPtr PropertySetAssign::getPropertySet() const
{
    return resolveRootNameReference<PropertySet>(getPropertySetString());
}

MATERIALX_NAMESPACE_END
