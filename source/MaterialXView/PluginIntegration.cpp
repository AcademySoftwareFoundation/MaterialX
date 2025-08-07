//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXView/PluginIntegration.h>
#include <MaterialXCore/Util.h>

#include <iostream>

PluginIntegration::PluginIntegration() :
    _initialized(false)
{
}

void PluginIntegration::initialize()
{
    if (_initialized)
        return;    // Get the plugin manager and set up any default plugins
    mx::PluginManager& pm = mx::PluginManager::getInstance();
      // Set up a callback to log plugin registration
    pm.addRegistrationCallback("materialx_view", [](const std::string& pluginId, bool registered)
    {
        if (registered)
        {
            std::cout << "Plugin registered: " << pluginId << std::endl;
        }
        else
        {
            std::cout << "Plugin unregistered: " << pluginId << std::endl;
        }
    });    _initialized = true;
}

void PluginIntegration::shutdown()
{
    if (!_initialized)
        return;
    
    // Remove the callback we registered
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    pm.removeRegistrationCallback("materialx_view");
    
    _initialized = false;
}

void PluginIntegration::setDocumentLoader(std::function<mx::DocumentPtr(const mx::FilePath&)> loader)
{
    _documentLoader = loader;
}

mx::DocumentPtr PluginIntegration::loadDocumentWithPlugins(const mx::FilePath& filename)
{
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    // First try to load with plugins
    try
    {
        mx::DocumentPtr doc = pm.importDocument(filename.asString());
        if (doc)
        {
            return doc;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Plugin import failed: " << e.what() << std::endl;
    }

    // Fall back to default document loader if available
    if (_documentLoader)
    {
        return _documentLoader(filename);
    }

    return nullptr;
}

bool PluginIntegration::saveDocumentWithPlugins(mx::ConstDocumentPtr document, const mx::FilePath& filename)
{
    if (!document)
        return false;

    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    try
    {
        return pm.exportDocument(document, filename.asString());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Plugin export failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::pair<std::string, std::string>> PluginIntegration::getImportFileFilters()
{
    std::vector<std::pair<std::string, std::string>> filters;
    
    // For now, return basic MaterialX support since we don't have access to 
    // registered loader extensions in the current PluginManager interface
    filters.emplace_back("MaterialX Documents", "*.mtlx");
    filters.emplace_back("All Files", "*.*");
    
    return filters;
}

std::vector<std::pair<std::string, std::string>> PluginIntegration::getExportFileFilters()
{
    std::vector<std::pair<std::string, std::string>> filters;
    
    // For now, return basic MaterialX support since we don't have access to 
    // registered loader extensions in the current PluginManager interface
    filters.emplace_back("MaterialX Documents", "*.mtlx");
    filters.emplace_back("All Files", "*.*");
    
    return filters;
}

bool PluginIntegration::canImportFile(const mx::FilePath& filename)
{
    // For now, just check if it's a MaterialX file since we don't have access to 
    // registered loader extensions in the current PluginManager interface
    std::string ext = filename.getExtension();
    return (ext == ".mtlx" || ext == ".MTLX");
}

bool PluginIntegration::canExportFile(const mx::FilePath& filename)
{
    // For now, just check if it's a MaterialX file since we don't have access to 
    // registered loader extensions in the current PluginManager interface  
    std::string ext = filename.getExtension();
    return (ext == ".mtlx" || ext == ".MTLX");
}
