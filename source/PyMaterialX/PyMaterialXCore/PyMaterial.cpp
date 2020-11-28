//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Material.h>
#include <MaterialXCore/Look.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMaterial(py::module& mod)
{
    mod.def("getShaderNodes", &mx::getShaderNodes);
    mod.def("getConnectedOutputs", &mx::getConnectedOutputs);
}

