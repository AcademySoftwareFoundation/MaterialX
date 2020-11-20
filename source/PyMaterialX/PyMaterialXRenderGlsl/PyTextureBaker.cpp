//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/TextureBaker.h>
#include <MaterialXCore/Material.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTextureBaker(py::module& mod)
{
    py::class_<mx::TextureBaker, mx::GlslRenderer, mx::TextureBakerPtr>(mod, "TextureBaker")
        .def_static("create", &mx::TextureBaker::create)
        .def("setExtension", &mx::TextureBaker::setExtension)
        .def("getExtension", &mx::TextureBaker::getExtension)
        .def("setColorSpace", &mx::TextureBaker::setColorSpace)
        .def("getColorSpace", &mx::TextureBaker::getColorSpace)
        .def("setupUnitSystem", &mx::TextureBaker::setupUnitSystem)
        .def("setTargetUnitSpace", &mx::TextureBaker::setTargetUnitSpace)
        .def("getTargetUnitSpace", &mx::TextureBaker::getTargetUnitSpace)
        .def("setBakedGraphName", &mx::TextureBaker::setBakedGraphName)
        .def("getBakedGraphName", &mx::TextureBaker::getBakedGraphName)
        .def("setBakedGeomInfoName", &mx::TextureBaker::setBakedGeomInfoName)
        .def("getBakedGeomInfoName", &mx::TextureBaker::getBakedGeomInfoName)
        .def("setOutputResourcePath", &mx::TextureBaker::setOutputResourcePath)
        .def("getOutputResourcePath", &mx::TextureBaker::getOutputResourcePath)
        .def("setOptimizeConstants", &mx::TextureBaker::setOptimizeConstants)
        .def("getOptimizeConstants", &mx::TextureBaker::getOptimizeConstants)
        .def("bakeAllMaterials", &mx::TextureBaker::bakeAllMaterials)
        .def("getBakedMaterial", &mx::TextureBaker::getBakedMaterial);
}
