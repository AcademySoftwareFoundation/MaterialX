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

#if 0
PluginManager::~PluginManager()
{
    std::cerr << "Destroy PluginManager instance at address: 0x"
              << std::hex << reinterpret_cast<uintptr_t>(this) << std::dec << std::endl;
    //_plugins.clear();
    //_registrationCallbacks.clear();
}
#endif

PluginManager& PluginManager::getInstance()
{
    static PluginManager instance;
    std::cerr << ">>>>>>>>> Get MaterialX PluginManager instance at address: 0x" 
              << std::hex << reinterpret_cast<uintptr_t>(&instance) << std::dec << std::endl;
    return instance;
}

bool PluginManager::registerPlugin(PluginPtr plugin)
{
    if (plugin)
    {
        const std::string& name = plugin->name();
        auto it = std::find_if(_plugins.begin(), _plugins.end(),
            [&name](const PluginPtr& p) { return p->name() == name; });
        if (it == _plugins.end())
        {
            // Register plugin
            std::cerr << "Register plugin: " << name << std::endl;
            _plugins.push_back(plugin);

            // Notify callback
            for (auto& cb : _registrationCallbacks)
            {
                std::cerr << "Notify callback of registration: " << cb.first << std::endl;
                cb.second(name, true);
            }
            return true;
        }
    }
    return false;
}

bool PluginManager::unregisterPlugin(const std::string& name)
{
    // Find plugin by name
    auto it = std::find_if(_plugins.begin(), _plugins.end(),
        [&name](const PluginPtr& p) { return p->name() == name; });

    if (it != _plugins.end())
    {
        // Unregister plugin
        std::cerr << "Unregister plugin: " << name << std::endl;
        _plugins.erase(it);

        // Notify callback
        for (auto& cb : _registrationCallbacks)
        {
            std::cerr << "Notify callback of unregistration: " << cb.first << std::endl;
            cb.second(name, false);
        }
        return true;

    }
    return false;
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

    std::cerr << "Remove callback: " << identifier << std::endl;
    
    auto it = _registrationCallbacks.find(identifier);
    if (it != _registrationCallbacks.end())
    {
        _registrationCallbacks.erase(it);
        return true;
    }
    
    return false;
}

MATERIALX_NAMESPACE_END