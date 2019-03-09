//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_FACTORY_H
#define MATERIALX_FACTORY_H

/// @file
/// Class instantiator factory helper class

#include <MaterialXCore/Library.h>

namespace MaterialX
{

/// @class Factory
/// Factory class for creating instances of classes given their type name.
template<class T>
class Factory
{
public:
    using Ptr = shared_ptr<T>;
    using CreatorFunction = Ptr(*)();
    using CreatorMap = std::unordered_map<string, CreatorFunction>;

    /// Register a new class given a unique type name
    /// and a creator function for the class.
    static void registerClass(const string& typeName, CreatorFunction f)
    {
        creatorMap()[typeName] = f;
    }

    /// Determine if a class has been registered for a type name
    static bool classRegistered(const string& typeName)
    {
        CreatorMap& map = creatorMap();
        return map.find(typeName) != map.end();
    }

    /// Unregister a registered class
    static void unregisterClass(const string& typeName)
    {
        CreatorMap& map = creatorMap();
        auto it = map.find(typeName);
        if (it != map.end())
        {
            map.erase(it);
        }
    }

    static void unregisterClasses(StringVec& registeredImplNames)
    {
        for (string registeredImplName : registeredImplNames)
        {
            unregisterClass(registeredImplName);
        }
    }

    /// Create a new instance of the class with given type name.
    /// Returns nullptr if no class with given name is registered.
    static Ptr create(const string& typeName)
    {
        CreatorMap& map = creatorMap();
        auto it = map.find(typeName);
        return (it != map.end() ? it->second() : nullptr);
    }

private:
    static CreatorMap& creatorMap()
    {
        static CreatorMap s_creatorMap;
        return s_creatorMap;
    }
};

} // namespace MaterialX

#endif
