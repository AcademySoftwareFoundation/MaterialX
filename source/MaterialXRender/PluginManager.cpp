//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/PluginManager.h>
#include <MaterialXCore/Util.h>

#include <algorithm>

MATERIALX_NAMESPACE_BEGIN

PluginManager::PluginManager()
{
    // Initialize the document handler
    _documentHandler = DocumentHandler::create();
    _registrationCallback = nullptr;
}

PluginManager::~PluginManager()
{
    _documentHandler = nullptr;
    _registrationCallback = nullptr;
}


PluginManager& PluginManager::getInstance()
{
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerDocumentLoader(DocumentLoaderPtr loader)
{
    if (!loader || loader->getIdentifier().empty())
    {
        return false;
    }
    bool registered = _documentHandler->registerLoader(loader);
    if (registered)
    {
        // Notify callback if set
        if (_registrationCallback)
        {
            _registrationCallback(loader->getIdentifier(), true);
        }
    }
    return registered; // Return the actual result from registerLoader
}

bool PluginManager::unregisterDocumentLoader(const std::string& identifier)
{
    if (identifier.empty())
    {
        return false;
    }
    bool unregistered = _documentHandler->unregisterLoader(identifier);
    if (unregistered)
    {
        // Notify callback if set
        if (_registrationCallback)
        {
            _registrationCallback(identifier, false);
        }
        return true;
    }

    return false;
}

DocumentPtr PluginManager::importDocument(const std::string& uri)
{
    return _documentHandler->importDocument(uri);
}

bool PluginManager::exportDocument(ConstDocumentPtr document, const std::string& uri)
{
    if (!document || uri.empty())
    {
        return false;
    }
    return _documentHandler->exportDocument(document, uri);
}

MATERIALX_NAMESPACE_END