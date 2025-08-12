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

namespace py = pybind11;
namespace mx = MaterialX;
namespace fs = std::filesystem;

// Trampoline classes
class PyPlugin : public mx::IPlugin {
  public:
    using mx::IPlugin::IPlugin;

    std::string getPluginType() const override {
        PYBIND11_OVERRIDE_PURE(std::string, mx::IPlugin, getPluginType, );
    }

    std::string getIdentifier() const override {
        PYBIND11_OVERRIDE_PURE(std::string, mx::IPlugin, getIdentifier, );
    }
};

class PyDocumentLoader : public mx::IDocumentPlugin {
  public:
    using mx::IDocumentPlugin::IDocumentPlugin;

    std::string getPluginType() const override 
    { 
        return "DocumentLoader"; 
    }
    std::string getIdentifier() const override 
    {
        PYBIND11_OVERRIDE_PURE(std::string, mx::IDocumentPlugin, getIdentifier, );
    }

    mx::DocumentPtr load(const std::string& path) override 
    {
        PYBIND11_OVERRIDE_PURE(mx::DocumentPtr, mx::IDocumentPlugin, load, path);
    }

    bool save(mx::ConstDocumentPtr document, const std::string& path) override
    {
        PYBIND11_OVERRIDE_PURE(bool, mx::IDocumentPlugin, save, document, path);
    }

};

// Plugin registration function
void registerPlugin(py::object py_plugin)
{
    try 
    {
        // Create shared_ptr with custom deleter
        auto plugin_ptr = std::shared_ptr<mx::IPlugin>(
            py_plugin.cast<mx::IPlugin*>(), [](mx::IPlugin*) {});

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
        .def("getPluginType", &mx::IPlugin::getPluginType)
        .def("getIdentifier", &mx::IPlugin::getIdentifier);

    // Document loader
    py::class_<mx::IDocumentPlugin, mx::IPlugin, PyDocumentLoader,
        std::shared_ptr<mx::IDocumentPlugin>>(mod, "IDocumentPlugin")
        .def(py::init<>())
        .def("load", &mx::IDocumentPlugin::load);

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

}
