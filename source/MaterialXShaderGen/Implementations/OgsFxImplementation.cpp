#include <MaterialXShaderGen/Implementations/OgsFxImplementation.h>

namespace MaterialX
{

const string OgsFxImplementation::SPACE  = "space";
const string OgsFxImplementation::WORLD  = "world";
const string OgsFxImplementation::OBJECT = "object";
const string OgsFxImplementation::MODEL  = "model";
const string OgsFxImplementation::INDEX  = "index";

const string& OgsFxImplementation::getLanguage() const
{
    return OgsFxShaderGenerator::LANGUAGE;
}

const string& OgsFxImplementation::getTarget() const
{
    return OgsFxShaderGenerator::TARGET;
}

} // namespace MaterialX
