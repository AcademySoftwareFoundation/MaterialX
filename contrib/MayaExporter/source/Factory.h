// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_FACTORY_H
#define MATERIALXFORMAYA_FACTORY_H

#include <ExporterTypes.h>

namespace MaterialXForMaya
{

/// @class Factory
/// Static map of names to node translator functions
template<class T>
class Factory
{
  public:
    /// Shared point to type
    using Ptr = shared_ptr<T>;
    /// Creator function
    using CreatorFunction = Ptr(*)();
    /// Creator map
    using CreatorMap = unordered_map<string, CreatorFunction>;

    /// Register a new class given a unique type name
    /// and a creator function for the class.
    static void registerClass(const string& typeName, CreatorFunction f)
    {
        creatorMap()[typeName] = f;
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

} //namespace MaterialXForMaya

#endif
