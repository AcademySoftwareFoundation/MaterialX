//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDER_PLUGINMANAGER_H
#define MATERIALX_RENDER_PLUGINMANAGER_H

/// @file
/// Plugin manager for MaterialX handler registration and discovery

#include <MaterialXRender/Export.h>
#include <MaterialXCore/Document.h>
//#include <MaterialXRender/DocumentHandler.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

// Base plugin interface
class MX_RENDER_API Plugin 
{
  public:
    virtual ~Plugin() = default;

    // Get the identifier for this plugin
    virtual string name() const = 0;
};

/// Document loader interface
class MX_RENDER_API DocumentLoaderPlugin : public Plugin
{
  public:
    virtual DocumentPtr run(const string& path) = 0;
};

/// Document saver interface
class MX_RENDER_API DocumentSaverPlugin : public Plugin
{
  public:
    virtual void run(DocumentPtr doc, const string& path) = 0;
};

using PluginPtr = shared_ptr<Plugin>;
using DocumentLoaderPluginPtr = shared_ptr<DocumentLoaderPlugin>;
using DocumentSaverPluginPtr = shared_ptr<DocumentSaverPlugin>;
using PluginVec = std::vector<PluginPtr>;

class PluginManager;
using PluginManagerPtr = std::shared_ptr<PluginManager>;

/// Plugin manager class 
class MX_RENDER_API PluginManager
{
  public:
    /// Get the singleton instance of the plugin manager
    static PluginManagerPtr getInstance();
    ~PluginManager() = default;

    /// Register a plugin
    bool registerPlugin(PluginPtr plugin);
    /// Unregister a plugin by name
    bool unregisterPlugin(const std::string& name);

    template <typename T>
    std::shared_ptr<T> getPlugin(const std::string& name) const
    {
        for (auto& p : _plugins) 
        {
            if (p->name() == name) 
            {
                return std::dynamic_pointer_cast<T>(p);
            }
        }
        return nullptr;
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> getPlugins() const
    {
        std::vector<std::shared_ptr<T>> plugins;
        for (auto& p : _plugins)
        {
            if (auto plugin = std::dynamic_pointer_cast<T>(p))
            {
                plugins.push_back(plugin);
            }
        }
        return plugins; 
    }

    StringVec getPluginList() const 
    {
        std::vector<std::string> names;
        for (auto& p : _plugins)
        {
            names.push_back(p->name());
        }
        return names;
    }

    // Registration and unregistration monitoring
    bool addRegistrationCallback(const std::string& identifier, std::function<void(const std::string&, bool)> callback);
    bool removeRegistrationCallback(const std::string& identifier);

  protected:
      PluginManager();

  private:
    //PluginManager(const PluginManager&) = delete;
    //PluginManager& operator=(const PluginManager&) = delete;    

    PluginVec _plugins;
    std::unordered_map<std::string, std::function<void(const std::string&, bool)>> _registrationCallbacks;
};

MATERIALX_NAMESPACE_END

#endif
