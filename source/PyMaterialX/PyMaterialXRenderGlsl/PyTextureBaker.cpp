//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/TextureBaker.h>
#include <MaterialXCore/Material.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTextureBaker(py::module& mod)
{
    py::class_<mx::TextureBakerGlsl, mx::GlslRenderer, mx::TextureBakerPtr>(mod, "TextureBaker", "A helper class for baking procedural material content to textures.\n\nTODO: Add support for graphs containing geometric nodes such as position and normal.")
        .def_static("create", &mx::TextureBakerGlsl::create, "")
        .def("setExtension", &mx::TextureBakerGlsl::setExtension, "Set the file extension for baked textures.")
        .def("getExtension", &mx::TextureBakerGlsl::getExtension, "Return the file extension for baked textures.")
        .def("setColorSpace", &mx::TextureBakerGlsl::setColorSpace, "Set the color space in which color textures are encoded.")
        .def("getColorSpace", &mx::TextureBakerGlsl::getColorSpace, "Return the color space in which color textures are encoded.")
        .def("setDistanceUnit", &mx::TextureBakerGlsl::setDistanceUnit, "Set the distance unit to which textures are baked. Defaults to meters.")
        .def("getDistanceUnit", &mx::TextureBakerGlsl::getDistanceUnit, "Return the distance unit to which textures are baked.")
        .def("setAverageImages", &mx::TextureBakerGlsl::setAverageImages, "Set whether images should be averaged to generate constants. Defaults to false.")
        .def("getAverageImages", &mx::TextureBakerGlsl::getAverageImages, "Return whether images should be averaged to generate constants.")
        .def("setOptimizeConstants", &mx::TextureBakerGlsl::setOptimizeConstants, "Set whether uniform textures should be stored as constants. Defaults to true.")
        .def("getOptimizeConstants", &mx::TextureBakerGlsl::getOptimizeConstants, "Return whether uniform textures should be stored as constants.")
        .def("setOutputImagePath", &mx::TextureBakerGlsl::setOutputImagePath, "Set the output location for baked texture images.\n\nDefaults to the root folder of the destination material.")
        .def("getOutputImagePath", &mx::TextureBakerGlsl::getOutputImagePath, "Get the current output location for baked texture images.")
        .def("setBakedGraphName", &mx::TextureBakerGlsl::setBakedGraphName, "Set the name of the baked graph element.")
        .def("getBakedGraphName", &mx::TextureBakerGlsl::getBakedGraphName, "Return the name of the baked graph element.")
        .def("setBakedGeomInfoName", &mx::TextureBakerGlsl::setBakedGeomInfoName, "Set the name of the baked geometry info element.")
        .def("getBakedGeomInfoName", &mx::TextureBakerGlsl::getBakedGeomInfoName, "Return the name of the baked geometry info element.")
        .def("setTextureFilenameTemplate", &mx::TextureBakerGlsl::setTextureFilenameTemplate, "Set the texture filename template.")
        .def("getTextureFilenameTemplate", &mx::TextureBakerGlsl::getTextureFilenameTemplate, "Get the texture filename template.")
        .def("setFilenameTemplateVarOverride", &mx::TextureBakerGlsl::setFilenameTemplateVarOverride, "Set texFilenameOverrides if template variable exists.")
        .def("setHashImageNames", &mx::TextureBakerGlsl::setHashImageNames, "Set whether to create a short name for baked images by hashing the baked image filenames This is useful for file systems which may have a maximum limit on filename size.\n\nBy default names are not hashed.")
        .def("getHashImageNames", &mx::TextureBakerGlsl::getHashImageNames, "Return whether automatic baked texture resolution is set.")
        .def("setTextureSpaceMin", &mx::TextureBakerGlsl::setTextureSpaceMin, "Set the minimum texcoords used in texture baking. Defaults to 0, 0.")
        .def("getTextureSpaceMin", &mx::TextureBakerGlsl::getTextureSpaceMin, "Return the minimum texcoords used in texture baking.")
        .def("setTextureSpaceMax", &mx::TextureBakerGlsl::setTextureSpaceMax, "Set the maximum texcoords used in texture baking. Defaults to 1, 1.")
        .def("getTextureSpaceMax", &mx::TextureBakerGlsl::getTextureSpaceMax, "Return the maximum texcoords used in texture baking.")
        .def("setupUnitSystem", &mx::TextureBakerGlsl::setupUnitSystem, "Set up the unit definitions to be used in baking.")
        .def("bakeMaterialToDoc", &mx::TextureBakerGlsl::bakeMaterialToDoc, "Bake material to document in memory and write baked textures to disk.")
        .def("bakeAllMaterials", &mx::TextureBakerGlsl::bakeAllMaterials, "Bake materials in the given document and write them to disk.\n\nIf multiple documents are written, then the given output filename will be used as a template.")
        .def("writeDocumentPerMaterial", &mx::TextureBakerGlsl::writeDocumentPerMaterial, "Set whether to write a separate document per material when calling bakeAllMaterials.\n\nBy default separate documents are written.");
}
