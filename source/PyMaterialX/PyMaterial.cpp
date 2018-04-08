//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Material.h>

#include <MaterialXCore/Look.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_MATERIAL_FUNC_INSTANCE(NAME, T) \
.def("_setOverrideValue" #NAME, &mx::Material::setOverrideValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyMaterial(py::module& mod)
{
    py::class_<mx::Material, mx::MaterialPtr, mx::Element>(mod, "Material")
        .def("_addShaderRef", &mx::Material::addShaderRef,
            py::arg("name") = mx::EMPTY_STRING, py::arg("node") = mx::EMPTY_STRING)
        .def("getShaderRef", &mx::Material::getShaderRef)
        .def("getShaderRefs", &mx::Material::getShaderRefs)
        .def("getActiveShaderRefs", &mx::Material::getActiveShaderRefs)
        .def("removeShaderRef", &mx::Material::removeShaderRef)
        .def("_addOverride", &mx::Material::addOverride)
        .def("getOverride", &mx::Material::getOverride)
        .def("getOverrides", &mx::Material::getOverrides)
        .def("getActiveOverrides", &mx::Material::getActiveOverrides)
        .def("removeOverride", &mx::Material::removeOverride)
        .def("addMaterialInherit", &mx::Material::addMaterialInherit,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getMaterialInherit", &mx::Material::getMaterialInherit)
        .def("getMaterialInherits", &mx::Material::getMaterialInherits)
        .def("removeMaterialInherit", &mx::Material::removeMaterialInherit)
        .def("getShaderNodeDefs", &mx::Material::getShaderNodeDefs,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getReferencingMaterialAssigns", &mx::Material::getReferencingMaterialAssigns)
        .def("getPrimaryShaderNodeDef", &mx::Material::getPrimaryShaderNodeDef,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderName", &mx::Material::getPrimaryShaderName,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderParameters", &mx::Material::getPrimaryShaderParameters,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderInputs", &mx::Material::getPrimaryShaderInputs,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getBoundGeomStrings", &mx::Material::getBoundGeomStrings)
        .def("getBoundGeomCollections", &mx::Material::getBoundGeomCollections)
        BIND_MATERIAL_FUNC_INSTANCE(integer, int)
        BIND_MATERIAL_FUNC_INSTANCE(boolean, bool)
        BIND_MATERIAL_FUNC_INSTANCE(float, float)
        BIND_MATERIAL_FUNC_INSTANCE(color2, mx::Color2)
        BIND_MATERIAL_FUNC_INSTANCE(color3, mx::Color3)
        BIND_MATERIAL_FUNC_INSTANCE(color4, mx::Color4)
        BIND_MATERIAL_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_MATERIAL_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_MATERIAL_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_MATERIAL_FUNC_INSTANCE(matrix33, mx::Matrix33)
        BIND_MATERIAL_FUNC_INSTANCE(matrix44, mx::Matrix44)
        BIND_MATERIAL_FUNC_INSTANCE(string, std::string)
        .def_readonly_static("CATEGORY", &mx::Material::CATEGORY);

    py::class_<mx::BindParam, mx::BindParamPtr, mx::ValueElement>(mod, "BindParam")
        .def_readonly_static("CATEGORY", &mx::BindParam::CATEGORY);

    py::class_<mx::BindInput, mx::BindInputPtr, mx::ValueElement>(mod, "BindInput")
        .def("setNodeGraphString", &mx::BindInput::setNodeGraphString)
        .def("getNodeGraphString", &mx::BindInput::getNodeGraphString)
        .def("setOutputString", &mx::BindInput::setOutputString)
        .def("getOutputString", &mx::BindInput::getOutputString)
        .def("setConnectedOutput", &mx::BindInput::setConnectedOutput)
        .def("getConnectedOutput", &mx::BindInput::getConnectedOutput)
        .def_readonly_static("CATEGORY", &mx::BindInput::CATEGORY);

    py::class_<mx::ShaderRef, mx::ShaderRefPtr, mx::Element>(mod, "ShaderRef")
        .def("setNodeString", &mx::ShaderRef::setNodeString)
        .def("hasNodeString", &mx::ShaderRef::hasNodeString)
        .def("getNodeString", &mx::ShaderRef::getNodeString)
        .def("setNodeDefString", &mx::ShaderRef::setNodeDefString)
        .def("hasNodeDefString", &mx::ShaderRef::hasNodeDefString)
        .def("getNodeDefString", &mx::ShaderRef::getNodeDefString)
        .def("getNodeDef", &mx::ShaderRef::getNodeDef)
        .def("addBindParam", &mx::ShaderRef::addBindParam,
            py::arg("name"), py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getBindParam", &mx::ShaderRef::getBindParam)
        .def("getBindParams", &mx::ShaderRef::getBindParams)
        .def("removeBindParam", &mx::ShaderRef::removeBindParam)
        .def("addBindInput", &mx::ShaderRef::addBindInput,
            py::arg("name"), py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getBindInput", &mx::ShaderRef::getBindInput)
        .def("getBindInputs", &mx::ShaderRef::getBindInputs)
        .def("removeBindInput", &mx::ShaderRef::removeBindInput)
        .def("getReferencedOutputs", &mx::ShaderRef::getReferencedOutputs)
        .def_readonly_static("CATEGORY", &mx::ShaderRef::CATEGORY);

    py::class_<mx::Override, mx::OverridePtr, mx::ValueElement>(mod, "Override")
        .def("getReceiver", &mx::Override::getReceiver)
        .def_readonly_static("CATEGORY", &mx::Override::CATEGORY);

    py::class_<mx::MaterialInherit, mx::MaterialInheritPtr, mx::Element>(mod, "MaterialInherit")
        .def_readonly_static("CATEGORY", &mx::MaterialInherit::CATEGORY);
}
