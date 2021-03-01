//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenOgsXml/OgsFragment.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;
namespace my = MaterialXMaya;

void bindPyOgsFragment(py::module& mod)
{
    py::class_<my::OgsFragment, std::unique_ptr<my::OgsFragment>>(mod, "OgsFragment")
        .def(py::init<mx::ElementPtr, const mx::FileSearchPath&>())
        .def(py::init<mx::ElementPtr, mx::GenContext&>())
        .def_property_readonly("elementPath", &my::OgsFragment::getElementPath)
        .def_property_readonly("document", &my::OgsFragment::getDocument)
        .def_property_readonly("fragmentSource", &my::OgsFragment::getFragmentSource)
        .def_property_readonly("lightRigSource", &my::OgsFragment::getLightRigSource)
        .def_property_readonly("fragmentName", &my::OgsFragment::getFragmentName)
        .def_property_readonly("lightRigName", &my::OgsFragment::getLightRigName)
        .def_property_readonly("shader", &my::OgsFragment::getShader)
        .def_property_readonly("pathInputMap", &my::OgsFragment::getPathInputMap)
        .def_property_readonly("isElementAShader", &my::OgsFragment::isElementAShader)
        .def_property_readonly("isTransparent", &my::OgsFragment::isTransparent)
        .def("getImageSamplingProperties", &my::OgsFragment::getImageSamplingProperties)
        .def("getMatrix4Name", &my::OgsFragment::getMatrix4Name);
}
