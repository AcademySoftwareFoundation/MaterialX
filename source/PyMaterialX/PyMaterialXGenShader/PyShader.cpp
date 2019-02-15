//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShader(py::module& mod)
{
    py::class_<mx::Shader::Variable, mx::Shader::VariablePtr>(mod, "Variable")
        .def(py::init([](const mx::TypeDesc* t, const std::string& n, const std::string& p, const std::string& s, mx::ValuePtr v) { return new mx::Shader::Variable(t, n, p, s, v); }))
        .def_readwrite("type", &mx::Shader::Variable::type)
        .def_readwrite("name", &mx::Shader::Variable::name)
        .def_readwrite("path", &mx::Shader::Variable::path)
        .def_readwrite("semantic", &mx::Shader::Variable::semantic)
        .def_readwrite("value", &mx::Shader::Variable::value);

    py::class_<mx::Shader::VariableBlock, mx::Shader::VariableBlockPtr>(mod, "VariableBlock")
        .def(py::init([](const std::string& n, const std::string& i) { return new mx::Shader::VariableBlock(n, i); }))
        .def("empty", &mx::Shader::VariableBlock::empty)
        .def("size", &mx::Shader::VariableBlock::size)
        .def("__len__", &mx::Shader::VariableBlock::size)
        .def("__getitem__", [](const mx::Shader::VariableBlock &vb, size_t i)
        {
            if (i >= vb.size()) throw py::index_error();
            return vb[i];
        }, py::return_value_policy::reference_internal)
        .def_readwrite("name", &mx::Shader::VariableBlock::name)
        .def_readwrite("instance", &mx::Shader::VariableBlock::instance)
        .def_readwrite("variableMap", &mx::Shader::VariableBlock::variableMap)
        .def_readwrite("variableOrder", &mx::Shader::VariableBlock::variableOrder);

    py::class_<mx::Shader, mx::ShaderPtr>(mod, "Shader")
        .def("numStages", &mx::Shader::numStages)
        .def("getConstantBlock", &mx::Shader::getConstantBlock)
        .def("getUniformBlocks", &mx::Shader::getUniformBlocks)
        .def("getUniformBlock", &mx::Shader::getUniformBlock)
        .def("getAppDataBlock", &mx::Shader::getAppDataBlock)
        .def("getOutputBlock", &mx::Shader::getOutputBlock)
        .def("getName", &mx::Shader::getName)
        .def("getSourceCode", &mx::Shader::getSourceCode)
        .def_readonly_static("PIXEL_STAGE", &mx::Shader::PIXEL_STAGE)
        .def_readonly_static("PRIVATE_UNIFORMS", &mx::Shader::PRIVATE_UNIFORMS)
        .def_readonly_static("PUBLIC_UNIFORMS", &mx::Shader::PUBLIC_UNIFORMS);
}
