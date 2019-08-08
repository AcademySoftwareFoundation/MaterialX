#ifndef MATERIALX_MAYA_PLUGIN_H
#define MATERIALX_MAYA_PLUGIN_H

#include <MaterialXFormat/File.h>
#include <MaterialXCore/Document.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

class Plugin
{
  public:
    static Plugin& instance();

    /// Plugin initialization
    void initialize(const std::string& pluginLoadPath);

    /// Get the search paths for resources such as texture files
    mx::FileSearchPath getResourceSearchPath() const;

    void loadLibraries();

    /// Get the search paths for MaterialX libraries
    const mx::FileSearchPath& getLibrarySearchPath() const
    {
        return _librarySearchPath;
    }

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
