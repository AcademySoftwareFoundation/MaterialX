//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <pybind11/embed.h>
#include <filesystem>
#include <MaterialXRender/PluginManager.h>
#include <iostream>
#include <MaterialXGraphEditor/PluginIntegration.h>

namespace py = pybind11;
namespace fs = std::filesystem;

void load_python_plugins(const std::string& plugin_dir)
{
    py::scoped_interpreter guard{};

    try
    {
        // Set up Python path to include MaterialX modules
        auto sys = py::module_::import("sys");
        sys.attr("path").attr("append")(plugin_dir);
        
        // Add potential MaterialX Python module paths
        // The order matters - check installed location first, then build locations
        std::vector<std::string> potential_paths = {
            "build/installed/python", // Installed Python modules location
            "installed/python",       // Alternative installed location  
            "build/python",           // Build directory (legacy)
            "python",                 // Relative to executable
            "build/lib",              // Alternative build location where .so files are
            "lib"                     // Alternative relative location
        };
        
        for (const auto& path : potential_paths) {
            try {
                auto abs_path = fs::absolute(path);
                if (fs::exists(abs_path)) {
                    sys.attr("path").attr("append")(abs_path.string());
                    std::cout << "Added Python path: " << abs_path.string() << std::endl;
                }
            } catch (const std::exception&) {
                // Ignore individual path errors
            }
        }

        // Import MaterialX modules to make them available to Python plugins
        try
        {
            // Print Python path for debugging
            auto sys_path = sys.attr("path");
            std::cout << "Python sys.path contains:" << std::endl;
            for (auto item : sys_path) {
                std::cout << "  " << py::str(item).cast<std::string>() << std::endl;
            }
            
            // First import the base MaterialX module
            auto pymx = py::module_::import("MaterialX");
            std::cout << "Version: " << pymx.attr("getVersionString")().cast<std::string>() << std::endl;
            std::cout << "MaterialX base module loaded successfully" << std::endl;

            // Don't pre-load render modules - let plugins import them as needed
            // This avoids segfaults from premature module loading

            // Check if plugin_dir exists
            if (!fs::exists(plugin_dir) || !fs::is_directory(plugin_dir))
            {
                std::string absolute_path = fs::absolute(plugin_dir).string();
                std::cerr << "Plugin directory does not exist: " << absolute_path << std::endl;
                return;
            }

            std::cout << "Looking for plugins in: " << fs::absolute(plugin_dir).string() << std::endl;

            // Discover plugins
            for (const auto& entry : fs::directory_iterator(plugin_dir))
            {
                if (entry.is_directory())
                    continue;

                if (entry.path().extension() == ".py")
                {
                    std::string module_name = entry.path().stem().string();
                    std::cout << "Attempting to load plugin: " << module_name << std::endl;

                    try
                    {
                        py::module_ mod = py::module_::import(module_name.c_str());

                        // Find plugin classes
                        py::dict dict = mod.attr("__dict__").cast<py::dict>();
                        for (auto& kv : dict)
                        {
                            py::handle value = kv.second;
                            if (py::isinstance<py::type>(value))
                            {
                                py::type cls = value.cast<py::type>();
                                // Check if it inherits from IPlugin
                                if (py::hasattr(cls, "__bases__"))
                                {
                                    for (auto base : cls.attr("__bases__"))
                                    {
                                        if (py::hasattr(base, "__name__") &&
                                            base.attr("__name__").cast<std::string>() == "IPlugin")
                                        {
                                            // Instantiate and register
                                            auto instance = cls();
                                            
                                            // Try multiple ways to register the plugin
                                            try {
                                                // First try the MaterialX package import
                                                auto bridge = py::module_::import("MaterialX.PyMaterialXRender");
                                                bridge.attr("registerPlugin")(instance);
                                                std::cout << "Loaded plugin: " << module_name << " (via MaterialX package)" << std::endl;
                                            } catch (const py::error_already_set&) {
                                                try {
                                                    // Fallback to direct module import
                                                    auto bridge = py::module_::import("PyMaterialXRender");
                                                    bridge.attr("registerPlugin")(instance);
                                                    std::cout << "Loaded plugin: " << module_name << " (via direct import)" << std::endl;
                                                } catch (const py::error_already_set&) {
                                                    // Final fallback - just instantiate without registration
                                                    std::cout << "Plugin " << module_name << " instantiated but not registered (no bridge available)" << std::endl;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    catch (const py::error_already_set& e)
                    {
                        std::cerr << "Error loading plugin " << module_name << ": " << e.what() << std::endl;
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error in load_python_plugins: " << e.what() << std::endl;
        }
    }
    catch (const py::error_already_set& e)
    {
        std::cerr << "Warning: Could not import MaterialX Python modules: " << e.what() << std::endl;
        std::cerr << "Python error details:" << std::endl;
        if (e.type()) {
            std::cerr << "  Type: " << py::str(e.type()).cast<std::string>() << std::endl;
        }
        if (e.value()) {
            std::cerr << "  Value: " << py::str(e.value()).cast<std::string>() << std::endl;
        }
        if (e.trace()) {
            std::cerr << "  Traceback: " << py::str(e.trace()).cast<std::string>() << std::endl;
        }
        // Continue anyway as plugins might not need these modules
        return;
    }
}