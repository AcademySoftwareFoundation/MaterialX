#ifndef MATERIALX_OGSFXIMPLEMENTATION_H
#define MATERIALX_OGSFXIMPLEMENTATION_H

#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>

namespace MaterialX
{

/// Base class for node implementations targeting OgsFx
class OgsFxImplementation : public SgImplementation
{
public:
    const string& getLanguage() const override;
    const string& getTarget() const override;

protected:
    OgsFxImplementation() {}

    static const string SPACE;
    static const string WORLD;
    static const string OBJECT;
    static const string MODEL;
    static const string INDEX;
};

} // namespace MaterialX

#endif
