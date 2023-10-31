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
    py::enum_<mx::ShaderInterfaceType>(mod, "ShaderInterfaceType",
                                       PYMATERIALX_DOCSTRING(R"docstring(
    Enumeration of the type of shader interface to be generated.
)docstring"))

        .value("SHADER_INTERFACE_COMPLETE", mx::ShaderInterfaceType::SHADER_INTERFACE_COMPLETE,
               PYMATERIALX_DOCSTRING(R"docstring(
    Create a complete interface with uniforms for all editable inputs on all
    nodes used by the shader.

    This interface makes the shader fully editable by value without requiring
    any rebuilds.

    This is the default interface type.
)docstring"))

        .value("SHADER_INTERFACE_REDUCED", mx::ShaderInterfaceType::SHADER_INTERFACE_REDUCED,
               PYMATERIALX_DOCSTRING(R"docstring(
    Create a reduced interface with uniforms only for the inputs that has been
    declared in the shaders nodedef interface. If values on other inputs are
    changed the shader needs to be rebuilt.
)docstring"))

        .export_values();

    py::enum_<mx::HwSpecularEnvironmentMethod>(mod, "HwSpecularEnvironmentMethod",
                                               PYMATERIALX_DOCSTRING(R"docstring(
    Enumeration of the method to use for specular environment lighting.
)docstring"))

        .value("SPECULAR_ENVIRONMENT_PREFILTER", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_PREFILTER,
               PYMATERIALX_DOCSTRING(R"docstring(
    Use pre-filtered environment maps for specular environment/indirect lighting.
)docstring"))

        .value("SPECULAR_ENVIRONMENT_FIS", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_FIS,
               PYMATERIALX_DOCSTRING(R"docstring(
    Use Filtered Importance Sampling for specular environment/indirect lighting.
)docstring"))

        .value("SPECULAR_ENVIRONMENT_NONE", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_NONE,
               PYMATERIALX_DOCSTRING(R"docstring(
    Do not use specular environment maps.
)docstring"))

        .export_values();

    py::class_<mx::GenOptions>(mod, "GenOptions")

        .def_readwrite("shaderInterfaceType", &mx::GenOptions::shaderInterfaceType,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`ShaderInterfaceType`)
    Sets the type of shader interface to be generated.
)docstring"))

        .def_readwrite("fileTextureVerticalFlip", &mx::GenOptions::fileTextureVerticalFlip,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    If `True` the y-component of texture coordinates used for sampling
    file textures will be flipped before sampling. This can be used if
    file textures need to be flipped vertically to match the target's
    texture space convention. By default this option is `False`.
)docstring"))

        .def_readwrite("targetColorSpaceOverride", &mx::GenOptions::targetColorSpaceOverride,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    An optional override for the target color space.
    Shader fragments will be generated to transform
    input values and textures into this color space.
)docstring"))

        .def_readwrite("addUpstreamDependencies", &mx::GenOptions::addUpstreamDependencies,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Sets whether to include upstream dependencies
    for the element to generate a shader for.
)docstring"))

        .def_readwrite("libraryPrefix", &mx::GenOptions::libraryPrefix,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`FilePath`)
    The standard library prefix, which will be applied to
    calls to emitLibraryInclude during code generation.
    Defaults to `"libraries"`.
)docstring"))

        .def_readwrite("targetDistanceUnit", &mx::GenOptions::targetDistanceUnit,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Define the target distance unit.
    Shader fragments will be generated to transform
    input distance values to the given unit.
)docstring"))

        .def_readwrite("hwTransparency", &mx::GenOptions::hwTransparency,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Sets if transparency is needed or not for HW shaders.
    If a surface shader has potential of being transparent
    this must be set to true, otherwise no transparency
    code fragments will be generated for the shader and
    the surface will be fully opaque.
)docstring"))

        .def_readwrite("hwSpecularEnvironmentMethod", &mx::GenOptions::hwSpecularEnvironmentMethod,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`HwSpecularEnvironmentMethod`)
    Sets the method to use for specular environment lighting
    for HW shader targets.
)docstring"))

        .def_readwrite("hwWriteDepthMoments", &mx::GenOptions::hwWriteDepthMoments,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Enables the writing of depth moments for HW shader targets.
    Defaults to `False`.
)docstring"))

        .def_readwrite("hwShadowMap", &mx::GenOptions::hwShadowMap,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Enables shadow mapping for HW shader targets.
    Defaults to `False`.
)docstring"))

        .def_readwrite("hwMaxActiveLightSources", &mx::GenOptions::hwMaxActiveLightSources,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    Sets the maximum number of light sources that can
    be active at once.
)docstring"))

        .def_readwrite("hwNormalizeUdimTexCoords", &mx::GenOptions::hwNormalizeUdimTexCoords,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Sets whether to transform texture coordinates to normalize
    uv space when UDIMs images are bound to an image. Can be
    enabled for when texture atlas generation is performed to
    compress a set of UDIMs into a single normalized image for
    hardware rendering.
)docstring"))

        .def_readwrite("hwAmbientOcclusion", &mx::GenOptions::hwAmbientOcclusion,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Enables ambient occlusion rendering for HW shader targets.
    Defaults to `False`.
)docstring"))

        .def_readwrite("hwWriteAlbedoTable", &mx::GenOptions::hwWriteAlbedoTable,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Enables the writing of a directional albedo table.
    Defaults to `False`.
)docstring"))

        .def_readwrite("hwImplicitBitangents", &mx::GenOptions::hwImplicitBitangents,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Calculate fallback bitangents from existing normals and tangents
    inside the bitangent node.
)docstring"))

        .def_readwrite("emitColorTransforms", &mx::GenOptions::emitColorTransforms,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Enable emitting colorspace transform code if a color management
    system is defined. Defaults to `True`.
)docstring"))

        .def(py::init<>(), PYMATERIALX_DOCSTRING(R"docstring(
    Initializes an instance of this class.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class holding options to configure shader generation.

    :see: https://materialx.org/docs/api/class_gen_options.html
)docstring");

    // FIXME(SH): Expose hwDirectionalAlbedoMethod and hwTransmissionRenderMethod?
    // Sets the method to use for directional albedo evaluation
    // for HW shader targets.
    // HwDirectionalAlbedoMethod hwDirectionalAlbedoMethod;
    // Sets the method to use for transmission rendering
    // for HW shader targets.
    // HwTransmissionRenderMethod hwTransmissionRenderMethod;
}
