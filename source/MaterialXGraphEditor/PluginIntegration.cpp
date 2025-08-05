//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/PluginIntegration.h>
#include <MaterialXCore/Util.h>

#include <sstream>
#include <iostream>

GraphEditorPluginIntegration::GraphEditorPluginIntegration() :
    _initialized(false)
{
}

void GraphEditorPluginIntegration::initialize()
{
    if (_initialized)
        return;

    // Get the plugin manager and set up any default plugins
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    // Set up a callback to log plugin registration
    pm.setPluginRegistrationCallback([](const std::string& pluginId, bool registered)
    {
        if (registered)
        {
            std::cout << "Graph Editor Plugin registered: " << pluginId << std::endl;
        }
        else
        {
            std::cout << "Graph Editor Plugin unregistered: " << pluginId << std::endl;
        }
    });

    _initialized = true;
}

void GraphEditorPluginIntegration::setDocumentLoader(std::function<mx::DocumentPtr(const mx::FilePath&)> loader)
{
    _documentLoader = loader;
}

void GraphEditorPluginIntegration::setDocumentSaver(std::function<bool(mx::ConstDocumentPtr, const mx::FilePath&)> saver)
{
    _documentSaver = saver;
}

mx::DocumentPtr GraphEditorPluginIntegration::loadDocumentWithPlugins(const mx::FilePath& filename)
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

bool GraphEditorPluginIntegration::saveDocumentWithPlugins(mx::ConstDocumentPtr document, const mx::FilePath& filename)
{
    if (!document)
        return false;

    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    // First try to save with plugins
    try
    {
        if (pm.exportDocument(document, filename.asString()))
        {
            return true;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Plugin export failed: " << e.what() << std::endl;
    }

    // Fall back to default document saver if available
    if (_documentSaver)
    {
        return _documentSaver(document, filename);
    }

    return false;
}

std::string GraphEditorPluginIntegration::getImportFileFilters()
{
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    std::vector<std::string> extensions = pm.getSupportedImportExtensions();
    
    if (extensions.empty())
        return "";

    std::ostringstream filters;
    
    // Add "All Supported" filter first
    filters << "All Supported Plugin Formats (";
    for (size_t i = 0; i < extensions.size(); ++i)
    {
        if (i > 0)
            filters << ",";
        filters << "*" << extensions[i];
    }
    filters << ")\0";
    for (const std::string& ext : extensions)
    {
        filters << "*" << ext << ";";
    }
    filters << "\0";

    // Add individual format filters
    for (const std::string& ext : extensions)
    {
        filters << "Plugin " << ext << " (*" << ext << ")\0*" << ext << "\0";
    }

    return filters.str();
}

std::string GraphEditorPluginIntegration::getExportFileFilters()
{
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    std::vector<std::string> extensions = pm.getSupportedExportExtensions();
    
    if (extensions.empty())
        return "";

    std::ostringstream filters;
    
    // Add "All Supported" filter first
    filters << "All Supported Plugin Formats (";
    for (size_t i = 0; i < extensions.size(); ++i)
    {
        if (i > 0)
            filters << ",";
        filters << "*" << extensions[i];
    }
    filters << ")\0";
    for (const std::string& ext : extensions)
    {
        filters << "*" << ext << ";";
    }
    filters << "\0";

    // Add individual format filters
    for (const std::string& ext : extensions)
    {
        filters << "Plugin " << ext << " (*" << ext << ")\0*" << ext << "\0";
    }

    return filters.str();
}

bool GraphEditorPluginIntegration::canImportFile(const mx::FilePath& filename)
{
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    // Extract extension from filename
    std::string filenameStr = filename.asString();
    size_t dotPos = filenameStr.find_last_of(".");
    if (dotPos == std::string::npos)
        return false;
    
    std::string extension = filenameStr.substr(dotPos);
    auto plugins = pm.getImportPluginsForExtension(extension);
    
    return !plugins.empty();
}

bool GraphEditorPluginIntegration::canExportFile(const mx::FilePath& filename)
{
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    // Extract extension from filename
    std::string filenameStr = filename.asString();
    size_t dotPos = filenameStr.find_last_of(".");
    if (dotPos == std::string::npos)
        return false;
    
    std::string extension = filenameStr.substr(dotPos);
    auto plugins = pm.getExportPluginsForExtension(extension);
    
    return !plugins.empty();
}

std::vector<mx::PluginInfo> GraphEditorPluginIntegration::getAvailablePlugins()
{
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    return pm.getAllPluginInfo();
}
