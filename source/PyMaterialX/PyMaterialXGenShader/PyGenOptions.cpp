//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenOptions.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGenOptions(py::module& mod)
{
    py::enum_<mx::ShaderInterfaceType>(mod, "ShaderInterfaceType", R"docstring(
    Enumeration of the type of shader interface to be generated.
)docstring")
        .value("SHADER_INTERFACE_COMPLETE", mx::ShaderInterfaceType::SHADER_INTERFACE_COMPLETE)
        .value("SHADER_INTERFACE_REDUCED", mx::ShaderInterfaceType::SHADER_INTERFACE_REDUCED)
        .export_values();

    py::enum_<mx::HwSpecularEnvironmentMethod>(mod, "HwSpecularEnvironmentMethod", R"docstring(
    Enumeration of the method to use for specular environment lighting.
)docstring")
        .value("SPECULAR_ENVIRONMENT_PREFILTER", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_PREFILTER)
        .value("SPECULAR_ENVIRONMENT_FIS", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_FIS)
        .value("SPECULAR_ENVIRONMENT_NONE", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_NONE)
        .export_values();

    py::class_<mx::GenOptions>(mod, "GenOptions")
        .def_readwrite("shaderInterfaceType", &mx::GenOptions::shaderInterfaceType)
        .def_readwrite("fileTextureVerticalFlip", &mx::GenOptions::fileTextureVerticalFlip)
        .def_readwrite("targetColorSpaceOverride", &mx::GenOptions::targetColorSpaceOverride)
        .def_readwrite("addUpstreamDependencies", &mx::GenOptions::addUpstreamDependencies)
        .def_readwrite("libraryPrefix", &mx::GenOptions::libraryPrefix)        
        .def_readwrite("targetDistanceUnit", &mx::GenOptions::targetDistanceUnit)
        .def_readwrite("hwTransparency", &mx::GenOptions::hwTransparency)
        .def_readwrite("hwSpecularEnvironmentMethod", &mx::GenOptions::hwSpecularEnvironmentMethod)
        .def_readwrite("hwWriteDepthMoments", &mx::GenOptions::hwWriteDepthMoments)
        .def_readwrite("hwShadowMap", &mx::GenOptions::hwShadowMap)
        .def_readwrite("hwMaxActiveLightSources", &mx::GenOptions::hwMaxActiveLightSources)
        .def_readwrite("hwNormalizeUdimTexCoords", &mx::GenOptions::hwNormalizeUdimTexCoords)
        .def_readwrite("hwAmbientOcclusion", &mx::GenOptions::hwAmbientOcclusion)        
        .def_readwrite("hwWriteAlbedoTable", &mx::GenOptions::hwWriteAlbedoTable)
        .def_readwrite("hwImplicitBitangents", &mx::GenOptions::hwImplicitBitangents)
        .def_readwrite("emitColorTransforms", &mx::GenOptions::emitColorTransforms)
        .def(py::init<>())
        .doc() = R"docstring(
    Class holding options to configure shader generation.

    :see: https://materialx.org/docs/api/class_gen_options.html
)docstring";

    auto GenOptions = mod.attr("GenOptions");

    GenOptions.attr("shaderInterfaceType").doc() = R"docstring(
    (`ShaderInterfaceType`)
    Sets the type of shader interface to be generated.
)docstring";

    GenOptions.attr("fileTextureVerticalFlip").doc() = R"docstring(
    (`bool`)
    If `True` the y-component of texture coordinates used for sampling
    file textures will be flipped before sampling. This can be used if
    file textures need to be flipped vertically to match the target's
    texture space convention. By default this option is `False`.
)docstring";

    GenOptions.attr("targetColorSpaceOverride").doc() = R"docstring(
    (`str`)
    An optional override for the target color space.
    Shader fragments will be generated to transform
    input values and textures into this color space.
)docstring";

    GenOptions.attr("addUpstreamDependencies").doc() = R"docstring(
    (`bool`)
    Sets whether to include upstream dependencies
    for the element to generate a shader for.
)docstring";

    GenOptions.attr("libraryPrefix").doc() = R"docstring(
    (`FilePath`)
    The standard library prefix, which will be applied to
    calls to emitLibraryInclude during code generation.
    Defaults to `"libraries"`.
)docstring";

    GenOptions.attr("targetDistanceUnit").doc() = R"docstring(
    (`str`)
    Define the target distance unit.
    Shader fragments will be generated to transform
    input distance values to the given unit.
)docstring";

    GenOptions.attr("hwTransparency").doc() = R"docstring(
    (`bool`)
    Sets if transparency is needed or not for HW shaders.
    If a surface shader has potential of being transparent
    this must be set to true, otherwise no transparency
    code fragments will be generated for the shader and
    the surface will be fully opaque.
)docstring";

    GenOptions.attr("hwSpecularEnvironmentMethod").doc() = R"docstring(
    (`HwSpecularEnvironmentMethod`)
    Sets the method to use for specular environment lighting
    for HW shader targets.
)docstring";

    GenOptions.attr("hwWriteDepthMoments").doc() = R"docstring(
    (`bool`)
    Enables the writing of depth moments for HW shader targets.
    Defaults to `False`.
)docstring";

    GenOptions.attr("hwShadowMap").doc() = R"docstring(
    (`bool`)
    Enables shadow mapping for HW shader targets.
    Defaults to `False`.
)docstring";

    GenOptions.attr("hwMaxActiveLightSources").doc() = R"docstring(
    (`int`)
    Sets the maximum number of light sources that can
    be active at once.
)docstring";

    GenOptions.attr("hwNormalizeUdimTexCoords").doc() = R"docstring(
    (`bool`)
    Sets whether to transform texture coordinates to normalize
    uv space when UDIMs images are bound to an image. Can be
    enabled for when texture atlas generation is performed to
    compress a set of UDIMs into a single normalized image for
    hardware rendering.
)docstring";

    GenOptions.attr("hwAmbientOcclusion").doc() = R"docstring(
    (`bool`)
    Enables ambient occlusion rendering for HW shader targets.
    Defaults to `False`.
)docstring";

    GenOptions.attr("hwWriteAlbedoTable").doc() = R"docstring(
    (`bool`)
    Enables the writing of a directional albedo table.
    Defaults to `False`.
)docstring";

    GenOptions.attr("hwImplicitBitangents").doc() = R"docstring(
    (`bool`)
    Calculate fallback bitangents from existing normals and tangents
    inside the bitangent node.
)docstring";

    GenOptions.attr("emitColorTransforms").doc() = R"docstring(
    (`bool`)
    Enable emitting colorspace transform code if a color management
    system is defined. Defaults to `True`.
)docstring";

    // FIXME(SH): Expose hwDirectionalAlbedoMethod and hwTransmissionRenderMethod?
    // Sets the method to use for directional albedo evaluation
    // for HW shader targets.
    // HwDirectionalAlbedoMethod hwDirectionalAlbedoMethod;
    // Sets the method to use for transmission rendering
    // for HW shader targets.
    // HwTransmissionRenderMethod hwTransmissionRenderMethod;
}
