//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/PluginIntegration.h>
#include <MaterialXCore/Util.h>

#include <iostream>
#include <sstream>

GraphEditorPluginIntegration::GraphEditorPluginIntegration() :
    _initialized(false)
{
}

void GraphEditorPluginIntegration::initialize()
{
    if (_initialized)
        return;    // Get the plugin manager and set up any default handlers
    mx::PluginManager& pm = mx::PluginManager::getInstance();
    
    // Set up a callback to log handler registration
    pm.setRegistrationCallback([](const std::string& handlerId, bool registered)
    {
        if (registered)
        {
            std::cout << "Graph Editor Plugin registered: " << handlerId << std::endl;
        }
        else
        {
            std::cout << "Graph Editor Plugin unregistered: " << handlerId << std::endl;
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
    
    // Try to load with PluginManager
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
    
    // Try to export with PluginManager
    try
    {
        bool success = pm.exportDocument(document, filename.asString());
        if (success)
            return true;
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
    std::stringstream ss;
    
    // For now, return basic MaterialX support since we don't have access to 
    // registered loader extensions in the current PluginManager interface
    ss << "MaterialX Documents (*.mtlx)\0*.mtlx\0";
    ss << "All Files (*.*)\0*.*\0";
    ss << "\0";
    
    return ss.str();
}

std::string GraphEditorPluginIntegration::getExportFileFilters()
{
    std::stringstream ss;
    
    // For now, return basic MaterialX support since we don't have access to 
    // registered loader extensions in the current PluginManager interface
    ss << "MaterialX Documents (*.mtlx)\0*.mtlx\0";
    ss << "All Files (*.*)\0*.*\0";
    ss << "\0";
    
    return ss.str();
}

bool GraphEditorPluginIntegration::canImportFile(const mx::FilePath& filename)
{
    // For now, just check if it's a MaterialX file since we don't have access to 
    // registered loader extensions in the current PluginManager interface
    std::string ext = filename.getExtension();
    return (ext == ".mtlx" || ext == ".MTLX");
}

bool GraphEditorPluginIntegration::canExportFile(const mx::FilePath& filename)
{
    // For now, just check if it's a MaterialX file since we don't have access to 
    // registered loader extensions in the current PluginManager interface  
    std::string ext = filename.getExtension();
    return (ext == ".mtlx" || ext == ".MTLX");
}
