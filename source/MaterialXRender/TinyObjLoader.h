//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TINYOBJLOADER_H
#define MATERIALX_TINYOBJLOADER_H

/// @file 
/// OBJ geometry format loader using the TinyObj library

#include <MaterialXRender/GeometryHandler.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a TinyObjLoader
using TinyObjLoaderPtr = std::shared_ptr<class TinyObjLoader>;

/// @class TinyObjLoader
/// Wrapper for geometry loader to read in OBJ files using the TinyObj library.
class MX_RENDER_API TinyObjLoader : public GeometryLoader
{
  public:
    TinyObjLoader()
    {
        _extensions = { "obj", "OBJ" };
    }
    virtual ~TinyObjLoader() { }

    /// Create a new TinyObjLoader
    static TinyObjLoaderPtr create() { return std::make_shared<TinyObjLoader>(); }

    /// Load geometry from disk
    bool load(const FilePath& filePath, MeshList& meshList, bool texcoordVerticalFlip = false) override;
};

MATERIALX_NAMESPACE_END

#endif
