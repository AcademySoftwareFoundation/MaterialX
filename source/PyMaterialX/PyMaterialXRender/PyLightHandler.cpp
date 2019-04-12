//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXRender/Handlers/LightHandler.h>

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
        .def("setLightEnvIrradiancePath", &mx::LightHandler::setLightEnvIrradiancePath)
        .def("getLightEnvIrradiancePath", &mx::LightHandler::getLightEnvIrradiancePath)
        .def("setLightEnvRadiancePath", &mx::LightHandler::setLightEnvRadiancePath)
        .def("getLightEnvRadiancePath", &mx::LightHandler::getLightEnvRadiancePath)
        .def("mapNodeDefToIdentiers", &mx::LightHandler::mapNodeDefToIdentiers)
        .def("findLights", &mx::LightHandler::findLights)
        .def("registerLights", &mx::LightHandler::registerLights);
}
