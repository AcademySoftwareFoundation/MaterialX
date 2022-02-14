//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CGLTFLOADER_H
#define MATERIALX_CGLTFLOADER_H

/// @file 
/// GLTF format loader using the cgltf library

#include <MaterialXRender/GeometryHandler.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a GLTFLoader
using CGLTFLoaderPtr = std::shared_ptr<class CGLTFLoader>;

/// @class CGLTFLoader
/// Wrapper for loader to read in GLTF files using the cgltf library.
class MX_RENDER_API CGLTFLoader : public GeometryLoader
{
  public:
    CGLTFLoader()
    : _debugLevel(0)
    {
        _extensions = { "glb", "GLB", "gltf", "GLTF" };
    }
    virtual ~CGLTFLoader() { }

    /// Create a new loader
    static CGLTFLoaderPtr create() { return std::make_shared<CGLTFLoader>(); }

    /// Load geometry from file path
    bool load(const FilePath& filePath, MeshList& meshList, bool texcoordVerticalFlip=false) override;

private:
    unsigned int _debugLevel;
};

MATERIALX_NAMESPACE_END

#endif
