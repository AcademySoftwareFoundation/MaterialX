//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslMaterial.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslMaterial(py::module& mod)
{
    py::class_<mx::HlslMaterial, mx::HlslMaterialPtr> hlslMat(mod, "HlslMaterial");

    py::enum_<mx::HlslMaterial::Stage>(hlslMat, "Stage")
        .value("Vertex", mx::HlslMaterial::Stage::Vertex)
        .value("Pixel",  mx::HlslMaterial::Stage::Pixel);

    hlslMat.def_static("create", &mx::HlslMaterial::create)
        .def("getPixelBindings",     &mx::HlslMaterial::getPixelBindings)
        .def("lookupVariableOffset", &mx::HlslMaterial::lookupVariableOffset)
        // Cbuffer writes accept Python `bytes`. We marshal through a
        // small lambda because pybind11 doesn't bind raw void* + size.
        .def("setCbufferDataByName",
             [](mx::HlslMaterial& self, mx::HlslMaterial::Stage stage,
                const std::string& name, py::bytes data) {
                 const std::string s = data;
                 return self.setCbufferDataByName(stage, name, s.data(), s.size());
             })
        .def("setCbufferDataBySlot",
             [](mx::HlslMaterial& self, mx::HlslMaterial::Stage stage,
                unsigned int slot, py::bytes data) {
                 const std::string s = data;
                 return self.setCbufferDataBySlot(stage, slot, s.data(), s.size());
             })
        .def("setCbufferRange",
             [](mx::HlslMaterial& self, mx::HlslMaterial::Stage stage,
                const std::string& name, std::size_t offset, py::bytes data) {
                 const std::string s = data;
                 return self.setCbufferRange(stage, name, offset, s.data(), s.size());
             });
}
