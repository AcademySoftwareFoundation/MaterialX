//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GEOMETRYHANDLER_H
#define MATERIALX_GEOMETRYHANDLER_H

/// @file
/// Geometry loader interfaces

#include <MaterialXRender/Mesh.h>

#include <MaterialXFormat/File.h>

#include <map>

namespace MaterialX
{

/// Shared pointer to a GeometryLoader
using GeometryLoaderPtr = std::shared_ptr<class GeometryLoader>;

/// @class GeometryLoader
/// Base class representing a geometry loader. A loader can be
/// associated with one or more file extensions.
class GeometryLoader
{
  public:
    GeometryLoader()
    {
    }
    virtual ~GeometryLoader() { }

    /// Returns a list of supported extensions
    /// @return List of support extensions
    const StringSet& supportedExtensions() const
    {
        return _extensions;
    }

    /// Load geometry from disk. Must be implemented by derived classes.
    /// @param filePath Path to file to load
    /// @param meshList List of meshes to update
    /// @return True if load was successful
    virtual bool load(const FilePath& filePath, MeshList& meshList) = 0;

  protected:
    // List of supported string extensions
    StringSet _extensions;
};

/// Shared pointer to an GeometryHandler
using GeometryHandlerPtr = std::shared_ptr<class GeometryHandler>;

/// Map of extensions to image loaders
using GeometryLoaderMap = std::multimap<string, GeometryLoaderPtr>;

/// @class GeometryHandler
/// Class which holds a set of geometry loaders. Each loader is associated with
/// a given set of file extensions.
class GeometryHandler
{
  public:
    GeometryHandler()
    {
    }
    virtual ~GeometryHandler() { }

    /// Create a new geometry handler
    static GeometryHandlerPtr create()
    {
        return std::make_shared<GeometryHandler>();
    }

    /// Add a geometry loader
    /// @param loader Loader to add to list of available loaders.
    void addLoader(GeometryLoaderPtr loader);

    /// Get a list of extensions supported by the handler
    void supportedExtensions(StringSet& extensions);

    /// Clear all loaded geometry
    void clearGeometry();

    // Determine if any meshes have been loaded from a given location
    bool hasGeometry(const string& location);

    // Find all meshes loaded from a given location
    void getGeometry(MeshList& meshes, const string& location);

    /// Load geometry from a given location
    bool loadGeometry(const FilePath& filePath);

    /// Get list of meshes
    const MeshList& getMeshes() const
    {
        return _meshes;
    }

    /// Return the minimum bounds for all meshes
    const Vector3& getMinimumBounds() const
    {
        return _minimumBounds;
    }

    /// Return the minimum bounds for all meshes
    const Vector3& getMaximumBounds() const
    {
        return _maximumBounds;
    }

    /// Utility to create a quad mesh
    static MeshPtr createQuadMesh();
    
  protected:
    // Recompute bounds for all stored geometry
    void computeBounds();

  protected:
    GeometryLoaderMap _geometryLoaders;
    MeshList _meshes;
    Vector3 _minimumBounds;
    Vector3 _maximumBounds;
};

} // namespace MaterialX

#endif
