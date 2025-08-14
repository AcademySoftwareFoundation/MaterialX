//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/PluginManager.h>
#include <MaterialXGraphEditor/PluginIntegration.h>

#include <pybind11/embed.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace py = pybind11;
namespace fs = std::filesystem;
namespace mx = MaterialX;

static std::vector<mx::FilePath> getSearchPaths() 
{
    std::vector<mx::FilePath> search_paths;

    std::string pluginPaths = mx::getEnviron("MATERIALX_PLUGIN_PATHS");
    if (!pluginPaths.empty()) 
    {
#ifdef _WIN32
        const std::string sep = ";";
#else
        const std::string sep = ":";
#endif
        const mx::StringVec pathVec = mx::splitString(pluginPaths, sep);
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
    auto pkgutil = py::module_::import("pkgutil");
    auto importlib = py::module_::import("importlib");

    for (auto& path : getSearchPaths()) 
    {
        // Add path to sys.path
        py::module_::import("sys").attr("path").attr("append")(path.getParentPath().asString());

        std::string package_name = "plugins";

        py::object package = importlib.attr("import_module")(package_name);

        auto iter_modules = pkgutil.attr("iter_modules")(package.attr("__path__"));
        for (auto& info : iter_modules) {
            auto module_name = info.attr("name").cast<std::string>();
            importlib.attr("import_module")(package_name + "." + module_name);
        }
    }
}

mx::DocumentPtr PluginIntegration::loadDocument(const std::string& pluginName, const mx::FilePath& path) const
{
    if (pluginName.empty() || path.isEmpty())
    {
        return nullptr;
    }
    return nullptr;
}

bool PluginIntegration::saveDocument(const std::string& pluginName, mx::DocumentPtr doc, const mx::FilePath& path) const
{
    if (pluginName.empty() || !doc || path.isEmpty())
    {
        return false;
    }
    return true;
}


void PluginIntegration::loadPythonPlugins()
{
    py::scoped_interpreter guard{};

    try {
        // Import "Materialx" and "MaterialX.PyMaterialXRender"
        auto pymx = py::module_::import("MaterialX");
        std::cout << "Version: " << pymx.attr("getVersionString")().cast<std::string>() << std::endl;
        std::cout << "MaterialX base module loaded successfully" << std::endl;
        py::module_ myplugins_mod = py::module_::import("MaterialX.PyMaterialXRender");
        std::cout << "MaterialX.PyMaterialXRender module imported" << std::endl;


        // Scan + load plugins from either exe/plugins or env var paths
        loadPlugins(myplugins_mod);

        auto& manager = myplugins_mod.attr("getPluginManager")().cast<mx::PluginManager&>();

        _pluginList = manager.getPluginList();
        for (auto& name : _pluginList ) {
            std::cout << "Discovered plugin: " << name << "\n";
            mx::DocumentLoaderPluginPtr p = manager.getPlugin<mx::DocumentLoaderPlugin>(name);
            if (p) 
            {
                std::cout << "Run LOADER plugin : " << name << " with test file" << std::endl;
                p->run("testfile.mtlx");
            }
            else if (mx::DocumentSaverPluginPtr ps = manager.getPlugin<mx::DocumentSaverPlugin>(name))
            {
                std::cout << "Run SAVER plugin : " << name << " with test document" << std::endl;
                mx::DocumentPtr doc = mx::createDocument();
                ps->run(doc, "testfile_out.mtlx");
            }
            else
            {
                std::cout << "Plugin " << name << " is not found" << std::endl;
            }
        }


        //manager.runPlugin("PluginB");
        //manager.run_all();

    }
    catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << "\n";
    }
}
