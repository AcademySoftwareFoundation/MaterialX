//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenContext.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGenContext(py::module& mod)
{
    py::class_<mx::ApplicationVariableHandler>(mod, "ApplicationVariableHandler");

    py::class_<mx::GenContext, mx::GenContextPtr>(mod, "GenContext", "A context class for shader generation.\n\nUsed for thread local storage of data needed during shader generation.")
        .def(py::init<mx::ShaderGeneratorPtr>())
        .def("getShaderGenerator", &mx::GenContext::getShaderGenerator, "Return shader generatior.")
        .def("getOptions", static_cast<mx::GenOptions & (mx::GenContext::*)()>(&mx::GenContext::getOptions), py::return_value_policy::reference, "Return shader generation options.")
        .def("getTypeDesc", &mx::GenContext::getTypeDesc, "Return a TypeDesc for the given type name.")
        .def("registerSourceCodeSearchPath", static_cast<void (mx::GenContext::*)(const mx::FilePath&)>(&mx::GenContext::registerSourceCodeSearchPath), "Register a user search path for finding source code during code generation.")
        .def("registerSourceCodeSearchPath", static_cast<void (mx::GenContext::*)(const mx::FileSearchPath&)>(&mx::GenContext::registerSourceCodeSearchPath), "Register a user search path for finding source code during code generation.")
        .def("resolveSourceFile", &mx::GenContext::resolveSourceFile, "Resolve a source code filename, first checking the given local path then checking any file paths registered by the user.")
        .def("pushUserData", &mx::GenContext::pushUserData, "Add user data to the context to make it available during shader generator.")
        .def("setApplicationVariableHandler", &mx::GenContext::setApplicationVariableHandler, "Set handler for application variables.")
        .def("getApplicationVariableHandler", &mx::GenContext::getApplicationVariableHandler, "Get handler for application variables.");
}

void bindPyGenUserData(py::module& mod)
{
    py::class_<mx::GenUserData, mx::GenUserDataPtr>(mod, "GenUserData", "Base class for custom user data needed during shader generation.")
        .def("getSelf", static_cast<mx::GenUserDataPtr(mx::GenUserData::*)()>(&mx::GenUserData::getSelf), "Return a shared pointer for this object.");
}