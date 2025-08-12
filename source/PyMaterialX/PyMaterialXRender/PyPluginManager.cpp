//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/PluginManager.h>
//#include <MaterialXRender/DocumentHandler.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <filesystem>
#include <iostream>

namespace py = pybind11;
namespace mx = MaterialX;
namespace fs = std::filesystem;

// Trampoline classes
// Updated trampoline classes with non-pure virtual handling
class PyPlugin : public mx::IPlugin {
public:
    using mx::IPlugin::IPlugin;

    // Non-pure virtuals use PYBIND11_OVERRIDE (without _PURE)
    const std::string& getPluginType() override {
        PYBIND11_OVERRIDE(const std::string&, mx::IPlugin, getPluginType);
    }

    const std::string& getIdentifier() const override {
        PYBIND11_OVERRIDE(const std::string&, mx::IPlugin, getIdentifier);
    }
};

class PyDocumentPlugin : public mx::IDocumentPlugin {
public:
    using mx::IDocumentPlugin::IDocumentPlugin;

    // Implement base IPlugin methods
    const std::string& getPluginType() override {
        PYBIND11_OVERRIDE(const std::string&, mx::IDocumentPlugin, getPluginType);
    }

    const std::string& getIdentifier() const override {
        PYBIND11_OVERRIDE(const std::string&, mx::IDocumentPlugin, getIdentifier);
    }

    // Document methods with default implementations
    mx::DocumentPtr load(const std::string& path) override {
        PYBIND11_OVERRIDE(mx::DocumentPtr, mx::IDocumentPlugin, load, path);
    }

    bool save(mx::ConstDocumentPtr document, const std::string& path) override {
        PYBIND11_OVERRIDE(bool, mx::IDocumentPlugin, save, document, path);
    }
};

// Plugin registration function
void registerPlugin(py::object py_plugin)
{
    try 
    {
        mx::IPluginPtr plugin_ptr = py_plugin.cast<mx::IPluginPtr>();
        mx::PluginManager::getInstance().registerPlugin(plugin_ptr);
    }
    catch (const std::exception&) {
        // Handle registration error
    }
}

// Unregister plugin function
bool unregisterPlugin(const std::string& identifier)
{
    return mx::PluginManager::getInstance().unregisterPlugin(identifier);
}

void bindPyPluginManager(py::module& mod)
{
    // Base plugin class
    py::class_<mx::IPlugin, PyPlugin, std::shared_ptr<mx::IPlugin>>(mod, "IPlugin")
        .def(py::init<>())
        .def_readwrite("_identifier", &mx::IPlugin::_identifier)
        .def_readwrite("_pluginType", &mx::IPlugin::_pluginType)
        .def("getPluginType", &mx::IPlugin::getPluginType)
        .def("getIdentifier", &mx::IPlugin::getIdentifier);

    // Document loader
    py::class_<mx::IDocumentPlugin, mx::IPlugin, PyDocumentPlugin,
        std::shared_ptr<mx::IDocumentPlugin>>(mod, "IDocumentPlugin")
        .def(py::init<>())
        .def("load", &mx::IDocumentPlugin::load)
        .def("save", &mx::IDocumentPlugin::save);

    /* // Image loader
    py::class_<mx::IImageLoader, IPlugin, PyImageLoader,
        std::shared_ptr<mx::IImageLoader>>(m, "IImageLoader")
        .def(py::init<>())
        .def("load_image", &mx::IImageLoader::load_image);
    */

    mod.def("registerPlugin", &registerPlugin,
        "Register a Python plugin with the C++ system");
    mod.def("unregisterPlugin", &unregisterPlugin,
        "Unregister a plugin by its identifier",
        py::arg("identifier"));

    // Expose PluginManager methods
    mod.def("getPlugin", [](const std::string& identifier) -> py::object {
        auto plugin = mx::PluginManager::getInstance().getPlugin(identifier);
        if (plugin) 
        {
            std::cerr << "Return plugin: " << plugin->getIdentifier() << ", Type: " << plugin->getPluginType() << std::endl;
            return py::cast(plugin);
        }
        return py::none();
    }, "Get a plugin by its identifier", py::arg("identifier"));
    
    mod.def("getPlugins", [](const std::string& pluginType) -> py::list {
        auto plugins = mx::PluginManager::getInstance().getPlugins(pluginType);
        py::list result;
        for (const auto& plugin : plugins) 
        {
            std::cerr << "Return plugin: " << plugin->getIdentifier() << ", Type: " << plugin->getPluginType() << std::endl;
            result.append(py::cast(plugin));
        }
        return result;
    }, "Get all plugins of a specific type", py::arg("pluginType"));
    
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

}
