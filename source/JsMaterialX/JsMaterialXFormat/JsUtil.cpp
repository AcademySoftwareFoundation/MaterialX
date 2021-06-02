#include "../Helpers.h"
#include <MaterialXFormat/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(xformatUtil)
{
  ems::constant("PATH_LIST_SEPARATOR", mx::PATH_LIST_SEPARATOR);
  ems::constant("MATERIALX_SEARCH_PATH_ENV_VAR", mx::MATERIALX_SEARCH_PATH_ENV_VAR);
  ems::constant("MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR", mx::MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR);
  ems::constant("MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR", mx::MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR);
}

