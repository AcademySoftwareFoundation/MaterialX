//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TINYEXRIMAGELOADER_H
#define MATERIALX_TINYEXRIMAGELOADER_H

#include <MaterialXRender/ImageHandler.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an TinyEXRImageLoader
using TinyEXRImageLoaderPtr = std::shared_ptr<class TinyEXRImageLoader>;

/// @class @TinyEXRImageLoader
/// Disk image loader wrapper using TinyEXR
///
class TinyEXRImageLoader : public ImageLoader
{
public:
    static TinyEXRImageLoaderPtr create() { return std::make_shared<TinyEXRImageLoader>(); }

    TinyEXRImageLoader() 
    {
        // Add EXR to list of supported extension
        _extensions.insert(EXR_EXTENSION);
    }

    virtual ~TinyEXRImageLoader() {}    

    bool saveImage(const FilePath& filePath,
                   ConstImagePtr image,
                   bool verticalFlip = false) override;
    ImagePtr loadImage(const FilePath& filePath) override;
};

MATERIALX_NAMESPACE_END

#endif
