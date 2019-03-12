//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHwShaderGenerator(py::module& mod)
{
    mod.attr("VERTEX_STAGE") = mx::Stage::VERTEX;

    mod.attr("HW_VERTEX_INPUTS") = mx::HW::VERTEX_INPUTS;
    mod.attr("HW_VERTEX_DATA") = mx::HW::VERTEX_DATA;
    mod.attr("HW_PRIVATE_UNIFORMS") = mx::HW::PRIVATE_UNIFORMS;
    mod.attr("HW_PUBLIC_UNIFORMS") = mx::HW::PUBLIC_UNIFORMS;
    mod.attr("HW_LIGHT_DATA") = mx::HW::LIGHT_DATA;
    mod.attr("HW_PIXEL_OUTPUTS") = mx::HW::PIXEL_OUTPUTS;
    mod.attr("HW_NORMAL_DIR") = mx::HW::NORMAL_DIR;
    mod.attr("HW_LIGHT_DIR") = mx::HW::LIGHT_DIR;
    mod.attr("HW_VIEW_DIR") = mx::HW::VIEW_DIR;
    mod.attr("HW_ATTR_TRANSPARENT") =  mx::HW::ATTR_TRANSPARENT;

    py::class_<mx::HwShaderGenerator, mx::ShaderGenerator, mx::HwShaderGeneratorPtr>(mod, "HwShaderGenerator")
        .def("getNodeClosureContexts", &mx::HwShaderGenerator::getNodeClosureContexts)
        .def("bindLightShader", &mx::HwShaderGenerator::bindLightShader)
        .def("unbindLightShader", &mx::HwShaderGenerator::unbindLightShader)
        .def("unbindLightShaders", &mx::HwShaderGenerator::unbindLightShaders);
}
