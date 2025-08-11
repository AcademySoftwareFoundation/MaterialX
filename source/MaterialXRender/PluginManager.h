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
class MX_RENDER_API IPlugin 
{
  public:
    virtual ~IPlugin() = default;

    // Get the type of the plugin
    virtual std::string getPluginType() const = 0;

    // Get the identifier for this plugin
    virtual std::string getIdentifier() const = 0;
};

using IPluginPtr = shared_ptr<IPlugin>;
using IPluginVec = std::vector<IPluginPtr>;

// Document loader interface
class MX_RENDER_API IDocumentPlugin : public IPlugin 
{
  public:
    // Load a document from a file path
    virtual DocumentPtr load(const std::string& path) = 0;

    // Save a document to a file path
    virtual bool save(ConstDocumentPtr document, const std::string& path) = 0;
};

using DocumentLoaderPtr = std::shared_ptr<IDocumentPlugin>;

/// Plugin manager class that handles registration and discovery of various handler types
class MX_RENDER_API PluginManager
{
  public:
    /// Get the singleton instance of the plugin manager
    static PluginManager& getInstance();

    void registerPlugin(IPluginPtr plugin);
    bool unregisterPlugin(const std::string& identifier);

    IPluginPtr getPlugin(const std::string& identifier);
    IPluginVec getPlugins(const string pluginType);

    // Registration and unregistration monitoring
    bool addRegistrationCallback(const std::string& identifier, std::function<void(const std::string&, bool)> callback);
    bool removeRegistrationCallback(const std::string& identifier);

  private:
    PluginManager();
    ~PluginManager();
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;    

    IPluginVec _plugins;
    std::unordered_map<std::string, std::function<void(const std::string&, bool)>> _registrationCallbacks;
};

MATERIALX_NAMESPACE_END

#endif
