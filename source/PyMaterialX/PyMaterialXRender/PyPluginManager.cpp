//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/PluginManager.h>
#include <MaterialXRender/DocumentHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyPluginManager(py::module& mod)
{
    // Bind PluginManager singleton methods as module functions
    mod.def("registerDocumentLoader", [](std::shared_ptr<mx::DocumentLoader> loader) -> bool {
        return mx::PluginManager::getInstance().registerDocumentLoader(loader);
    }, py::arg("loader"));
    
    mod.def("unregisterDocumentLoader", [](const std::string& identifier) -> bool {
        return mx::PluginManager::getInstance().unregisterDocumentLoader(identifier);
    }, py::arg("identifier"));
    
    mod.def("importDocument", [](const std::string& uri) {
        return mx::PluginManager::getInstance().importDocument(uri);
    }, py::arg("uri"));
    
    mod.def("exportDocument", [](mx::ConstDocumentPtr document, const std::string& uri) {
        return mx::PluginManager::getInstance().exportDocument(document, uri);
    }, py::arg("document"), py::arg("uri"));
    
    mod.def("addRegistrationCallback", [](const std::string& identifier, py::function callback) -> bool {
        return mx::PluginManager::getInstance().addRegistrationCallback(identifier, [callback](const std::string& id, bool registered) {
            callback(id, registered);
        });
    }, py::arg("identifier"), py::arg("callback"));
    
    mod.def("removeRegistrationCallback", [](const std::string& identifier) -> bool {
        return mx::PluginManager::getInstance().removeRegistrationCallback(identifier);
    }, py::arg("identifier"));
}
