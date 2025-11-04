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

    py::class_<mx::ShaderPortPredicate>(mod, "ShaderPortPredicate");

    py::class_<mx::VariableBlock, mx::VariableBlockPtr>(mod, "VariableBlock", "A block of variables in a shader stage.")
        .def(py::init<const std::string&, const std::string&>())
        .def("getName", &mx::VariableBlock::getName, "Return the ColorManagementSystem name.")
        .def("getInstance", &mx::VariableBlock::getInstance, "Get the instance name of this block.")
        .def("empty", &mx::VariableBlock::empty, "Return true if the block has no variables.")
        .def("size", &mx::VariableBlock::size, "Return the number of strings in the path.")
        .def("find", static_cast<mx::ShaderPort* (mx::VariableBlock::*)(const std::string&)>(&mx::VariableBlock::find), "Given an input filename, iterate through each path in this sequence, returning the first combined path found on the file system.\n\nOn success, the combined path is returned; otherwise the original filename is returned unmodified.")
        .def("find", (mx::ShaderPort* (mx::VariableBlock::*)(const mx::ShaderPortPredicate& )) &mx::VariableBlock::find, "Given an input filename, iterate through each path in this sequence, returning the first combined path found on the file system.\n\nOn success, the combined path is returned; otherwise the original filename is returned unmodified.")
        .def("__len__", &mx::VariableBlock::size, "Return the number of strings in the path.")
        .def("__getitem__", [](const mx::VariableBlock &vb, size_t i)
        {
            if (i >= vb.size()) throw py::index_error();
            return vb[i];
        }, py::return_value_policy::reference_internal);

    py::class_<mx::ShaderStage>(mod, "ShaderStage", "A shader stage, containing the state and resulting source code for the stage.")
        .def(py::init<const std::string&, mx::ConstSyntaxPtr>())
        .def("getName", &mx::ShaderStage::getName, "Return the ColorManagementSystem name.")
        .def("getFunctionName", &mx::ShaderStage::getFunctionName, "Return the stage function name.")
        .def("getSourceCode", &mx::ShaderStage::getSourceCode, "Return the shader source code for a given shader stage.")
        .def("getUniformBlock", static_cast<mx::VariableBlock& (mx::ShaderStage::*)(const std::string&)>(&mx::ShaderStage::getUniformBlock), "Return the uniform variable block with given name.")
        .def("getInputBlock", static_cast<mx::VariableBlock& (mx::ShaderStage::*)(const std::string&)>(&mx::ShaderStage::getInputBlock), "Return the input variable block with given name.")
        .def("getOutputBlock", static_cast<mx::VariableBlock& (mx::ShaderStage::*)(const std::string&)>(&mx::ShaderStage::getOutputBlock), "Return the output variable block with given name.")
        .def("getConstantBlock", static_cast<mx::VariableBlock& (mx::ShaderStage::*)()>(&mx::ShaderStage::getConstantBlock), "Return the constant variable block.")
        .def("getUniformBlocks", &mx::ShaderStage::getUniformBlocks, "Return a map of all uniform blocks.")
        .def("getInputBlocks", &mx::ShaderStage::getInputBlocks, "Return a map of all input blocks.")
        .def("getIncludes", &mx::ShaderStage::getIncludes, "Return a set of all include files.")
        .def("getSourceDependencies", &mx::ShaderStage::getSourceDependencies, "Return a set of all source dependencies.")
        .def("getOutputBlocks", &mx::ShaderStage::getOutputBlocks, "Return a map of all output blocks.");
}
