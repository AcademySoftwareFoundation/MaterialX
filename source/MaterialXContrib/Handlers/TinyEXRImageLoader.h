//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TINYEXRIMAGELOADER_H
#define MATERIALX_TINYEXRIMAGELOADER_H

#include <MaterialXRender/ImageHandler.h>

namespace MaterialX
{
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
                    const ImageDesc &imageDesc,
                    bool verticalFlip = false) override;
    bool loadImage(const FilePath& filePath, ImageDesc &imageDesc,
                      const ImageDescRestrictions* restrictions = nullptr) override;
};

} // namespace MaterialX;

#endif
