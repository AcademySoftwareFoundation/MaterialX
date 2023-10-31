//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderStage.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderStage(py::module& mod)
{
    mod.attr("PIXEL_STAGE") = &mx::Stage::PIXEL;

    py::class_<mx::ShaderPortPredicate>(mod, "ShaderPortPredicate")
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a predicate function taking a `ShaderPort` and returning
    a `bool`.
)docstring");

    py::class_<mx::VariableBlock, mx::VariableBlockPtr>(mod, "VariableBlock")

        .def(py::init<const std::string&, const std::string&>(),
             py::arg("name"),
             py::arg("instance"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given `name` and `instance`
    name.
)docstring"))

        .def("getName", &mx::VariableBlock::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of this block.
)docstring"))

        .def("getInstance", &mx::VariableBlock::getInstance,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the instance name of this block.
)docstring"))

        .def("empty", &mx::VariableBlock::empty,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the block has no variables.
)docstring"))

        .def("size", &mx::VariableBlock::size,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of variables in this block.
)docstring"))

        .def("find",
             static_cast<mx::ShaderPort* (mx::VariableBlock::*)(const std::string&)>(&mx::VariableBlock::find),
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a variable by name.

    Returns `None` if no variable is found by the given `name`.
)docstring"))

        .def("find",
             (mx::ShaderPort* (mx::VariableBlock::*)(const mx::ShaderPortPredicate& )) &mx::VariableBlock::find,
             py::arg("predicate"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Find a port based on a predicate.
)docstring"))

        .def("__len__", &mx::VariableBlock::size)
        .def("__getitem__", [](const mx::VariableBlock &vb, size_t i)
        {
            if (i >= vb.size()) throw py::index_error();
            return vb[i];
        }, py::return_value_policy::reference_internal)

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a block of variables in a shader stage.

    :see: https://materialx.org/docs/api/class_variable_block.html
)docstring");

    py::class_<mx::ShaderStage>(mod, "ShaderStage")

        .def(py::init<const std::string&, mx::ConstSyntaxPtr>(),
             py::arg("name"),
             py::arg("syntax"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given `name` and `syntax`.
)docstring"))

        .def("getName", &mx::ShaderStage::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the stage name.
)docstring"))

        .def("getFunctionName", &mx::ShaderStage::getFunctionName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the stage function name.
)docstring"))

        .def("getSourceCode", &mx::ShaderStage::getSourceCode,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the stage source code.
)docstring"))

        .def("getUniformBlock",
             static_cast<mx::VariableBlock& (mx::ShaderStage::*)(const std::string&)>(&mx::ShaderStage::getUniformBlock),
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the uniform variable block with given name.
)docstring"))

        .def("getInputBlock",
             static_cast<mx::VariableBlock& (mx::ShaderStage::*)(const std::string&)>(&mx::ShaderStage::getInputBlock),
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the input variable block with given `name`.
)docstring"))

        .def("getOutputBlock",
             static_cast<mx::VariableBlock& (mx::ShaderStage::*)(const std::string&)>(&mx::ShaderStage::getOutputBlock),
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the output variable block with given `name`.
)docstring"))

        .def("getConstantBlock",
             static_cast<mx::VariableBlock& (mx::ShaderStage::*)()>(&mx::ShaderStage::getConstantBlock),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the constant variable block.
)docstring"))

        .def("getUniformBlocks", &mx::ShaderStage::getUniformBlocks,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a map of all uniform blocks.
)docstring"))

        .def("getInputBlocks", &mx::ShaderStage::getInputBlocks,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a map of all input blocks.
)docstring"))

        .def("getIncludes", &mx::ShaderStage::getIncludes,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a set of all include files.
)docstring"))

        .def("getSourceDependencies", &mx::ShaderStage::getSourceDependencies,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a set of all source dependencies
)docstring"))

        .def("getOutputBlocks", &mx::ShaderStage::getOutputBlocks,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a map of all output blocks.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a shader stage, containing the state and
    resulting source code for the stage.

    :see: https://materialx.org/docs/api/class_shader_stage.html
)docstring");
}
