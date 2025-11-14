//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Geom.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_GEOMINFO_FUNC_INSTANCE(NAME, T) \
.def("_setGeomPropValue" #NAME, &mx::GeomInfo::setGeomPropValue<T>)

void bindPyGeom(py::module& mod)
{
    py::class_<mx::GeomElement, mx::GeomElementPtr, mx::Element>(mod, "GeomElement", "The base class for geometric elements, which support bindings to geometries and geometric collections.")
        .def("setGeom", &mx::GeomElement::setGeom, "Set the geometry string of this element.")
        .def("hasGeom", &mx::GeomElement::hasGeom, "Return true if this element has a geometry string.")
        .def("getGeom", &mx::GeomElement::getGeom, "Return the geometry string of this element.")
        .def("setCollectionString", &mx::GeomElement::setCollectionString, "Set the collection string of this element.")
        .def("hasCollectionString", &mx::GeomElement::hasCollectionString, "Return true if this element has a collection string.")
        .def("getCollectionString", &mx::GeomElement::getCollectionString, "Return the collection string of this element.")
        .def("setCollection", &mx::GeomElement::setCollection, "Assign a Collection to this element.")
        .def("getCollection", &mx::GeomElement::getCollection, "Return the Collection, if any, with the given name.");

    py::class_<mx::GeomInfo, mx::GeomInfoPtr, mx::GeomElement>(mod, "GeomInfo", "A geometry info element within a Document.")
        .def("addGeomProp", &mx::GeomInfo::addGeomProp, "Add a GeomProp to this element.\n\nArgs:\n    name: The name of the new GeomProp. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new GeomProp.")
        .def("getGeomProp", &mx::GeomInfo::getGeomProp, "Return the GeomProp, if any, with the given name.")
        .def("getGeomProps", &mx::GeomInfo::getGeomProps, "Return a vector of all GeomProp elements.")
        .def("removeGeomProp", &mx::GeomInfo::removeGeomProp, "Remove the GeomProp, if any, with the given name.")
        .def("addToken", &mx::GeomInfo::addToken,
            py::arg("name") = mx::DEFAULT_TYPE_STRING, "Add a Token to this element.\n\nArgs:\n    name: The name of the new Token. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Token.")
        .def("getToken", &mx::GeomInfo::getToken, "Return the Token, if any, with the given name.")
        .def("getTokens", &mx::GeomInfo::getTokens, "Return a vector of all Token elements.")
        .def("removeToken", &mx::GeomInfo::removeToken, "Remove the Token, if any, with the given name.")
        .def("setTokenValue", &mx::GeomInfo::setTokenValue, "Set the string value of a Token by its name, creating a child element to hold the Token if needed.")
        BIND_GEOMINFO_FUNC_INSTANCE(integer, int)
        BIND_GEOMINFO_FUNC_INSTANCE(boolean, bool)
        BIND_GEOMINFO_FUNC_INSTANCE(float, float)
        BIND_GEOMINFO_FUNC_INSTANCE(color3, mx::Color3)
        BIND_GEOMINFO_FUNC_INSTANCE(color4, mx::Color4)
        BIND_GEOMINFO_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_GEOMINFO_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_GEOMINFO_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix33, mx::Matrix33)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix44, mx::Matrix44)
        BIND_GEOMINFO_FUNC_INSTANCE(string, std::string)
        BIND_GEOMINFO_FUNC_INSTANCE(integerarray, mx::IntVec)
        BIND_GEOMINFO_FUNC_INSTANCE(booleanarray, mx::BoolVec)
        BIND_GEOMINFO_FUNC_INSTANCE(floatarray, mx::FloatVec)
        BIND_GEOMINFO_FUNC_INSTANCE(stringarray, mx::StringVec)
        .def_readonly_static("CATEGORY", &mx::GeomInfo::CATEGORY);

    py::class_<mx::GeomProp, mx::GeomPropPtr, mx::ValueElement>(mod, "GeomProp", "A geometric property element within a GeomInfo.")
        .def_readonly_static("CATEGORY", &mx::GeomProp::CATEGORY);

    py::class_<mx::GeomPropDef, mx::GeomPropDefPtr, mx::TypedElement>(mod, "GeomPropDef", "An element representing a declaration of geometric property data.\n\nA GeomPropDef element contains a reference to a geometric node and a set of modifiers for that node. For example, a world-space normal can be declared as a reference to the \"normal\" geometric node with a space setting of \"world\", or a specific set of texture coordinates can be declared as a reference to the \"texcoord\" geometric node with an index setting of \"1\".")
        .def("setGeomProp", &mx::GeomPropDef::setGeomProp, "Set the geometric property string of this element.")
        .def("hasGeomProp", &mx::GeomPropDef::hasGeomProp, "Return true if this element has a geometric property string.")
        .def("getGeomProp", &mx::GeomPropDef::getGeomProp, "Return the GeomProp, if any, with the given name.")
        .def("setSpace", &mx::GeomPropDef::setSpace, "Set the geometric space string of this element.")
        .def("hasSpace", &mx::GeomPropDef::hasSpace, "Return true if this element has a geometric space string.")
        .def("getSpace", &mx::GeomPropDef::getSpace, "Return the geometric space string of this element.")
        .def("setIndex", &mx::GeomPropDef::setIndex, "Set the index string of this element.")
        .def("hasIndex", &mx::GeomPropDef::hasIndex, "Return true if this element has an index string.")
        .def("getIndex", &mx::GeomPropDef::getIndex, "Return the index string of this element.")
        .def("setGeomProp", &mx::GeomPropDef::setGeomProp, "Set the geometric property string of this element.")
        .def("hasGeomProp", &mx::GeomPropDef::hasGeomProp, "Return true if this element has a geometric property string.")
        .def("getGeomProp", &mx::GeomPropDef::getGeomProp, "Return the GeomProp, if any, with the given name.")
        .def_readonly_static("CATEGORY", &mx::GeomPropDef::CATEGORY);

    py::class_<mx::Collection, mx::CollectionPtr, mx::Element>(mod, "Collection", "A collection element within a Document.")
        .def("setIncludeGeom", &mx::Collection::setIncludeGeom, "Set the include geometry string of this element.")
        .def("hasIncludeGeom", &mx::Collection::hasIncludeGeom, "Return true if this element has an include geometry string.")
        .def("getIncludeGeom", &mx::Collection::getIncludeGeom, "Return the include geometry string of this element.")
        .def("setExcludeGeom", &mx::Collection::setExcludeGeom, "Set the exclude geometry string of this element.")
        .def("hasExcludeGeom", &mx::Collection::hasExcludeGeom, "Return true if this element has an exclude geometry string.")
        .def("getExcludeGeom", &mx::Collection::getExcludeGeom, "Return the exclude geometry string of this element.")
        .def("setIncludeCollectionString", &mx::Collection::setIncludeCollectionString, "Set the include collection string of this element.")
        .def("hasIncludeCollectionString", &mx::Collection::hasIncludeCollectionString, "Return true if this element has an include collection string.")
        .def("getIncludeCollectionString", &mx::Collection::getIncludeCollectionString, "Return the include collection string of this element.")
        .def("setIncludeCollection", &mx::Collection::setIncludeCollection, "Set the collection that is directly included by this element.")
        .def("setIncludeCollections", &mx::Collection::setIncludeCollections, "Set the vector of collections that are directly included by this element.")
        .def("getIncludeCollections", &mx::Collection::getIncludeCollections, "Return the vector of collections that are directly included by this element.")
        .def("hasIncludeCycle", &mx::Collection::hasIncludeCycle, "Return true if the include chain for this element contains a cycle.")
        .def("matchesGeomString", &mx::Collection::matchesGeomString, "Return true if this collection and the given geometry string have any geometries in common.")
        .def_readonly_static("CATEGORY", &mx::Collection::CATEGORY);

    mod.def("geomStringsMatch", &mx::geomStringsMatch);

    mod.attr("GEOM_PATH_SEPARATOR") = mx::GEOM_PATH_SEPARATOR;
    mod.attr("UNIVERSAL_GEOM_NAME") = mx::UNIVERSAL_GEOM_NAME;
    mod.attr("UDIM_TOKEN") = mx::UDIM_TOKEN;
    mod.attr("UV_TILE_TOKEN") = mx::UV_TILE_TOKEN;
    mod.attr("UDIM_SET_PROPERTY") = mx::UDIM_SET_PROPERTY;
}
