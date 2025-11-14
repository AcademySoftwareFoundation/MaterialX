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
    py::class_<mx::LightHandler, mx::LightHandlerPtr>(mod, "LightHandler", "Utility light handler for creating and providing light data for shader binding.")
        .def_static("create", &mx::LightHandler::create)
        .def(py::init<>())
        .def("setLightTransform", &mx::LightHandler::setLightTransform, "Set the light transform.")
        .def("getLightTransform", &mx::LightHandler::getLightTransform, "Return the light transform.")
        .def("setDirectLighting", &mx::LightHandler::setDirectLighting, "Set whether direct lighting is enabled.")
        .def("getDirectLighting", &mx::LightHandler::getDirectLighting, "Return whether direct lighting is enabled.")
        .def("setIndirectLighting", &mx::LightHandler::setIndirectLighting, "Set whether indirect lighting is enabled.")
        .def("getIndirectLighting", &mx::LightHandler::getIndirectLighting, "Return whether indirect lighting is enabled.")
        .def("setEnvRadianceMap", &mx::LightHandler::setEnvRadianceMap, "Set the environment radiance map.")
        .def("getEnvRadianceMap", &mx::LightHandler::getEnvRadianceMap, "Return the environment radiance map.")
        .def("setEnvIrradianceMap", &mx::LightHandler::setEnvIrradianceMap, "Set the environment irradiance map.")
        .def("getEnvIrradianceMap", &mx::LightHandler::getEnvIrradianceMap, "Return the environment irradiance map.")
        .def("setAlbedoTable", &mx::LightHandler::setAlbedoTable, "Set the directional albedo table.")
        .def("getAlbedoTable", &mx::LightHandler::getAlbedoTable, "Return the directional albedo table.")
        .def("setEnvSampleCount", &mx::LightHandler::setEnvSampleCount, "Set the environment lighting sample count.")
        .def("getEnvSampleCount", &mx::LightHandler::getEnvSampleCount, "Return the environment lighting sample count.")
        .def("setRefractionTwoSided", &mx::LightHandler::setRefractionTwoSided, "Set the two-sided refraction property.")
        .def("getRefractionTwoSided", &mx::LightHandler::getRefractionTwoSided, "Return the two-sided refraction property.")        
        .def("addLightSource", &mx::LightHandler::addLightSource, "Add a light source.")
        .def("setLightSources", &mx::LightHandler::setLightSources, "Set the vector of light sources.")
        .def("getLightSources", &mx::LightHandler::getLightSources, "Return the vector of light sources.")
        .def("getFirstLightOfCategory", &mx::LightHandler::getFirstLightOfCategory, "Return the first light source, if any, of the given category.")
        .def("getLightIdMap", &mx::LightHandler::getLightIdMap, "Get a list of identifiers associated with a given light nodedef.")
        .def("computeLightIdMap", &mx::LightHandler::computeLightIdMap, "From a set of nodes, create a mapping of corresponding nodedef identifiers to numbers.")
        .def("findLights", &mx::LightHandler::findLights, "Find lights to use based on an input document.\n\nArgs:\n    doc: Document to scan for lights\n    lights: List of lights found in document")
        .def("registerLights", &mx::LightHandler::registerLights, "Register light node definitions and light count with a given generation context.\n\nArgs:\n    doc: Document containing light nodes and definitions\n    lights: Lights to register\n    context: Context to update");
}
