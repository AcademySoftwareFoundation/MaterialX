#ifndef PLUGIN_H
#define PLUGIN_H

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

    /// Get the search paths for resources
    const mx::FileSearchPath& getResourceSearchPath() const
    {
        return _resourceSearchPath;
    }

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
    mx::FileSearchPath _resourceSearchPath;

    mx::DocumentPtr _libraryDocument;
};

} // namespace MaterialXMaya

#endif /* PLUGIN_H */
