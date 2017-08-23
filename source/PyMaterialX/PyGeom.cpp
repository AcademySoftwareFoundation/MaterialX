//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Geom.h>

#include <PyBind11/stl.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_GEOMINFO_FUNC_INSTANCE(NAME, T) \
.def("_setGeomAttrValue" #NAME, &mx::GeomInfo::setGeomAttrValue<T>)

void bindPyGeom(py::module& mod)
{
    py::class_<mx::GeomElement, mx::GeomElementPtr, mx::Element>(mod, "GeomElement", py::metaclass())
        .def("setGeom", &mx::GeomElement::setGeom)
        .def("getGeom", &mx::GeomElement::getGeom)
        .def("setCollection", &mx::GeomElement::setCollection)
        .def("getCollection", &mx::GeomElement::getCollection);

    py::class_<mx::GeomInfo, mx::GeomInfoPtr, mx::GeomElement>(mod, "GeomInfo", py::metaclass())
        .def("_addGeomAttr", &mx::GeomInfo::addGeomAttr)
        .def("getGeomAttrs", &mx::GeomInfo::getGeomAttrs)
        .def("removeGeomAttr", &mx::GeomInfo::removeGeomAttr)
        BIND_GEOMINFO_FUNC_INSTANCE(integer, int)
        BIND_GEOMINFO_FUNC_INSTANCE(boolean, bool)
        BIND_GEOMINFO_FUNC_INSTANCE(float, float)
        BIND_GEOMINFO_FUNC_INSTANCE(color2, mx::Color2)
        BIND_GEOMINFO_FUNC_INSTANCE(color3, mx::Color3)
        BIND_GEOMINFO_FUNC_INSTANCE(color4, mx::Color4)
        BIND_GEOMINFO_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_GEOMINFO_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_GEOMINFO_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix33, mx::Matrix3x3)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix44, mx::Matrix4x4)
        BIND_GEOMINFO_FUNC_INSTANCE(string, std::string)
        .def_readonly_static("CATEGORY", &mx::GeomInfo::CATEGORY);

    py::class_<mx::GeomAttr, mx::GeomAttrPtr, mx::ValueElement>(mod, "GeomAttr", py::metaclass())
        .def_readonly_static("CATEGORY", &mx::GeomAttr::CATEGORY);

    py::class_<mx::Collection, mx::CollectionPtr, mx::Element>(mod, "Collection", py::metaclass())
        .def("addCollectionAdd", &mx::Collection::addCollectionAdd,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getCollectionAdd", &mx::Collection::getCollectionAdd)
        .def("getCollectionAdds", &mx::Collection::getCollectionAdds)
        .def("removeCollectionAdd", &mx::Collection::removeCollectionAdd)
        .def("addCollectionRemove", &mx::Collection::addCollectionRemove,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getCollectionRemove", &mx::Collection::getCollectionRemove)
        .def("getCollectionRemoves", &mx::Collection::getCollectionRemoves)
        .def("removeCollectionRemove", &mx::Collection::removeCollectionRemove)
        .def_readonly_static("CATEGORY", &mx::Collection::CATEGORY);

    py::class_<mx::CollectionAdd, mx::CollectionAddPtr, mx::GeomElement>(mod, "CollectionAdd", py::metaclass())
        .def_readonly_static("CATEGORY", &mx::CollectionAdd::CATEGORY);

    py::class_<mx::CollectionRemove, mx::CollectionRemovePtr, mx::GeomElement>(mod, "CollectionRemove", py::metaclass())
        .def_readonly_static("CATEGORY", &mx::CollectionRemove::CATEGORY);

    mod.def("geomStringsMatch", &mx::geomStringsMatch);
}
