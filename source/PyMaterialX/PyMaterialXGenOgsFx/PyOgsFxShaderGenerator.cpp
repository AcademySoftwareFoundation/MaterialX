//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyOgsFxShaderGenerator(py::module& mod)
{
    mod.attr("FX_STAGE") = mx::HW::FX_STAGE;

    py::class_<mx::OgsFxShaderGenerator, mx::ShaderGenerator, mx::OgsFxShaderGeneratorPtr>(mod, "OgsFxShaderGenerator")
        .def_static("create", &mx::OgsFxShaderGenerator::create)
        .def(py::init<>())
        .def("getTarget", &mx::OgsFxShaderGenerator::getTarget);
        .def("generate", &mx::OgsFxShaderGenerator::generate)
}
