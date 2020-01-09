//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
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
        .def_static("create", &mx::LightHandler::create)
        .def(py::init<>())
        .def("addLightSource", &mx::LightHandler::addLightSource)
        .def("getLightSources", &mx::LightHandler::getLightSources)
        .def("getLightIdentifierMap", &mx::LightHandler::getLightIdentifierMap)
        .def("setLightSources", &mx::LightHandler::setLightSources)
        .def("setEnvRadianceMap", &mx::LightHandler::setEnvRadianceMap)
        .def("getEnvRadianceMap", &mx::LightHandler::getEnvRadianceMap)
        .def("setEnvIrradianceMap", &mx::LightHandler::setEnvIrradianceMap)
        .def("getEnvIrradianceMap", &mx::LightHandler::getEnvIrradianceMap)
        .def("computeLightIdMap", &mx::LightHandler::computeLightIdMap)
        .def("findLights", &mx::LightHandler::findLights)
        .def("registerLights", &mx::LightHandler::registerLights);
}
