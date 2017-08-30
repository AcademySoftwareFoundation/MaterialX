// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_PLUGIN_H
#define MATERIALXFORMAYA_PLUGIN_H

#include <ExporterTypes.h>
#include <Factory.h>
#include <NodeTranslator.h>

namespace MaterialXForMaya
{

/// @class Plugin
/// Class to handle translators
class Plugin
{
  public:
    /// Return instance of plug-in
    static Plugin& instance();

    /// Initialize the plug-in
    bool initialize(const string& loadPath);

    /// Creation function type
    using CreatorFunction = NodeTranslatorPtr(*)();
    /// Registration of a creator function (based on type)
    void registerTranslator(const string& typeName, CreatorFunction f);

    /// Default translator
    void setDefaultTranslator(const string& typeName);

    /// Get a translator for a given Maya node
    NodeTranslatorPtr getTranslator(const MObject& mayaNode);

    /// Search path for MatX library
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

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_PLUGIN_H
