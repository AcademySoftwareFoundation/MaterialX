#ifndef PLUGIN_H
#define PLUGIN_H

#include <MaterialXFormat/File.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

namespace mx = MaterialX;

class Plugin
{
  public:
    static Plugin& instance();

    /// Plugin initialization
    void initialize(const std::string& loadPath);

    /// Get the search paths for MaterialX libraries
    const mx::FileSearchPath& getLibrarySearchPath() const
    {
        return _librarySearchPath;
    }

    /// Get the search paths for resources
    const mx::FileSearchPath& getResourceSearchPath() const
    {
        return _resourceSearchPath;
    }

  private:
    Plugin()
    {
    }

    mx::FileSearchPath _librarySearchPath;
    mx::FileSearchPath _resourceSearchPath;
};

#endif /* PLUGIN_H */
