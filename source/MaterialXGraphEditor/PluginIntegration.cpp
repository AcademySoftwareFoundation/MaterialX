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
namespace mx = MaterialX;

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
                        std::cout << "Importing module: " << module_name << std::endl;
                        py::module_ mod = py::module_::import(module_name.c_str());
                        std::cout << "Module imported successfully" << std::endl;

                        // Find plugin classes
                        std::cout << "Getting module dictionary..." << std::endl;
                        py::dict dict = mod.attr("__dict__").cast<py::dict>();
                        std::cout << "Module dictionary size: " << dict.size() << std::endl;
                        
                        bool found_any_class = false;
                        for (auto& kv : dict)
                        {
                            std::string key = py::str(kv.first).cast<std::string>();
                            py::handle value = kv.second;
                            std::cout << "Checking attribute: " << key << " (type: " << py::str(value.get_type()).cast<std::string>() << ")" << std::endl;
                            
                            if (py::isinstance<py::type>(value))
                            {
                                found_any_class = true;
                                py::type cls = value.cast<py::type>();
                                std::string class_name = cls.attr("__name__").cast<std::string>();
                                std::cout << "Found class: " << class_name << std::endl;
                                
                                // Check if it inherits from IPlugin by looking for the actual base class
                                if (py::hasattr(cls, "__bases__"))
                                {
                                    auto bases = cls.attr("__bases__");
                                    std::cout << "  Checking " << py::len(bases) << " base classes..." << std::endl;
                                    
                                    bool is_iplugin = false;
                                    int base_index = 0;
                                    for (auto base : bases)
                                    {
                                        base_index++;
                                        std::cout << "    Base " << base_index << ": ";
                                        
                                        if (py::hasattr(base, "__module__") && py::hasattr(base, "__name__"))
                                        {
                                            std::string base_module = base.attr("__module__").cast<std::string>();
                                            std::string base_name = base.attr("__name__").cast<std::string>();
                                            std::cout << base_module << "." << base_name << std::endl;
                                            
                                            // Check for IPlugin from MaterialX.PyMaterialXRender
                                            if ((base_module == "MaterialX.PyMaterialXRender" || base_module == "PyMaterialXRender") && 
                                                base_name == "IDocumentPlugin")
                                            {
                                                std::cout << "      -> MATCH! Found IPlugin base class" << std::endl;
                                                is_iplugin = true;
                                                break;
                                            }
                                            else
                                            {
                                                std::cout << "      -> No match (looking for MaterialX.PyMaterialXRender.IPlugin or PyMaterialXRender.IPlugin)" << std::endl;
                                            }
                                        }
                                        else
                                        {
                                            std::cout << "missing __module__ or __name__ attributes" << std::endl;
                                        }
                                    }
                                    
                                    if (is_iplugin)
                                    {
                                        std::cout << "Found IPlugin subclass: " << class_name << std::endl;
                                        
                                        // Instantiate and register
                                        try {
                                            std::cout << "Creating instance of " << class_name << "..." << std::endl;
                                            auto instance = cls();
                                            std::cout << "Plugin instance created successfully" << std::endl;
                                            
                                            // Try to get plugin info for debugging
                                            try {
                                                auto plugin_id = instance.attr("getIdentifier")();
                                                auto plugin_type = instance.attr("getPluginType")();
                                                std::cout << "Plugin info - ID: " << py::str(plugin_id).cast<std::string>() 
                                                         << ", Type: " << py::str(plugin_type).cast<std::string>() << std::endl;
                                                auto save_function = instance.attr("save");
                                                if (py::isinstance<py::function>(save_function)) {
                                                    std::cout << "Plugin has a save function" << std::endl;
                                                }
                                                else {
                                                    std::cout << "Plugin does not have a save function" << std::endl;
                                                }
                                                auto load_function = instance.attr("load");
                                                if (py::isinstance<py::function>(load_function)) {
                                                    std::cout << "Plugin has a load function" << std::endl;
                                                }
                                                else {
                                                    std::cout << "Plugin does not have a load function" << std::endl;
                                                }
                                            } catch (const py::error_already_set& e) {
                                                std::cerr << "Could not get plugin info: " << e.what() << std::endl;
                                            }
                                            
                                            // Try multiple ways to register the plugin
                                            try {
                                                std::cout << "Attempting registration via MaterialX.PyMaterialXRender..." << std::endl;
                                                // First try the MaterialX package import
                                                auto bridge = py::module_::import("MaterialX.PyMaterialXRender");
                                                std::cout << "MaterialX.PyMaterialXRender module imported" << std::endl;
                                                
                                                // Check if registerPlugin function exists
                                                if (py::hasattr(bridge, "registerPlugin")) {
                                                    std::cout << "registerPlugin function found" << std::endl;
                                                    bridge.attr("registerPlugin")(instance);
                                                    std::cout << "Successfully loaded plugin: " << module_name << " (via MaterialX package)" << std::endl;
                                                } else {
                                                    std::cerr << "registerPlugin function not found in MaterialX.PyMaterialXRender" << std::endl;
                                                    throw py::error_already_set();
                                                }
                                            } catch (const py::error_already_set& e) {
                                                std::cerr << "MaterialX package registration failed: " << e.what() << std::endl;
                                                try {
                                                    std::cout << "Attempting registration via PyMaterialXRender..." << std::endl;
                                                    // Fallback to direct module import
                                                    auto bridge = py::module_::import("PyMaterialXRender");
                                                    std::cout << "PyMaterialXRender module imported" << std::endl;
                                                    
                                                    if (py::hasattr(bridge, "registerPlugin")) {
                                                        std::cout << "registerPlugin function found in direct import" << std::endl;
                                                        bridge.attr("registerPlugin")(instance);
                                                        std::cout << "Successfully loaded plugin: " << module_name << " (via direct import)" << std::endl;
                                                    } else {
                                                        std::cerr << "registerPlugin function not found in PyMaterialXRender" << std::endl;
                                                        throw py::error_already_set();
                                                    }
                                                } catch (const py::error_already_set& e2) {
                                                    std::cerr << "Direct module registration failed: " << e2.what() << std::endl;
                                                    // Final fallback - just instantiate without registration
                                                    std::cout << "Plugin " << module_name << " instantiated but not registered (no bridge available)" << std::endl;
                                                }
                                            }
                                        } catch (const py::error_already_set& e) {
                                            std::cerr << "Failed to instantiate plugin class: " << e.what() << std::endl;
                                            if (e.type()) {
                                                std::cerr << "  Error type: " << py::str(e.type()).cast<std::string>() << std::endl;
                                            }
                                            if (e.value()) {
                                                std::cerr << "  Error value: " << py::str(e.value()).cast<std::string>() << std::endl;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        std::cout << "  Class " << class_name << " does not inherit from IPlugin" << std::endl;
                                    }
                                }
                                else
                                {
                                    std::cout << "  Class " << class_name << " has no __bases__ attribute" << std::endl;
                                }
                            }
                        }
                        
                        if (!found_any_class) {
                            std::cout << "No classes found in module " << module_name << std::endl;
                        }
                    }
                    catch (const py::error_already_set& e)
                    {
                        std::cerr << "Error loading plugin " << module_name << ": " << e.what() << std::endl;
                    }
                }
            }
            
            // Test the newly exposed PluginManager methods
            std::cout << "\n=== Testing PluginManager methods ===" << std::endl;
            try {
                auto bridge = py::module_::import("MaterialX.PyMaterialXRender");
                
                // Test addRegistrationCallback
                std::cout << "Testing addRegistrationCallback..." << std::endl;
                auto callback = py::cpp_function([](const std::string& id, bool registered) {
                    std::cout << "  Callback: Plugin " << id << " " << (registered ? "registered" : "unregistered") << std::endl;
                });
                
                bool callback_added = bridge.attr("addRegistrationCallback")("test_callback", callback).cast<bool>();
                std::cout << "  Callback added: " << (callback_added ? "true" : "false") << std::endl;
                
                // Test getPlugins - get all plugins of DocumentLoader type
#if 1
                std::cout << "Testing getPlugins('DocumentLoader')..." << std::endl;
                auto plugins_list = bridge.attr("getPlugins")("DocumentLoader");
                size_t plugin_count = py::len(plugins_list);
                std::cout << "  Found " << plugin_count << " DocumentLoader plugins" << std::endl;

                // Test getPlugins - get all plugins (empty string should return all)
                std::cout << "Testing getPlugins('')..." << std::endl;
                auto all_plugins_list = bridge.attr("getPlugins")("");
                size_t all_plugin_count = py::len(all_plugins_list);
                std::cout << "  Found " << all_plugin_count << " plugins total" << std::endl;
                
                // Test getPlugin by identifier
                std::cout << "Testing getPlugin..." << std::endl;
                if (plugin_count > 0) {
                    // Get the first plugin from the list and try to retrieve it by ID
                    auto first_plugin = plugins_list[py::int_(0)];
                    if (!first_plugin.is_none()) {
                        try {
                            auto plugin_id = first_plugin.attr("getIdentifier")().cast<std::string>();
                            std::cout << "  Looking for plugin with ID: " << plugin_id << std::endl;
                            
                            auto retrieved_plugin = bridge.attr("getPlugin")(plugin_id);
                            if (!retrieved_plugin.is_none()) {
                                py::gil_scoped_acquire acquire;

                                auto retrieved_id = retrieved_plugin.attr("getIdentifier")().cast<std::string>();
                                std::cout << "  Successfully retrieved plugin: " << retrieved_id << std::endl;
                                // Call plugin load() method for testing

                                auto load_function = retrieved_plugin.attr("load");
                                bool have_load = false;
                                if (py::isinstance<py::function>(load_function)) {
                                    std::cout << "Plugin has a load function" << std::endl;
                                    have_load = true;
                                }
                                else {
                                    std::cout << "Plugin does not have a load function" << std::endl;
                                }

                                auto py_doc = have_load ? load_function("testfile.mtlx") : pybind11::none();
                                
                                auto doc_type = py::module::import("MaterialX").attr("Document");
                                std::cerr << "Document type: " << py::cast<std::string>(py::str(doc_type)) << std::endl;

                                std::string type_name = py::str(py_doc.get_type()).cast<std::string>();
                                std::cout << "Returned Python type: " << type_name << std::endl;

                                if (!py_doc.is_none()) {
                                    std::cout << "  Python document object returned" << std::endl;

                                    // 2. Extract C++ DocumentPtr from Python object
                                    try {
                                        mx::DocumentPtr doc = py_doc.cast<mx::DocumentPtr>();
                                        if (doc) {
                                            std::cout << "  Document loaded successfully: "
                                                << doc->getName() << std::endl;
                                        }
                                        else {
                                            std::cout << "  Cast to DocumentPtr returned null" << std::endl;
                                        }
                                    }
                                    catch (const py::cast_error& e) {
                                        std::cerr << "  Cast error: " << e.what() << std::endl;

                                        // 3. Debugging: Print the actual Python type
                                        std::string type_name = py::str(py_doc.get_type()).cast<std::string>();
                                        std::cerr << "  Actual Python type: " << type_name << std::endl;
                                    }
                                }
                                else {
                                    std::cout << "  Plugin returned None for document" << std::endl;
                                }

                                
                            } else {
                                std::cout << "  Plugin not found by ID" << std::endl;
                            }
                        } catch (const py::error_already_set& e) {
                            std::cerr << "  Error testing getPlugin: " << e.what() << std::endl;
                        }
                    }
                } else {
                    std::cout << "  No plugins available to test getPlugin with" << std::endl;
                }
#endif                
                // Test removeRegistrationCallback
                std::cout << "Testing removeRegistrationCallback..." << std::endl;
                bool callback_removed = bridge.attr("removeRegistrationCallback")("test_callback").cast<bool>();
                std::cout << "  Callback removed: " << (callback_removed ? "true" : "false") << std::endl;
                
            } catch (const py::error_already_set& e) {
                std::cerr << "Error testing PluginManager methods: " << e.what() << std::endl;
            }
            std::cout << "=== End PluginManager method tests ===\n" << std::endl;
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

    mx::PluginManager& globalManager = mx::PluginManager::getInstance();
    mx::IPluginVec plugins = globalManager.getPlugins("DocumentLoader");
    if (plugins.empty())
    {
        std::cerr << "No DocumentLoader plugins found. Please ensure you have installed the MaterialX Python package." << std::endl;
    }
    else
    {
        std::cout << "Found " << plugins.size() << " DocumentLoader plugins:" << std::endl;
        for (const auto& plugin : plugins)
        {
            std::cout << "  Plugin ID: " << plugin->getIdentifier()
                << ", Type: " << plugin->getPluginType() << std::endl;
        }
    }   
}