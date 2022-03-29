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
        .def("setDistanceUnit", &mx::TextureBaker::setDistanceUnit)
        .def("getDistanceUnit", &mx::TextureBaker::getDistanceUnit)
        .def("setAverageImages", &mx::TextureBaker::setAverageImages)
        .def("getAverageImages", &mx::TextureBaker::getAverageImages)
        .def("setOptimizeConstants", &mx::TextureBaker::setOptimizeConstants)
        .def("getOptimizeConstants", &mx::TextureBaker::getOptimizeConstants)
        .def("setOutputImagePath", &mx::TextureBaker::setOutputImagePath)
        .def("getOutputImagePath", &mx::TextureBaker::getOutputImagePath)
        .def("setBakedGraphName", &mx::TextureBaker::setBakedGraphName)
        .def("getBakedGraphName", &mx::TextureBaker::getBakedGraphName)
        .def("setBakedGeomInfoName", &mx::TextureBaker::setBakedGeomInfoName)
        .def("getBakedGeomInfoName", &mx::TextureBaker::getBakedGeomInfoName)
        .def("setTextureFilenameTemplate", &mx::TextureBaker::setTextureFilenameTemplate)
        .def("getTextureFilenameTemplate", &mx::TextureBaker::getTextureFilenameTemplate)
        .def("setFilenameTemplateVarOverride", &mx::TextureBaker::setFilenameTemplateVarOverride)
        .def("setHashImageNames", &mx::TextureBaker::setHashImageNames)
        .def("getHashImageNames", &mx::TextureBaker::getHashImageNames)
        .def("setTextureSpaceMin", &mx::TextureBaker::setTextureSpaceMin)
        .def("getTextureSpaceMin", &mx::TextureBaker::getTextureSpaceMin)
        .def("setTextureSpaceMax", &mx::TextureBaker::setTextureSpaceMax)
        .def("getTextureSpaceMax", &mx::TextureBaker::getTextureSpaceMax)
        .def("setupUnitSystem", &mx::TextureBaker::setupUnitSystem)
        .def("bakeMaterialToDoc", &mx::TextureBaker::bakeMaterialToDoc)
        .def("bakeAllMaterials", &mx::TextureBaker::bakeAllMaterials);
}
