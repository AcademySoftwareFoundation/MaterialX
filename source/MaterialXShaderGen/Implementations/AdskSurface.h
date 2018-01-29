#ifndef MATERIALX_ADSKSURFACE_H
#define MATERIALX_ADSKSURFACE_H

#include <MaterialXShaderGen/Implementations/SourceCode.h>

namespace MaterialX
{

/// Implementation of 'adskSurface' node for OgsFx
class AdskSurfaceOgsFx : public SourceCode
{
  public:
    static SgImplementationPtr creator();

    const string& getLanguage() const override;

    const string& getTarget() const override;

    bool isTransparent(const SgNode& node) const override;
};

} // namespace MaterialX

#endif
