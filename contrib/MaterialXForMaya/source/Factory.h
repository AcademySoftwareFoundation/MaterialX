#ifndef MATERIALXFORMAYA_FACTORY_H
#define MATERIALXFORMAYA_FACTORY_H

#include <Types.h>

template<class T>
class Factory
{
public:
    using Ptr = shared_ptr<T>;
    using CreatorFunction = Ptr(*)();
    using CreatorMap = unordered_map<string, CreatorFunction>;

    // Register a new class given a unique type name
    // and a creator function for the class.
    static void registerClass(const string& typeName, CreatorFunction f)
    {
        creatorMap()[typeName] = f;
    }

    // Create a new instance of the class with given type name.
    // Returns nullptr if no class with given name is registered.
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

#endif
