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
    // static std::unique_ptr<py::scoped_interpreter> guard;
    py::scoped_interpreter guard{};

    try
    {
        // if (!Py_IsInitialized()) {
        //     guard = std::make_unique<py::scoped_interpreter>();
        // }

        auto sys = py::module_::import("sys");
        sys.attr("path").attr("append")(plugin_dir);

        py::exec(R"(
        kwargs = dict(name="World", number=42)
        message = "Hello, {name}! The answer is {number}".format(**kwargs)
        print(message)
    )");

        // return;

        // Import MaterialX modules to make them available to Python plugins
        try
        {
            auto mtlx = py::module_::import("MaterialX");
            // Call getVersionString to ensure MaterialX is loaded
            std::cout << "Version: " << mtlx.attr("getVersionString")().cast<std::string>() << std::endl;
            // py::module_ pymxrender = py::module_::import("MaterialX.PyMaterialXRender");
            std::cout << "MaterialX Python modules loaded successfully" << std::endl;

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
                                            auto bridge = py::module_::import("PyMaterialXRender");
                                            bridge.attr("registerPlugin")(instance);
                                            std::cout << "Loaded plugin: " << module_name << std::endl;
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
        // Continue anyway as plugins might not need these modules
    }
}