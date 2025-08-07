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
}

PluginManager::~PluginManager()
{
    _documentHandler = nullptr;
    _registrationCallbacks.clear();
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
        // Notify all registered callbacks
        for (const auto& callbackPair : _registrationCallbacks)
        {
            callbackPair.second(loader->getIdentifier(), true);
        }
    }
    return registered;
}

bool PluginManager::unregisterDocumentLoader(const std::string& identifier)
{
    if (identifier.empty())
    {
        return false;
    }    bool unregistered = _documentHandler->unregisterLoader(identifier);
    if (unregistered)
    {
        // Notify all registered callbacks
        for (const auto& callbackPair : _registrationCallbacks)
        {
            callbackPair.second(identifier, false);
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
    }    return _documentHandler->exportDocument(document, uri);
}

bool PluginManager::addRegistrationCallback(const std::string& identifier, std::function<void(const std::string&, bool)> callback)
{
    if (identifier.empty() || !callback)
    {
        return false;
    }
    
    _registrationCallbacks[identifier] = callback;
    return true;
}

bool PluginManager::removeRegistrationCallback(const std::string& identifier)
{
    if (identifier.empty())
    {
        return false;
    }
    
    auto it = _registrationCallbacks.find(identifier);
    if (it != _registrationCallbacks.end())
    {
        _registrationCallbacks.erase(it);
        return true;
    }
    
    return false;
}

MATERIALX_NAMESPACE_END