//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TINYGLTFLOADER_H
#define MATERIALX_TINYGLTFLOADER_H

/// @file 
/// GLTF geometry format loader using the TinyGLTF library

#include <MaterialXRender/GeometryHandler.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a GLTFLoader
using TinyGLTFLoaderPtr = std::shared_ptr<class TinyGLTFLoader>;

/// @class GLTFLoader
/// Wrapper for geometry loader to read in GLTF files using the TinyGLTF library.
class MX_RENDER_API TinyGLTFLoader : public GeometryLoader
{
  public:
    TinyGLTFLoader()
    : _debugLevel(0)
    {
        _extensions = { "glb", "GLB", "gltf", "GLTF" };
    }
    virtual ~TinyGLTFLoader() { }

    /// Create a new GLTFLoader
    static TinyGLTFLoaderPtr create() { return std::make_shared<TinyGLTFLoader>(); }

    /// Load geometry from file path
    bool load(const FilePath& filePath, MeshList& meshList, bool texcoordVerticalFlip = false) override;

private:
    unsigned int _debugLevel;
};

MATERIALX_NAMESPACE_END

#endif
