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
}
