#include <JsMaterialX/Helpers.h>
#include <MaterialXFormat/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(xformatUtil)
{
  ems::function("getPathListSeparator", ems::optional_override([](){ return mx::PATH_LIST_SEPARATOR; }));
  ems::function("getSearchPathEnvVar", ems::optional_override([](){ return mx::MATERIALX_SEARCH_PATH_ENV_VAR; }));
}

