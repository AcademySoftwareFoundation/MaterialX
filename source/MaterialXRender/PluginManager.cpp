//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/PluginManager.h>
#include <MaterialXCore/Util.h>

#include <algorithm>
#include <sstream>

MATERIALX_NAMESPACE_BEGIN

bool Plugin::supportsExtension(const std::string& extension) const
{
    std::string normalizedExt = extension;
    if (!normalizedExt.empty() && normalizedExt[0] != '.')
    {
        normalizedExt = "." + normalizedExt;
    }
    
    for (const std::string& supportedExt : _info.supportedExtensions)
    {
        std::string normalizedSupported = supportedExt;
        if (!normalizedSupported.empty() && normalizedSupported[0] != '.')
        {
            normalizedSupported = "." + normalizedSupported;
        }
        
        if (stringEndsWith(normalizedExt, normalizedSupported))
        {
            return true;
        }
    }
    return false;
}

bool ImportPlugin::canImport(const std::string& filename) const
{
    // Extract extension from filename
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos)
    {
        return false;
    }
    
    std::string extension = filename.substr(dotPos);
    return supportsExtension(extension);
}

bool ExportPlugin::canExport(const std::string& filename) const
{
    // Extract extension from filename
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos)
    {
        return false;
    }
    
    std::string extension = filename.substr(dotPos);
    return supportsExtension(extension);
}

