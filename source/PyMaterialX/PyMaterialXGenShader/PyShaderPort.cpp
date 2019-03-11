//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderNode.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderPort(py::module& mod)
{
    py::class_<mx::ShaderPort, mx::ShaderPortPtr>(mod, "ShaderPort")
        .def("setType", &mx::ShaderPort::setType)
        .def("getType", &mx::ShaderPort::getType)
        .def("setName", &mx::ShaderPort::setName)
        .def("getName", &mx::ShaderPort::getName)
        .def("setSemantic", &mx::ShaderPort::setSemantic)
        .def("getSemantic", &mx::ShaderPort::getSemantic)
        .def("setValue", &mx::ShaderPort::setValue)
        .def("getValue", &mx::ShaderPort::getValue)
        .def("setPath", &mx::ShaderPort::setPath)
        .def("getPath", &mx::ShaderPort::getPath);
}
