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
                                       "Type of shader interface to be generated")
        .value("SHADER_INTERFACE_COMPLETE", mx::ShaderInterfaceType::SHADER_INTERFACE_COMPLETE,
               "Create a complete interface with uniforms for all "
               "editable inputs on all nodes used by the shader. "
               "This interface makes the shader fully editable by "
               "value without requiring any rebuilds. "
               "This is the default interface type.")
        .value("SHADER_INTERFACE_REDUCED", mx::ShaderInterfaceType::SHADER_INTERFACE_REDUCED,
               "Create a reduced interface with uniforms only for "
               "the inputs that has been declared in the shaders "
               "nodedef interface. If values on other inputs are "
               "changed the shader needs to be rebuilt.")
        .export_values();

    py::enum_<mx::HwSpecularEnvironmentMethod>(mod, "HwSpecularEnvironmentMethod",
                                               "Method to use for specular environment lighting")
        .value("SPECULAR_ENVIRONMENT_PREFILTER", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_PREFILTER,
               "Use pre-filtered environment maps for specular environment/indirect lighting.")
        .value("SPECULAR_ENVIRONMENT_FIS", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_FIS,
               "Use Filtered Importance Sampling for specular environment/indirect lighting.")
        .value("SPECULAR_ENVIRONMENT_NONE", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_NONE,
               "Do not use specular environment maps.")
        .export_values();

    py::class_<mx::GenOptions>(mod, "GenOptions")
        .def_readwrite("shaderInterfaceType", &mx::GenOptions::shaderInterfaceType,
                       "(`ShaderInterfaceType`)\n"
                       "Sets the type of shader interface to be generated.")
        .def_readwrite("fileTextureVerticalFlip", &mx::GenOptions::fileTextureVerticalFlip,
                       "(`bool`)\n"
                       "If `True` the y-component of texture coordinates used for sampling\n"
                       "file textures will be flipped before sampling. This can be used if\n"
                       "file textures need to be flipped vertically to match the target's\n"
                       "texture space convention. By default this option is `False`.")
        .def_readwrite("targetColorSpaceOverride", &mx::GenOptions::targetColorSpaceOverride,
                       "(`str`)\n"
                       "An optional override for the target color space.\n"
                       "Shader fragments will be generated to transform\n"
                       "input values and textures into this color space.")
        .def_readwrite("targetDistanceUnit", &mx::GenOptions::targetDistanceUnit,
                       "(`str`)\n"
                       "Define the target distance unit.\n"
                       "Shader fragments will be generated to transform\n"
                       "input distance values to the given unit.")
        .def_readwrite("addUpstreamDependencies", &mx::GenOptions::addUpstreamDependencies,
                       "(`bool`)\n"
                       "Sets whether to include upstream dependencies\n"
                       "for the element to generate a shader for.")
        .def_readwrite("libraryPrefix", &mx::GenOptions::libraryPrefix,
                       "(`str`)\n"
                       "The standard library prefix, which will be applied to\n"
                       "calls to `emitLibraryInclude()` during code generation.\n"
                       "Defaults to `'libraries'`.")
        .def_readwrite("emitColorTransforms", &mx::GenOptions::emitColorTransforms,
                       "(`bool`)\n"
                       "Enable emitting colorspace transform code if a color management\n"
                       "system is defined. Defaults to `True`.")
        .def_readwrite("hwTransparency", &mx::GenOptions::hwTransparency,
                       "(`bool`)\n"
                       "Sets if transparency is needed or not for HW shaders.\n"
                       "If a surface shader has potential of being transparent\n"
                       "this must be set to `True`, otherwise no transparency\n"
                       "code fragments will be generated for the shader and\n"
                       "the surface will be fully opaque.")
        .def_readwrite("hwSpecularEnvironmentMethod", &mx::GenOptions::hwSpecularEnvironmentMethod,
                       "(`HwSpecularEnvironmentMethod`)\n"
                       "Sets the method to use for specular environment lighting\n"
                       "for HW shader targets.")
        .def_readwrite("hwSrgbEncodeOutput", &mx::GenOptions::hwSrgbEncodeOutput,
                       "(`bool`)\n"
                       "Enables an sRGB encoding for the color output on HW shader targets.\n"
                       "Defaults to `False`.")
        .def_readwrite("hwWriteDepthMoments", &mx::GenOptions::hwWriteDepthMoments,
                       "(`bool`)\n"
                       "Enables the writing of depth moments for HW shader targets.\n"
                       "Defaults to `False`.")
        .def_readwrite("hwShadowMap", &mx::GenOptions::hwShadowMap,
                       "(`bool`)\n"
                       "Enables shadow mapping for HW shader targets.\n"
                       "Defaults to `False`.")
        .def_readwrite("hwMaxActiveLightSources", &mx::GenOptions::hwMaxActiveLightSources,
                       "(`int`)\n"
                       "Sets the maximum number of light sources that can\n"
                       "be active at once.")
        .def_readwrite("hwNormalizeUdimTexCoords", &mx::GenOptions::hwNormalizeUdimTexCoords,
                       "(`bool`)\n"
                       "Sets whether to transform texture coordinates to normalize\n"
                       "uv space when UDIMs images are bound to an image. Can be\n"
                       "enabled for when texture atlas generation is performed to\n"
                       "compress a set of UDIMs into a single normalized image for\n"
                       "hardware rendering.")
        .def_readwrite("hwAmbientOcclusion", &mx::GenOptions::hwAmbientOcclusion,
                       "(`bool`)\n"
                       "Enables ambient occlusion rendering for HW shader targets.\n"
                       "Defaults to `False`.")
        .def_readwrite("hwWriteAlbedoTable", &mx::GenOptions::hwWriteAlbedoTable,
                       "(`bool`)\n"
                       "Enables the writing of a directional albedo table.\n"
                       "Defaults to `False`.")
        .def_readwrite("hwWriteEnvPrefilter", &mx::GenOptions::hwWriteEnvPrefilter,
                       "(`bool`)\n"
                       "Enables the generation of a prefiltered environment map.\n"
                       "Defaults to `False`.")
        .def_readwrite("hwImplicitBitangents", &mx::GenOptions::hwImplicitBitangents,
                       "(`bool`)\n"
                       "Calculate fallback bitangents from existing normals and tangents\n"
                       "inside the bitangent node.")
        .def(py::init<>());
    mod.attr("GenOptions").doc() = R"docstring(
    Class holding options to configure shader generation.

    :see: https://materialx.org/docs/api/class_gen_options.html)docstring";
}
