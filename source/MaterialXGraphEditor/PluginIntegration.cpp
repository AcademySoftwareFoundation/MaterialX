//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/PluginIntegration.h>
#include <MaterialXFormat/Environ.h>

#include <iostream>

#include <pybind11/embed.h>    
#include <pybind11/pybind11.h> 

namespace mx = MaterialX;
namespace py = pybind11;

static std::vector<mx::FilePath> getSearchPaths() 
{
    std::vector<mx::FilePath> search_paths;

    std::string pluginPaths = mx::getEnviron("MATERIALX_PLUGIN_PATHS");
    if (!pluginPaths.empty()) 
    {
        const std::string sep = ";:";
        mx::StringVec pathVec = mx::splitString(pluginPaths, sep);
        for (const std::string& path : pathVec)
        {
            if (!path.empty()) 
            {
                std::cout << "Adding plugin path from env var: " << path << std::endl;
                mx::FilePath p = mx::FilePath(path);
                bool exists = p.exists();
                bool isdir = p.isDirectory();
                std::cout << "exists: " << exists << ", isdir: " << isdir << " for path: " << p.asString() << std::endl;
                if (exists) {
                    if (isdir)
                    {
                        std::cout << "Add in plugin path:" << p.asString() << std::endl;
                        search_paths.push_back(p);
                    }
                }
            }
        }
    }

    return search_paths;
}

static void loadPlugins(py::module_ myplugins_mod) 
{
    py::object pkgutil;
    py::object importlib;
    try {
        pkgutil = py::module_::import("pkgutil");
        importlib = py::module_::import("importlib");
    } catch (const py::error_already_set& e) {
        std::cerr << "Failed to import pkgutil or importlib: " << e.what() << std::endl;
        return;
    }

    for (auto& path : getSearchPaths()) 
    {
        // Add path to sys.path
        try {
            py::module_::import("sys").attr("path").attr("append")(path.getParentPath().asString());
        } catch (const py::error_already_set& e) {
            std::cerr << "Failed to append to sys.path: " << e.what() << std::endl;
            continue;
        }

        std::string package_name = path.getBaseName();

        py::object package;
        try {
            package = importlib.attr("import_module")(package_name);
            std::cout << "Import package '" << package_name << std::endl;
        } catch (const py::error_already_set& e) {
            std::cerr << "Failed to import package '" << package_name << "': " << e.what() << std::endl;
            continue;
        }

        py::object iter_modules;
        try {
            iter_modules = pkgutil.attr("iter_modules")(package.attr("__path__"));
        } catch (const py::error_already_set& e) {
            std::cerr << "Failed to iterate modules in package '" << package_name << "': " << e.what() << std::endl;
            continue;
        }

        for (auto& info : iter_modules) {
            auto module_name = info.attr("name").cast<std::string>();
            try {
                importlib.attr("import_module")(package_name + "." + module_name);
            } catch (const py::error_already_set& e) {
                std::cerr << "Failed to import module '" << package_name << "." << module_name << "': " << e.what() << std::endl;
            }
        }
    }
}

// pybind11 members encapsulated in an Impl struct
#if defined(__GNUC__) && !defined(__clang__)
struct __attribute__((visibility("hidden"))) PluginIntegration::Impl
#else
struct PluginIntegration::Impl
#endif
{
    std::unique_ptr<py::scoped_interpreter> _pyInterpreter;
    py::object _pymxModule;
    py::object _mypluginsModule;
    bool pythonInitialized = false;

    Impl()
    {
        _pyInterpreter = std::make_unique<py::scoped_interpreter>();
    }

    bool initialize()
    {
        pythonInitialized = false;
        try {
            _pymxModule = py::module_::import("MaterialX");
            _mypluginsModule = py::module_::import("MaterialX.PyMaterialXRender");
            std::cout << "-- Initializing integration class --" << std::endl;
            std::cout << "Version: " << _pymxModule.attr("getVersionString")().cast<std::string>() << std::endl;
            std::cout << "MaterialX base module loaded successfully" << std::endl;
            std::cout << "MaterialX.PyMaterialXRender module imported" << std::endl;
            pythonInitialized = true;
        } 
        catch (std::exception& e)
        {
            std::string pythonError = e.what();
            std::cerr << "Error importing MaterialX modules: " << pythonError << std::endl;
            pythonInitialized = false; 
        }

        return pythonInitialized;
    }
};

