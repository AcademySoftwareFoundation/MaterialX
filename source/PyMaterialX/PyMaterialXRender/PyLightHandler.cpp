//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXRender/LightHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyLightHandler(py::module& mod)
{
    py::class_<mx::LightHandler, mx::LightHandlerPtr>(mod, "LightHandler")

        .def_static("create", &mx::LightHandler::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("setLightTransform", &mx::LightHandler::setLightTransform,
             py::arg("mat"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the light transform.
)docstring"))

        .def("getLightTransform", &mx::LightHandler::getLightTransform,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the light transform.
)docstring"))

        .def("setDirectLighting", &mx::LightHandler::setDirectLighting,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set whether direct lighting is enabled.
)docstring"))

        .def("getDirectLighting", &mx::LightHandler::getDirectLighting,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return whether direct lighting is enabled.
)docstring"))

        .def("setIndirectLighting", &mx::LightHandler::setIndirectLighting,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set whether indirect lighting is enabled.
)docstring"))

        .def("getIndirectLighting", &mx::LightHandler::getIndirectLighting,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return whether indirect lighting is enabled.
)docstring"))

        .def("setEnvRadianceMap", &mx::LightHandler::setEnvRadianceMap,
             py::arg("envRadianceMap"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the environment radiance map.
)docstring"))

        .def("getEnvRadianceMap", &mx::LightHandler::getEnvRadianceMap,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the environment radiance map.
)docstring"))

        .def("setEnvIrradianceMap", &mx::LightHandler::setEnvIrradianceMap,
             py::arg("envIrradianceMap"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the environment irradiance map.
)docstring"))

        .def("getEnvIrradianceMap", &mx::LightHandler::getEnvIrradianceMap,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the environment irradiance map.
)docstring"))

        .def("setAlbedoTable", &mx::LightHandler::setAlbedoTable,
             py::arg("table"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the directional albedo table.
)docstring"))

        .def("getAlbedoTable", &mx::LightHandler::getAlbedoTable,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the directional albedo table.
)docstring"))

        .def("setEnvSampleCount", &mx::LightHandler::setEnvSampleCount,
             py::arg("count"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the environment lighting sample count.
)docstring"))

        .def("getEnvSampleCount", &mx::LightHandler::getEnvSampleCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the environment lighting sample count.
)docstring"))

        .def("setRefractionTwoSided", &mx::LightHandler::setRefractionTwoSided,
             py::arg("enable"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the two-sided refraction property.
)docstring"))

        .def("getRefractionTwoSided", &mx::LightHandler::getRefractionTwoSided,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the two-sided refraction property.
)docstring"))

        .def("addLightSource", &mx::LightHandler::addLightSource,
             py::arg("node"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a light source.
)docstring"))

        .def("setLightSources", &mx::LightHandler::setLightSources,
             py::arg("lights"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the list of light sources.
)docstring"))

        .def("getLightSources", &mx::LightHandler::getLightSources,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the list of light sources.
)docstring"))

        .def("getFirstLightOfCategory", &mx::LightHandler::getFirstLightOfCategory,
             py::arg("category"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first light source, if any, of the given `category`.
)docstring"))

        .def("getLightIdMap", &mx::LightHandler::getLightIdMap,
             PYMATERIALX_DOCSTRING(R"docstring(
    Get a list of identifiers associated with a given light nodedef.
)docstring"))

        .def("computeLightIdMap", &mx::LightHandler::computeLightIdMap,
             py::arg("nodes"),
             PYMATERIALX_DOCSTRING(R"docstring(
    From a set of nodes, create a mapping of corresponding nodedef identifiers
    to numbers.
)docstring"))

        .def("findLights", &mx::LightHandler::findLights,
             py::arg("doc"),
             py::arg("lights"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Find lights to use based on an input document.

    :param doc: Document to scan for lights.
    :type doc: Document
    :param lights: List of lights found in document.
    :type lights: List[Node]
)docstring"))

        .def("registerLights", &mx::LightHandler::registerLights,
             py::arg("doc"),
             py::arg("lights"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Register light node definitions and light count with a given generation
    context.

    :param doc: Document containing light nodes and definitions.
    :type doc: Document
    :param lights: Lights to register.
    :type lights: List[Node]
    :param context: Context to update.
    :type context: GenContext
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a utility light handler for creating and providing
    light data for shader binding.

    :see: https://materialx.org/docs/api/class_light_handler.html
)docstring");
}
