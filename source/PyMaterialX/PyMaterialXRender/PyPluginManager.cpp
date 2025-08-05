//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/PluginManager.h>

namespace py = pybind11;
namespace mx = MaterialX;

// Python wrapper classes for plugins
class PyImportPlugin : public mx::ImportPlugin
{
  public:
    using mx::ImportPlugin::ImportPlugin;

    mx::DocumentPtr importDocument(const std::string& filename, 
                                 const std::map<std::string, std::string>& options = {}) override
    {
        PYBIND11_OVERRIDE_PURE(mx::DocumentPtr, mx::ImportPlugin, importDocument, filename, options);
    }

    bool canImport(const std::string& filename) const override
    {
        PYBIND11_OVERRIDE(bool, mx::ImportPlugin, canImport, filename);
    }
};

class PyExportPlugin : public mx::ExportPlugin
{
  public:
    using mx::ExportPlugin::ExportPlugin;

    bool exportDocument(mx::ConstDocumentPtr document, const std::string& filename,
                       const std::map<std::string, std::string>& options = {}) override
    {
        PYBIND11_OVERRIDE_PURE(bool, mx::ExportPlugin, exportDocument, document, filename, options);
    }

    bool canExport(const std::string& filename) const override
    {
        PYBIND11_OVERRIDE(bool, mx::ExportPlugin, canExport, filename);
    }
};

void bindPyPluginManager(py::module& mod)
{
    py::enum_<mx::PluginType>(mod, "PluginType")
        .value("IMPORT", mx::PluginType::IMPORT)
        .value("EXPORT", mx::PluginType::EXPORT);

    py::class_<mx::PluginInfo>(mod, "PluginInfo")
        .def(py::init<>())
        .def_readwrite("identifier", &mx::PluginInfo::identifier)
        .def_readwrite("name", &mx::PluginInfo::name)
        .def_readwrite("description", &mx::PluginInfo::description)
        .def_readwrite("supportedExtensions", &mx::PluginInfo::supportedExtensions)
        .def_readwrite("type", &mx::PluginInfo::type)
        .def_readwrite("version", &mx::PluginInfo::version);

    py::class_<mx::Plugin, std::shared_ptr<mx::Plugin>>(mod, "Plugin")
        .def("getInfo", &mx::Plugin::getInfo, py::return_value_policy::reference)
        .def("getIdentifier", &mx::Plugin::getIdentifier)
        .def("getType", &mx::Plugin::getType)
        .def("supportsExtension", &mx::Plugin::supportsExtension);

    py::class_<mx::ImportPlugin, PyImportPlugin, mx::Plugin, std::shared_ptr<mx::ImportPlugin>>(mod, "ImportPlugin")
        .def(py::init<const std::string&, const std::string&, const std::string&, 
                     const std::vector<std::string>&, const std::string&>(),
             py::arg("identifier"), py::arg("name"), py::arg("description"), 
             py::arg("extensions"), py::arg("version") = "1.0.0")
        .def("importDocument", &mx::ImportPlugin::importDocument,
             py::arg("filename"), py::arg("options") = std::map<std::string, std::string>())
        .def("canImport", &mx::ImportPlugin::canImport);

    py::class_<mx::ExportPlugin, PyExportPlugin, mx::Plugin, std::shared_ptr<mx::ExportPlugin>>(mod, "ExportPlugin")
        .def(py::init<const std::string&, const std::string&, const std::string&, 
                     const std::vector<std::string>&, const std::string&>(),
             py::arg("identifier"), py::arg("name"), py::arg("description"), 
             py::arg("extensions"), py::arg("version") = "1.0.0")
        .def("exportDocument", &mx::ExportPlugin::exportDocument,
             py::arg("document"), py::arg("filename"), py::arg("options") = std::map<std::string, std::string>())
        .def("canExport", &mx::ExportPlugin::canExport);

    py::class_<mx::PluginManager>(mod, "PluginManager")
        .def_static("getInstance", &mx::PluginManager::getInstance, 
                   py::return_value_policy::reference)
        .def("registerImportPlugin", &mx::PluginManager::registerImportPlugin)
        .def("registerExportPlugin", &mx::PluginManager::registerExportPlugin)
        .def("unregisterPlugin", &mx::PluginManager::unregisterPlugin)
        .def("getImportPlugins", &mx::PluginManager::getImportPlugins,
             py::return_value_policy::reference)
        .def("getExportPlugins", &mx::PluginManager::getExportPlugins,
             py::return_value_policy::reference)
        .def("getImportPluginsForExtension", &mx::PluginManager::getImportPluginsForExtension)
        .def("getExportPluginsForExtension", &mx::PluginManager::getExportPluginsForExtension)
        .def("getPlugin", &mx::PluginManager::getPlugin)
        .def("getSupportedImportExtensions", &mx::PluginManager::getSupportedImportExtensions)
        .def("getSupportedExportExtensions", &mx::PluginManager::getSupportedExportExtensions)
        .def("importDocument", &mx::PluginManager::importDocument,
             py::arg("filename"), py::arg("pluginId") = "", 
             py::arg("options") = std::map<std::string, std::string>())
        .def("exportDocument", &mx::PluginManager::exportDocument,
             py::arg("document"), py::arg("filename"), py::arg("pluginId") = "",
             py::arg("options") = std::map<std::string, std::string>())
        .def("clearPlugins", &mx::PluginManager::clearPlugins)
        .def("getAllPluginInfo", &mx::PluginManager::getAllPluginInfo)
        .def("setPluginRegistrationCallback", [](mx::PluginManager& self, py::function callback) {
            self.setPluginRegistrationCallback([callback](const std::string& id, bool registered) {
                callback(id, registered);
            });
        });
}
