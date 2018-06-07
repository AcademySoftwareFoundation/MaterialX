//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Geom.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_GEOMINFO_FUNC_INSTANCE(NAME, T) \
.def("_setGeomAttrValue" #NAME, &mx::GeomInfo::setGeomAttrValue<T>)

void bindPyGeom(py::module& mod)
{
    py::class_<mx::GeomElement, mx::GeomElementPtr, mx::Element>(mod, "GeomElement")
        .def("setGeom", &mx::GeomElement::setGeom)
        .def("hasGeom", &mx::GeomElement::hasGeom)
        .def("getGeom", &mx::GeomElement::getGeom)
        .def("setCollectionString", &mx::GeomElement::setCollectionString)
        .def("hasCollectionString", &mx::GeomElement::hasCollectionString)
        .def("getCollectionString", &mx::GeomElement::getCollectionString)
        .def("setCollection", &mx::GeomElement::setCollection)
        .def("getCollection", &mx::GeomElement::getCollection);

    py::class_<mx::GeomInfo, mx::GeomInfoPtr, mx::GeomElement>(mod, "GeomInfo")
        .def("_addGeomAttr", &mx::GeomInfo::addGeomAttr)
        .def("getGeomAttr", &mx::GeomInfo::getGeomAttr)
        .def("getGeomAttrs", &mx::GeomInfo::getGeomAttrs)
        .def("removeGeomAttr", &mx::GeomInfo::removeGeomAttr)
        .def("addToken", &mx::GeomInfo::addToken,
            py::arg("name") = mx::DEFAULT_TYPE_STRING)
        .def("getToken", &mx::GeomInfo::getToken)
        .def("getTokens", &mx::GeomInfo::getTokens)
        .def("removeToken", &mx::GeomInfo::removeToken)
        .def("setTokenValue", &mx::GeomInfo::setTokenValue)
        BIND_GEOMINFO_FUNC_INSTANCE(integer, int)
        BIND_GEOMINFO_FUNC_INSTANCE(boolean, bool)
        BIND_GEOMINFO_FUNC_INSTANCE(float, float)
        BIND_GEOMINFO_FUNC_INSTANCE(color2, mx::Color2)
        BIND_GEOMINFO_FUNC_INSTANCE(color3, mx::Color3)
        BIND_GEOMINFO_FUNC_INSTANCE(color4, mx::Color4)
        BIND_GEOMINFO_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_GEOMINFO_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_GEOMINFO_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix33, mx::Matrix33)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix44, mx::Matrix44)
        BIND_GEOMINFO_FUNC_INSTANCE(string, std::string)
        .def_readonly_static("CATEGORY", &mx::GeomInfo::CATEGORY);

    py::class_<mx::GeomAttr, mx::GeomAttrPtr, mx::ValueElement>(mod, "GeomAttr")
        .def_readonly_static("CATEGORY", &mx::GeomAttr::CATEGORY);

    py::class_<mx::Collection, mx::CollectionPtr, mx::Element>(mod, "Collection")
        .def("setIncludeGeom", &mx::Collection::setIncludeGeom)
        .def("hasIncludeGeom", &mx::Collection::hasIncludeGeom)
        .def("getIncludeGeom", &mx::Collection::getIncludeGeom)
        .def("setExcludeGeom", &mx::Collection::setExcludeGeom)
        .def("hasExcludeGeom", &mx::Collection::hasExcludeGeom)
        .def("getExcludeGeom", &mx::Collection::getExcludeGeom)
        .def("setIncludeCollectionString", &mx::Collection::setIncludeCollectionString)
        .def("hasIncludeCollectionString", &mx::Collection::hasIncludeCollectionString)
        .def("getIncludeCollectionString", &mx::Collection::getIncludeCollectionString)
        .def("setIncludeCollection", &mx::Collection::setIncludeCollection)
        .def("setIncludeCollections", &mx::Collection::setIncludeCollections)
        .def("getIncludeCollections", &mx::Collection::getIncludeCollections)
        .def("hasIncludeCycle", &mx::Collection::hasIncludeCycle)
        .def("matchesGeomString", &mx::Collection::matchesGeomString)
        .def_readonly_static("CATEGORY", &mx::Collection::CATEGORY);

    mod.def("geomStringsMatch", &mx::geomStringsMatch);
}
