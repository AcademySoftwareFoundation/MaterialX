//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDER_PLUGINMANAGER_H
#define MATERIALX_RENDER_PLUGINMANAGER_H

/// @file
/// Plugin manager for MaterialX import/export functionality

#include <MaterialXRender/Export.h>
#include <MaterialXCore/Document.h>

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <functional>

MATERIALX_NAMESPACE_BEGIN

/// Plugin types supported by the plugin manager
enum class PluginType
{
    IMPORT,
    EXPORT
};

/// Plugin information structure
struct PluginInfo
{
    std::string identifier;
    std::string name;
    std::string description;
    std::vector<std::string> supportedExtensions;
    PluginType type;
    std::string version;
};

/// Base class for all plugins
class MX_RENDER_API Plugin
{
  public:
    Plugin(const std::string& identifier, const std::string& name, const std::string& description, 
           const std::vector<std::string>& extensions, PluginType type, const std::string& version = "1.0.0") :
        _info{identifier, name, description, extensions, type, version}
    {
    }

    virtual ~Plugin() = default;

    /// Get plugin information
    const PluginInfo& getInfo() const { return _info; }

    /// Get plugin identifier
    const std::string& getIdentifier() const { return _info.identifier; }

    /// Get plugin type
    PluginType getType() const { return _info.type; }

    /// Check if plugin supports a given file extension
    bool supportsExtension(const std::string& extension) const;

  protected:
    PluginInfo _info;
};

/// Import plugin interface
class MX_RENDER_API ImportPlugin : public Plugin
{
  public:
    ImportPlugin(const std::string& identifier, const std::string& name, const std::string& description,
                 const std::vector<std::string>& extensions, const std::string& version = "1.0.0") :
        Plugin(identifier, name, description, extensions, PluginType::IMPORT, version)
    {
    }

    /// Import a document from a file
    /// @param filename The file to import
    /// @param options Optional parameters for import
    /// @return A MaterialX document or nullptr on failure
    virtual DocumentPtr importDocument(const std::string& filename, 
                                     const std::map<std::string, std::string>& options = {}) = 0;

    /// Check if the plugin can import the given file
    /// @param filename The file to check
    /// @return True if the plugin can import the file
    virtual bool canImport(const std::string& filename) const;
};

/// Export plugin interface
class MX_RENDER_API ExportPlugin : public Plugin
{
  public:
    ExportPlugin(const std::string& identifier, const std::string& name, const std::string& description,
                 const std::vector<std::string>& extensions, const std::string& version = "1.0.0") :
        Plugin(identifier, name, description, extensions, PluginType::EXPORT, version)
    {
    }

    /// Export a document to a file
    /// @param document The document to export
    /// @param filename The output filename
    /// @param options Optional parameters for export
    /// @return True on success, false on failure
    virtual bool exportDocument(ConstDocumentPtr document, const std::string& filename,
                               const std::map<std::string, std::string>& options = {}) = 0;

    /// Check if the plugin can export to the given file
    /// @param filename The file to check
    /// @return True if the plugin can export to the file
    virtual bool canExport(const std::string& filename) const;
};

/// Plugin manager class that handles registration and discovery of plugins
class MX_RENDER_API PluginManager
{
  public:
    /// Get the singleton instance of the plugin manager
    static PluginManager& getInstance();

    /// Register an import plugin
    /// @param plugin Shared pointer to the import plugin
    /// @return True if registration was successful
    bool registerImportPlugin(std::shared_ptr<ImportPlugin> plugin);

    /// Register an export plugin
    /// @param plugin Shared pointer to the export plugin
    /// @return True if registration was successful
    bool registerExportPlugin(std::shared_ptr<ExportPlugin> plugin);

    /// Unregister a plugin by identifier
    /// @param identifier The plugin identifier
    /// @return True if unregistration was successful
    bool unregisterPlugin(const std::string& identifier);

    /// Get all registered import plugins
    const std::vector<std::shared_ptr<ImportPlugin>>& getImportPlugins() const { return _importPlugins; }

    /// Get all registered export plugins
    const std::vector<std::shared_ptr<ExportPlugin>>& getExportPlugins() const { return _exportPlugins; }

    /// Get import plugins that support a given file extension
    /// @param extension The file extension (with or without leading dot)
    /// @return Vector of import plugins that support the extension
    std::vector<std::shared_ptr<ImportPlugin>> getImportPluginsForExtension(const std::string& extension) const;

    /// Get export plugins that support a given file extension
    /// @param extension The file extension (with or without leading dot)
    /// @return Vector of export plugins that support the extension
    std::vector<std::shared_ptr<ExportPlugin>> getExportPluginsForExtension(const std::string& extension) const;

    /// Get a specific plugin by identifier
    /// @param identifier The plugin identifier
    /// @return Shared pointer to the plugin, or nullptr if not found
    std::shared_ptr<Plugin> getPlugin(const std::string& identifier) const;

    /// Get all supported import extensions
    std::vector<std::string> getSupportedImportExtensions() const;

    /// Get all supported export extensions
    std::vector<std::string> getSupportedExportExtensions() const;

    /// Import a document using the appropriate plugin
    /// @param filename The file to import
    /// @param pluginId Optional specific plugin identifier to use
    /// @param options Optional parameters for import
    /// @return A MaterialX document or nullptr on failure
    DocumentPtr importDocument(const std::string& filename, 
                             const std::string& pluginId = "",
                             const std::map<std::string, std::string>& options = {});

    /// Export a document using the appropriate plugin
    /// @param document The document to export
    /// @param filename The output filename
    /// @param pluginId Optional specific plugin identifier to use
    /// @param options Optional parameters for export
    /// @return True on success, false on failure
    bool exportDocument(ConstDocumentPtr document, const std::string& filename,
                       const std::string& pluginId = "",
                       const std::map<std::string, std::string>& options = {});

    /// Clear all registered plugins
    void clearPlugins();

    /// Get plugin information for all registered plugins
    std::vector<PluginInfo> getAllPluginInfo() const;

    /// Set callback for plugin registration events
    /// @param callback Function to call when plugins are registered/unregistered
    void setPluginRegistrationCallback(std::function<void(const std::string&, bool)> callback)
    {
        _registrationCallback = callback;
    }

  private:
    PluginManager() = default;
    ~PluginManager() = default;
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    /// Helper function to normalize file extensions
    std::string normalizeExtension(const std::string& extension) const;

    std::vector<std::shared_ptr<ImportPlugin>> _importPlugins;
    std::vector<std::shared_ptr<ExportPlugin>> _exportPlugins;
    std::map<std::string, std::shared_ptr<Plugin>> _pluginRegistry;
    std::function<void(const std::string&, bool)> _registrationCallback;
};

MATERIALX_NAMESPACE_END

#endif
