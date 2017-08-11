#ifndef MATERIALXFORMAYA_PLUGIN_H
#define MATERIALXFORMAYA_PLUGIN_H

#include <Types.h>
#include <Factory.h>
#include <NodeTranslator.h>

class Plugin
{
public:
    static Plugin& instance();

    bool initialize(const string& loadPath);

    using CreatorFunction = NodeTranslatorPtr(*)();
    void registerTranslator(const string& typeName, CreatorFunction f);

    void setDefaultTranslator(const string& typeName);

    NodeTranslatorPtr getTranslator(const MObject& mayaNode);

    const string& getLibrarySearchPath() const
    {
        return _librarySearchPath;
    }

private:
    Plugin();

    Factory<NodeTranslator> _factory;
    unordered_map<string,NodeTranslatorPtr> _translators;
    string _defaultTranslator;
    mx::DocumentPtr _translatorData;
    string _librarySearchPath;
};

#endif // MATERIALXFORMAYA_PLUGIN_H