PluginIntegration::PluginIntegration()
    : _impl(std::make_unique<Impl>())
{
}

bool PluginIntegration::initialize()
{
    return _impl->initialize();
}

PluginIntegration::~PluginIntegration() = default;

void PluginIntegration::loadPythonPlugins()
{
    if (!_impl->pythonInitialized)
    {
        return;
    }

    try 
    {
        // Scan + load plugins from either exe/plugins or env var paths
        loadPlugins(_impl->_mypluginsModule);

        // Check the plugin manager plugins
        mx::PluginManagerPtr manager = _impl->_mypluginsModule.attr("getPluginManager")().cast<mx::PluginManagerPtr>();

        _pluginList = manager->getPluginList();
        for (auto& name : _pluginList ) {
            std::cout << "Check plugin name: " << name << "\n";
            mx::DocumentLoaderPluginPtr p = manager->getPlugin<mx::DocumentLoaderPlugin>(name);
            if (p) 
            {
                std::cout << "- Found plugin as a document loader" << std::endl;
            }
            else if (mx::DocumentSaverPluginPtr ps = manager->getPlugin<mx::DocumentSaverPlugin>(name))
            {
                std::cout << "- Found plugin as a document saver" << std::endl;
            }
            else
            {
                std::cout << "Plugin " << name << " is not found" << std::endl;
            }
        }
    }
    catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << "\n";
    }
}

mx::DocumentPtr PluginIntegration::loadDocument(const std::string& pluginName, const mx::FilePath& path) const
{
    if (!_impl->pythonInitialized)
    {
        return nullptr;
    }

    if (pluginName.empty() || path.isEmpty())
    {
        return nullptr;
    }

    try {
        mx::PluginManagerPtr manager = _impl->_mypluginsModule.attr("getPluginManager")().cast<mx::PluginManagerPtr>();
        mx::DocumentLoaderPluginPtr p = manager->getPlugin<mx::DocumentLoaderPlugin>(pluginName);
        if (p)
        {
            std::cout << "METHOD: Run LOADER plugin : " << pluginName << " with test file" << std::endl;
            mx::DocumentPtr doc = p->run(path);
            return doc;
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error in loadDocument: " << e.what() << std::endl;
    }
    return nullptr;
}

bool PluginIntegration::saveDocument(const std::string& pluginName, mx::DocumentPtr doc, const mx::FilePath& path) const
{
    if (!_impl->pythonInitialized)
    {
        return false;
    }

    if (pluginName.empty() || !doc || path.isEmpty())
    {
        return false;
    }

    try {
        mx::PluginManagerPtr manager = _impl->_mypluginsModule.attr("getPluginManager")().cast<mx::PluginManagerPtr>();
        mx::DocumentSaverPluginPtr ps = manager->getPlugin<mx::DocumentSaverPlugin>(pluginName);
        if (ps)
        {
            std::cout << "METHOD: Run SAVER plugin : " << pluginName << " with test document" << std::endl;
            ps->run(doc, path);
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error in saveDocument: " << e.what() << std::endl;
        return false;
    }
    return true;
}

mx::PluginManagerPtr PluginIntegration::getPluginManager() const
{
    if (!_impl->pythonInitialized)
    {
        return nullptr;
    }

    try {
        return _impl->_mypluginsModule.attr("getPluginManager")().cast<mx::PluginManagerPtr>();
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error in getPluginManager: " << e.what() << std::endl;
        return nullptr;
    }
}
