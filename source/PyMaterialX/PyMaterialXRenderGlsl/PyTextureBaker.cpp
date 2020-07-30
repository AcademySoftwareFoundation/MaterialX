//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/TextureBaker.h>
#include <MaterialXCore/Material.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTextureBaker(py::module& mod)
{
    py::class_<mx::TextureBaker, mx::GlslRenderer, mx::TextureBakerPtr>(mod, "TextureBaker")
        .def_static("create", &mx::TextureBaker::create)
        .def_static("bakeAllMaterials", &mx::TextureBaker::bakeAllMaterials);
}
