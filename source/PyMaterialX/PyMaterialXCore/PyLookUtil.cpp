//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/LookUtil.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyLookUtil(py::module& mod)
{
    mod.def("getActiveLooksString", &mx::getActiveLooks);
    mod.def("appendLookGroup", &mx::appendLookGroup);
    mod.def("appendLook", &mx::appendLook);
    mod.def("combineLooks", &mx::combineLooks);
}

