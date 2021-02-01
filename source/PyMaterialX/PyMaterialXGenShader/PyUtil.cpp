//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUtil(py::module& mod)
{
    mod.def("isTransparentSurface", &mx::isTransparentSurface);
    mod.def("mapValueToColor", &mx::mapValueToColor);
    mod.def("requiresImplementation", &mx::requiresImplementation);
    mod.def("elementRequiresShading", &mx::elementRequiresShading);
    mod.def("findRenderableMaterialNodes", &mx::findRenderableMaterialNodes);
    mod.def("findRenderableMaterialNodes", &mx::findRenderableMaterialNodes);
    mod.def("findRenderableElements", &mx::findRenderableElements);
    mod.def("findNodeDefChild", &mx::findNodeDefChild);
    mod.def("tokenSubstitution", &mx::tokenSubstitution);
    mod.def("getUdimCoordinates", &mx::getUdimCoordinates);
    mod.def("getUdimScaleAndOffset", &mx::getUdimScaleAndOffset);
    mod.def("connectsToNodeOfCategory", &mx::connectsToNodeOfCategory);
    mod.def("hasElementAttributes", &mx::hasElementAttributes);
}
