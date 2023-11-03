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
    py::class_<mx::TextureBakerGlsl, mx::GlslRenderer, mx::TextureBakerPtr>(mod, "TextureBaker")

        .def_static("create", &mx::TextureBakerGlsl::create,
                    py::arg("width") = 1024,
                    py::arg("height") = 1024,
                    py::arg_v("baseType",
                              mx::Image::BaseType::UINT8,
                              "BaseType.UINT8"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class using the given texture resolution.
)docstring"))

        .def("setExtension", &mx::TextureBakerGlsl::setExtension,
             py::arg("extension"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the file extension for baked textures.
)docstring"))

        .def("getExtension", &mx::TextureBakerGlsl::getExtension,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the file extension for baked textures.
)docstring"))

        .def("setColorSpace", &mx::TextureBakerGlsl::setColorSpace,
             py::arg("colorSpace"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the color space in which color textures are encoded.

    By default, this color space is `"srgb_texture"`, and color inputs are
    automatically transformed to this space by the baker. If another color
    space is set, then the input graph is responsible for transforming colors
    to this space.
)docstring"))

        .def("getColorSpace", &mx::TextureBakerGlsl::getColorSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color space in which color textures are encoded.
)docstring"))

        .def("setDistanceUnit", &mx::TextureBakerGlsl::setDistanceUnit,
             py::arg("unitSpace"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the distance unit to which textures are baked.

    Defaults to `"meter"`.
)docstring"))

        .def("getDistanceUnit", &mx::TextureBakerGlsl::getDistanceUnit,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the distance unit to which textures are baked.
)docstring"))

        .def("setAverageImages", &mx::TextureBakerGlsl::setAverageImages,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set whether images should be averaged to generate constants.

    Defaults to `False`.
)docstring"))

        .def("getAverageImages", &mx::TextureBakerGlsl::getAverageImages,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return whether images should be averaged to generate constants.
)docstring"))

        .def("setOptimizeConstants", &mx::TextureBakerGlsl::setOptimizeConstants,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set whether uniform textures should be stored as constants.

    Defaults to `True`.
)docstring"))

        .def("getOptimizeConstants", &mx::TextureBakerGlsl::getOptimizeConstants,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return whether uniform textures should be stored as constants.
)docstring"))

        .def("setOutputImagePath", &mx::TextureBakerGlsl::setOutputImagePath,
             py::arg("outputImagePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the output location for baked texture images.

    Defaults to the root folder of the destination material.
)docstring"))

        .def("getOutputImagePath", &mx::TextureBakerGlsl::getOutputImagePath,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the current output location for baked texture images.
)docstring"))

        .def("setBakedGraphName", &mx::TextureBakerGlsl::setBakedGraphName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the name of the baked graph element.
)docstring"))

        .def("getBakedGraphName", &mx::TextureBakerGlsl::getBakedGraphName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the baked graph element.
)docstring"))

        .def("setBakedGeomInfoName", &mx::TextureBakerGlsl::setBakedGeomInfoName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the name of the baked geometry info element.
)docstring"))

        .def("getBakedGeomInfoName", &mx::TextureBakerGlsl::getBakedGeomInfoName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the baked geometry info element.
)docstring"))

        .def("setTextureFilenameTemplate", &mx::TextureBakerGlsl::setTextureFilenameTemplate,
             py::arg("filenameTemplate"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the texture filename template.
)docstring"))

        .def("getTextureFilenameTemplate", &mx::TextureBakerGlsl::getTextureFilenameTemplate,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the texture filename template.
)docstring"))

        .def("setFilenameTemplateVarOverride", &mx::TextureBakerGlsl::setFilenameTemplateVarOverride,
             py::arg("key"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set texture template overrides if template variable exists.
)docstring"))

        .def("setHashImageNames", &mx::TextureBakerGlsl::setHashImageNames,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set whether to create a short name for baked images by hashing the baked
    image filenames.

    This is useful for file systems which may have a maximum limit on filename
    size.

    By default names are not hashed.
)docstring"))

        .def("getHashImageNames", &mx::TextureBakerGlsl::getHashImageNames,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return whether hashing of baked image filenames is enabled.
)docstring"))

        .def("setTextureSpaceMin", &mx::TextureBakerGlsl::setTextureSpaceMin,
             py::arg("textureSpaceMin"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the minimum texcoords used in texture baking.

    Defaults to `0, 0`.
)docstring"))

        .def("getTextureSpaceMin", &mx::TextureBakerGlsl::getTextureSpaceMin,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the minimum texcoords used in texture baking.
)docstring"))

        .def("setTextureSpaceMax", &mx::TextureBakerGlsl::setTextureSpaceMax,
             py::arg("textureSpaceMax"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the maximum texcoords used in texture baking.

    Defaults to `1, 1`.
)docstring"))

        .def("getTextureSpaceMax", &mx::TextureBakerGlsl::getTextureSpaceMax,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the maximum texcoords used in texture baking.
)docstring"))

        .def("setupUnitSystem", &mx::TextureBakerGlsl::setupUnitSystem,
             py::arg("unitDefinitions"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set up the unit definitions to be used in baking.
)docstring"))

        .def("bakeMaterialToDoc", &mx::TextureBakerGlsl::bakeMaterialToDoc,
             py::arg("doc"),
             py::arg("searchPath"),
             py::arg("materialPath"), 
             py::arg("udimSet"),
             py::arg("documentName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bake material to document in memory and write baked textures to disk.
)docstring"))

        .def("bakeAllMaterials", &mx::TextureBakerGlsl::bakeAllMaterials,
             py::arg("doc"),
             py::arg("searchPath"),
             py::arg("outputFileName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bake materials in the given document and write them to disk.

    If multiple documents are written, then the given output filename will be
    used as a template.
)docstring"))

        .def("writeDocumentPerMaterial", &mx::TextureBakerGlsl::writeDocumentPerMaterial,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set whether to write a separate document per material when calling
    `bakeAllMaterials()`.

    By default separate documents are written.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing a texture baker based on GLSL shader generation.

    :see: https://materialx.org/docs/api/class_texture_baker_glsl.html
)docstring");
}
