#ifndef PLUGIN_H
#define PLUGIN_H

#include <MaterialXFormat/File.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

class Plugin
{
  public:
    static Plugin& instance();

    /// Plugin initialization
    void initialize(const std::string& loadPath);

    /// Get the search paths for MaterialX libraries
    const MaterialX::FileSearchPath& getLibrarySearchPath() const
    {
        return _librarySearchPath;
    }

    /// Get the search paths for resources
    const MaterialX::FileSearchPath& getResourceSearchPath() const
    {
        return _resourceSearchPath;
    }

    /// Get path for shader debugging output
    const MaterialX::FilePath& getShaderDebugPath() const
    {
        return _shaderDebugPath;
    }

  private:
    Plugin()
    {
    }

    MaterialX::FileSearchPath _librarySearchPath;
    MaterialX::FileSearchPath _resourceSearchPath;
    MaterialX::FilePath _shaderDebugPath;
};

#endif /* PLUGIN_H */
