//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/PluginManager.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <filesystem>
#include <iostream>

namespace py = pybind11;
namespace mx = MaterialX;
namespace fs = std::filesystem;

// Trampoline classes
class PyDocumentLoaderPlugin : public mx::DocumentLoaderPlugin
{
  public:
    using mx::DocumentLoaderPlugin::DocumentLoaderPlugin;
    PyDocumentLoaderPlugin() : mx::DocumentLoaderPlugin() 
    {
        std::cerr << "Created loader" << std::endl;
    }
    // Override the methods to call the Python implementation
    std::string name() const override
    {
        PYBIND11_OVERRIDE_PURE(std::string, mx::DocumentLoaderPlugin, name);
    }    
    mx::DocumentPtr run(const std::string& path) override
    {
        PYBIND11_OVERRIDE_PURE(mx::DocumentPtr, DocumentLoaderPlugin, run, path);
    }
};

class PyDocumentSaverPlugin : public mx::DocumentSaverPlugin
{
  public:
    using DocumentSaverPlugin::DocumentSaverPlugin;
    PyDocumentSaverPlugin() : mx::DocumentSaverPlugin() 
    {
        std::cerr << "Created saver" << std::endl;
    }
    std::string name() const override
    {
        PYBIND11_OVERRIDE_PURE(std::string, mx::DocumentSaverPlugin, name);
    }
    void run(mx::DocumentPtr doc, const std::string& path) override
    {
        PYBIND11_OVERRIDE_PURE(void, mx::DocumentSaverPlugin, run, doc, path);
    }
};

// Plugin registration function
void registerPlugin(mx::PluginPtr plugin)
{
    mx::PluginManager::getInstance().registerPlugin(std::move(plugin));
}

// Unregister plugin function
bool unregisterPlugin(const std::string& identifier)
{
    return mx::PluginManager::getInstance().unregisterPlugin(identifier);
}

void bindPyPluginManager(py::module& mod)
{
    py::class_<mx::Plugin, mx::PluginPtr>(mod, "Plugin")
        .def("name", &mx::Plugin::name);

    py::class_<mx::DocumentLoaderPlugin, mx::Plugin, PyDocumentLoaderPlugin, mx::DocumentLoaderPluginPtr>(mod, "DocumentLoaderPlugin")
        .def(py::init<>())
        .def("name", &mx::DocumentLoaderPlugin::name)
        .def("run", &mx::DocumentLoaderPlugin::run);

    py::class_<mx::DocumentSaverPlugin, mx::Plugin, PyDocumentSaverPlugin, mx::DocumentSaverPluginPtr>(mod, "DocumentSaverPlugin")
        .def(py::init<>())
        .def("name", &mx::DocumentSaverPlugin::name)
        .def("run", &mx::DocumentSaverPlugin::run);

    // Plugin manager class
    py::class_<mx::PluginManager>(mod, "PluginManager")
        .def("registerPlugin", [](mx::PluginManager& pm, mx::PluginPtr p) 
            {
                pm.registerPlugin(p);
            })
        .def("getPluginList", &mx::PluginManager::getPluginList)
        .def("getLoader", [](mx::PluginManager& manager, const std::string& name) 
            {
                return manager.getPlugin<mx::DocumentLoaderPlugin>(name);
            })
        .def("getSaver", [](mx::PluginManager& manager, const std::string& name)
            {
                return manager.getPlugin<mx::DocumentSaverPlugin>(name);
            });
    
    // Global functions for plugin management
    mod.def("getPluginManager", []() -> mx::PluginManager& 
        { 
            return mx::PluginManager::getInstance(); 
        },
        py::return_value_policy::reference);

    mod.def("registerPlugin", &registerPlugin);
    mod.def("unregisterPlugin", &unregisterPlugin);

    /* 
    mod.def("addRegistrationCallback", [](const std::string& identifier, py::function callback) -> bool {
        auto cpp_callback = [callback](const std::string& id, bool registered) {
            try 
            {
                callback(id, registered);
            } 
            catch (const py::error_already_set&) 
            {
                // Ignore Python callback errors
            }
        };
        return mx::PluginManager::getInstance().addRegistrationCallback(identifier, cpp_callback);
    }, "Add a callback for plugin registration events", 
       py::arg("identifier"), py::arg("callback"));
    
    mod.def("removeRegistrationCallback", [](const std::string& identifier) -> bool {
        return mx::PluginManager::getInstance().removeRegistrationCallback(identifier);
    }, "Remove a registration callback", py::arg("identifier"));
    */
}
