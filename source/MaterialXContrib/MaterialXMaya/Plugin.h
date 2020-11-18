#ifndef MATERIALX_MAYA_PLUGIN_H
#define MATERIALX_MAYA_PLUGIN_H

/// @file
/// Maya plugin definition.

#include <MaterialXFormat/File.h>
#include <MaterialXCore/Document.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

/// @class Plugin
/// A singleton holding data global to the plug-in.
class Plugin
{
  public:
    /// Get the singleton instance.
    static Plugin& instance();

    /// Plug-in initialization.
    void initialize(const std::string& pluginLoadPath);

    /// Get the search paths for resources such as texture files.
    mx::FileSearchPath getResourceSearchPath() const;

    /// Get the search paths for resources such as environment maps.
    mx::FileSearchPath getLightSearchPath() const;

    /// Load or reload MaterialX libraries.
    /// Reconfigures library search paths based on option variables and
    /// recreates the library document.
    void loadLibraries();

    /// Get the search paths for MaterialX libraries.
    const mx::FileSearchPath& getLibrarySearchPath() const
    {
        return _librarySearchPath;
    }

    /// Get the currently loaded MaterialX libraries stored in a document
    /// which can be imported to other documents.
    mx::ConstDocumentPtr getLibraryDocument() const
    {
        return _libraryDocument;
    }

  private:
    Plugin()
    {
    }

    mx::FilePath _pluginLoadPath;
    mx::FileSearchPath _librarySearchPath;

    mx::DocumentPtr _libraryDocument;
};

} // namespace MaterialXMaya

#endif
