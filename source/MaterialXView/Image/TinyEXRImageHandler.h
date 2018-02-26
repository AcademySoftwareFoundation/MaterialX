#ifndef MATERIALX_TINYEXRIMAGEHANDLER_H
#define MATERIALX_TINYEXRIMAGEHANDLER_H

#include <MaterialXView/Image/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an ImageHandler
using TinyEXRImageHandlerPtr = std::shared_ptr<class TinyEXRImageHandler>;

/// @class @TinyEXRImageHandler
/// Disk image handler wrapper using TinyEXR
///
class TinyEXRImageHandler : public ImageHandler
{
public:
    /// Static instance creator
    static TinyEXRImageHandlerPtr creator() { return std::make_shared<TinyEXRImageHandler>(); }

    /// Default constructor
    TinyEXRImageHandler() {}

    /// Default destructor
    virtual ~TinyEXRImageHandler() {}

    /// Save image to disk.
    /// @param fileName Name of file to save image to
    /// @param extension String giving extension (image type).
    /// @param width Width of image in pixels
    /// @param height Height of image in pixels
    /// @param channelCount Number of channels per pixel
    /// @param buffer Floating point buffer of pixels.
    bool saveImage(const std::string& fileName,
                    const std::string& extension,
                    unsigned int width,
                    unsigned int height,
                    unsigned int channelCount,
                    const float*  buffer) override;
};

} // namespace MaterialX;

#endif