PluginManager& PluginManager::getInstance()
{
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerImportPlugin(std::shared_ptr<ImportPlugin> plugin)
{
    if (!plugin)
    {
        return false;
    }

    const std::string& identifier = plugin->getIdentifier();
    
    // Check if plugin with same identifier already exists
    if (_pluginRegistry.find(identifier) != _pluginRegistry.end())
    {
        return false;
    }

    _importPlugins.push_back(plugin);
    _pluginRegistry[identifier] = plugin;

    if (_registrationCallback)
    {
        _registrationCallback(identifier, true);
    }

    return true;
}

bool PluginManager::registerExportPlugin(std::shared_ptr<ExportPlugin> plugin)
{
    if (!plugin)
    {
        return false;
    }

    const std::string& identifier = plugin->getIdentifier();
    
    // Check if plugin with same identifier already exists
    if (_pluginRegistry.find(identifier) != _pluginRegistry.end())
    {
        return false;
    }

    _exportPlugins.push_back(plugin);
    _pluginRegistry[identifier] = plugin;

    if (_registrationCallback)
    {
        _registrationCallback(identifier, true);
    }

    return true;
}

bool PluginManager::unregisterPlugin(const std::string& identifier)
{
    auto it = _pluginRegistry.find(identifier);
    if (it == _pluginRegistry.end())
    {
        return false;
    }

    std::shared_ptr<Plugin> plugin = it->second;
    
    // Remove from appropriate vector
    if (plugin->getType() == PluginType::IMPORT)
    {
        auto importIt = std::find_if(_importPlugins.begin(), _importPlugins.end(),
            [&identifier](const std::shared_ptr<ImportPlugin>& p) {
                return p->getIdentifier() == identifier;
            });
        if (importIt != _importPlugins.end())
        {
            _importPlugins.erase(importIt);
        }
    }
    else if (plugin->getType() == PluginType::EXPORT)
    {
        auto exportIt = std::find_if(_exportPlugins.begin(), _exportPlugins.end(),
            [&identifier](const std::shared_ptr<ExportPlugin>& p) {
                return p->getIdentifier() == identifier;
            });
        if (exportIt != _exportPlugins.end())
        {
            _exportPlugins.erase(exportIt);
        }
    }

    _pluginRegistry.erase(it);

    if (_registrationCallback)
    {
        _registrationCallback(identifier, false);
    }

    return true;
}

std::vector<std::shared_ptr<ImportPlugin>> PluginManager::getImportPluginsForExtension(const std::string& extension) const
{
    std::vector<std::shared_ptr<ImportPlugin>> matchingPlugins;
    std::string normalizedExt = normalizeExtension(extension);
    
    for (const auto& plugin : _importPlugins)
    {
        if (plugin->supportsExtension(normalizedExt))
        {
            matchingPlugins.push_back(plugin);
        }
    }
    
    return matchingPlugins;
}

std::vector<std::shared_ptr<ExportPlugin>> PluginManager::getExportPluginsForExtension(const std::string& extension) const
{
    std::vector<std::shared_ptr<ExportPlugin>> matchingPlugins;
    std::string normalizedExt = normalizeExtension(extension);
    
    for (const auto& plugin : _exportPlugins)
    {
        if (plugin->supportsExtension(normalizedExt))
        {
            matchingPlugins.push_back(plugin);
        }
    }
    
    return matchingPlugins;
}

std::shared_ptr<Plugin> PluginManager::getPlugin(const std::string& identifier) const
{
    auto it = _pluginRegistry.find(identifier);
    if (it != _pluginRegistry.end())
    {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::getSupportedImportExtensions() const
{
    std::vector<std::string> extensions;
    
    for (const auto& plugin : _importPlugins)
    {
        const auto& pluginExtensions = plugin->getInfo().supportedExtensions;
        for (const auto& ext : pluginExtensions)
        {
            std::string normalizedExt = normalizeExtension(ext);
            if (std::find(extensions.begin(), extensions.end(), normalizedExt) == extensions.end())
            {
                extensions.push_back(normalizedExt);
            }
        }
    }
    
    std::sort(extensions.begin(), extensions.end());
    return extensions;
}

std::vector<std::string> PluginManager::getSupportedExportExtensions() const
{
    std::vector<std::string> extensions;
    
    for (const auto& plugin : _exportPlugins)
    {
        const auto& pluginExtensions = plugin->getInfo().supportedExtensions;
        for (const auto& ext : pluginExtensions)
        {
            std::string normalizedExt = normalizeExtension(ext);
            if (std::find(extensions.begin(), extensions.end(), normalizedExt) == extensions.end())
            {
                extensions.push_back(normalizedExt);
            }
        }
    }
    
    std::sort(extensions.begin(), extensions.end());
    return extensions;
}

DocumentPtr PluginManager::importDocument(const std::string& filename, 
                                        const std::string& pluginId,
                                        const std::map<std::string, std::string>& options)
{
    if (!pluginId.empty())
    {
        // Use specific plugin
        auto plugin = std::dynamic_pointer_cast<ImportPlugin>(getPlugin(pluginId));
        if (plugin && plugin->canImport(filename))
        {
            return plugin->importDocument(filename, options);
        }
        return nullptr;
    }

    // Find suitable plugin based on file extension
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos)
    {
        return nullptr;
    }
    
    std::string extension = filename.substr(dotPos);
    auto plugins = getImportPluginsForExtension(extension);
    
    for (auto& plugin : plugins)
    {
        if (plugin->canImport(filename))
        {
            try
            {
                return plugin->importDocument(filename, options);
            }
            catch (...)
            {
                // Continue to next plugin if this one fails
                continue;
            }
        }
    }
    
    return nullptr;
}

bool PluginManager::exportDocument(ConstDocumentPtr document, const std::string& filename,
                                 const std::string& pluginId,
                                 const std::map<std::string, std::string>& options)
{
    if (!document)
    {
        return false;
    }

    if (!pluginId.empty())
    {
        // Use specific plugin
        auto plugin = std::dynamic_pointer_cast<ExportPlugin>(getPlugin(pluginId));
        if (plugin && plugin->canExport(filename))
        {
            return plugin->exportDocument(document, filename, options);
        }
        return false;
    }

    // Find suitable plugin based on file extension
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos)
    {
        return false;
    }
    
    std::string extension = filename.substr(dotPos);
    auto plugins = getExportPluginsForExtension(extension);
    
    for (auto& plugin : plugins)
    {
        if (plugin->canExport(filename))
        {
            try
            {
                return plugin->exportDocument(document, filename, options);
            }
            catch (...)
            {
                // Continue to next plugin if this one fails
                continue;
            }
        }
    }
    
    return false;
}

void PluginManager::clearPlugins()
{
    _importPlugins.clear();
    _exportPlugins.clear();
    _pluginRegistry.clear();
}

std::vector<PluginInfo> PluginManager::getAllPluginInfo() const
{
    std::vector<PluginInfo> infos;
    
    for (const auto& pair : _pluginRegistry)
    {
        infos.push_back(pair.second->getInfo());
    }
    
    return infos;
}

std::string PluginManager::normalizeExtension(const std::string& extension) const
{
    if (extension.empty())
    {
        return extension;
    }
    
    if (extension[0] != '.')
    {
        return "." + extension;
    }
    
    return extension;
}

MATERIALX_NAMESPACE_END
