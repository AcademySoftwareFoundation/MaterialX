//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

std::vector<mx::TypedElementPtr> findRenderableElements(mx::ConstDocumentPtr doc, bool includeReferencedGraphs)
{
    std::vector<mx::TypedElementPtr> elements;
    mx::findRenderableElements(doc, elements, includeReferencedGraphs);
    return elements;
}

void bindPyUtil(py::module& mod)
{
    mod.def("isTransparentSurface", &mx::isTransparentSurface);
    mod.def("mapValueToColor", &mx::mapValueToColor);
    mod.def("requiresImplementation", &mx::requiresImplementation);
    mod.def("elementRequiresShading", &mx::elementRequiresShading);
    mod.def("findRenderableMaterialNodes", &mx::findRenderableMaterialNodes);
    mod.def("findRenderableElements", &findRenderableElements);
    mod.def("getNodeDefInput", &mx::getNodeDefInput);
    mod.def("tokenSubstitution", &mx::tokenSubstitution);
    mod.def("getUdimCoordinates", &mx::getUdimCoordinates);
    mod.def("getUdimScaleAndOffset", &mx::getUdimScaleAndOffset);
    mod.def("connectsToWorldSpaceNode", &mx::connectsToWorldSpaceNode);
    mod.def("hasElementAttributes", &mx::hasElementAttributes);
}
