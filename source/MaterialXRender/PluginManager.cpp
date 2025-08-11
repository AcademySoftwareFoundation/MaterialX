//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/PluginManager.h>
#include <MaterialXCore/Util.h>

#include <algorithm>
#include <iostream>

MATERIALX_NAMESPACE_BEGIN

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    _plugins.clear();
    _registrationCallbacks.clear();
}


PluginManager& PluginManager::getInstance()
{
    static PluginManager instance;
    std::cerr << ">>>>>>>>> Get MaterialX PluginManager instance at address: 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(&instance) << std::dec << std::endl;
    return instance;
}

void PluginManager::registerPlugin(IPluginPtr plugin)
{
    _plugins.push_back(plugin);

    std::cerr << ">>>>>>>> register loader: " << plugin->getIdentifier() << std::endl;
    // Notify all registered callbacks
    for (const auto& callbackPair : _registrationCallbacks)
    {
        std::cerr << "<<<<< callback" << callbackPair.first << std::endl;
        callbackPair.second(plugin->getIdentifier(), true);
    }
}

bool PluginManager::unregisterPlugin(const std::string& identifier)
{
    if (identifier.empty())
    {
        return false;
    }

    auto it = std::remove_if(_plugins.begin(), _plugins.end(),
        [&identifier](const IPluginPtr& plugin) 
        {
            return plugin->getIdentifier() == identifier;
        });

    if (it != _plugins.end())
    {
        _plugins.erase(it, _plugins.end());

        // Notify all registered callbacks
        for (const auto& callbackPair : _registrationCallbacks)
        {
            callbackPair.second(identifier, false);
        }
        return true;
    }

    return false;
}

IPluginPtr PluginManager::getPlugin(const string& identifier)
{
    for (const auto& plugin : _plugins)
    {
        if (plugin->getIdentifier() == identifier)
        {
            return plugin;
        }
    }
    return nullptr;
}

IPluginVec PluginManager::getPlugins(const string pluginType)
{
    IPluginVec result;
    for (const auto& plugin : _plugins)
    {
        if (plugin->getPluginType() == pluginType)
        {
            result.push_back(plugin);
        }
    }
    return result;
}

bool PluginManager::addRegistrationCallback(const std::string& identifier, std::function<void(const std::string&, bool)> callback)
{
    if (identifier.empty() || !callback)
    {
        return false;
    }

    std::cerr << "Add callbacl: " << identifier << std::endl;
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