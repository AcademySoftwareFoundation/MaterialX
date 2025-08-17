//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/PluginManager.h>
#include <MaterialXCore/Value.h> 
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h> 
#include <pybind11/stl_bind.h>
#include <filesystem>
#include <iostream>

namespace py = pybind11;
namespace mx = MaterialX;
namespace fs = std::filesystem;

PYBIND11_MAKE_OPAQUE(mx::PluginOptionMap);

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
    std::string uiName() const override
    {
        PYBIND11_OVERRIDE(std::string, mx::DocumentLoaderPlugin, uiName);
    }
    mx::StringVec supportedExtensions() const override
    {
        PYBIND11_OVERRIDE_PURE(mx::StringVec, mx::DocumentLoaderPlugin, supportedExtensions);
    }
    mx::DocumentPtr run(const std::string& path) override
    {
        PYBIND11_OVERRIDE_PURE(mx::DocumentPtr, DocumentLoaderPlugin, run, path);
    }
    void getOptions(mx::PluginOptionMap& options) const override
    {
        PYBIND11_OVERRIDE(
            void,
            mx::DocumentLoaderPlugin,
            getOptions,
            options
        );
    }
    void setOption(const std::string& key, mx::ValuePtr value) override
    {
        PYBIND11_OVERRIDE(
            void,
            mx::DocumentLoaderPlugin,
            setOption,
            key,
            value
        );
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
    std::string uiName() const override
    {
        PYBIND11_OVERRIDE(std::string, mx::DocumentSaverPlugin, uiName);
    }
    mx::StringVec supportedExtensions() const override
    {
        PYBIND11_OVERRIDE_PURE(mx::StringVec, mx::DocumentSaverPlugin, supportedExtensions);
    }
    void run(mx::DocumentPtr doc, const std::string& path) override
    {
        PYBIND11_OVERRIDE_PURE(void, mx::DocumentSaverPlugin, run, doc, path);
    }
    void getOptions(mx::PluginOptionMap& options) const override
    {
        PYBIND11_OVERRIDE(
            void,
            mx::DocumentSaverPlugin,
            getOptions,
            options
        );
    }
    void setOption(const std::string& key, mx::ValuePtr value) override
    {
        PYBIND11_OVERRIDE(
            void,
            mx::DocumentSaverPlugin,
            setOption,
            key,
            value
        );
    }
};

// Plugin registration function
void registerPlugin(mx::PluginPtr plugin)
{
    mx::PluginManager::getInstance()->registerPlugin(std::move(plugin));
}

// Unregister plugin function
bool unregisterPlugin(const std::string& identifier)
{
    return mx::PluginManager::getInstance()->unregisterPlugin(identifier);
}

void bindPyPluginManager(py::module& mod)
{
    pybind11::bind_map<mx::PluginOptionMap>(mod, "PluginOptionMap"); 
    //py::bind_map<mx::PluginOptionMap>(mod, "PluginOptionMap");

    py::class_<mx::Plugin, mx::PluginPtr>(mod, "Plugin")
        .def("name", &mx::Plugin::name)
        .def("uiName", &mx::Plugin::uiName);

    py::class_<mx::DocumentLoaderPlugin, mx::Plugin, PyDocumentLoaderPlugin, mx::DocumentLoaderPluginPtr>(mod, "DocumentLoaderPlugin")
        .def(py::init<>())
        .def("name", &mx::DocumentLoaderPlugin::name)
        .def("uiName", &mx::DocumentLoaderPlugin::uiName)
        .def("supportedExtensions", &mx::DocumentLoaderPlugin::supportedExtensions)        
        .def("run", &mx::DocumentLoaderPlugin::run)
        .def("getOptions", &mx::DocumentLoaderPlugin::getOptions)
        .def("setOption", &mx::DocumentLoaderPlugin::setOption);

    py::class_<mx::DocumentSaverPlugin, mx::Plugin, PyDocumentSaverPlugin, mx::DocumentSaverPluginPtr>(mod, "DocumentSaverPlugin")
        .def(py::init<>())
        .def("name", &mx::DocumentSaverPlugin::name)
        .def("uiName", &mx::DocumentSaverPlugin::uiName)
        .def("supportedExtensions", &mx::DocumentSaverPlugin::supportedExtensions)
        .def("run", &mx::DocumentSaverPlugin::run)
        .def("getOptions", &mx::DocumentSaverPlugin::getOptions)
        .def("setOption", &mx::DocumentSaverPlugin::setOption);

    // Plugin manager class
    py::class_<mx::PluginManager, mx::PluginManagerPtr>(mod, "PluginManager")
        .def("registerPlugin", [](mx::PluginManagerPtr manager, mx::PluginPtr p)
            {
                std::cout << "Registering plugin: " << p->name() << std::endl;
                manager->registerPlugin(p);
            })
        .def("getPluginList", &mx::PluginManager::getPluginList)
        .def("getLoader", [](mx::PluginManagerPtr manager, const std::string& name) 
            {
                std::cout << "Getting loader plugin: " << name << std::endl;
                return manager->getPlugin<mx::DocumentLoaderPlugin>(name);
            })
        .def("getSaver", [](mx::PluginManagerPtr manager, const std::string& name)
            {
                std::cout << "Getting saver plugin: " << name << std::endl;
                return manager->getPlugin<mx::DocumentSaverPlugin>(name);
            });
    
    // Global functions for plugin management
    mod.def("getPluginManager", []() -> mx::PluginManagerPtr
        {
            return mx::PluginManager::getInstance();
        });

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
