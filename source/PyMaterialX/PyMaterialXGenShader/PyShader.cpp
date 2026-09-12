//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

const std::vector<mx::ShaderNode*>& getShaderNodes(mx::Shader& self)
{
    return self.getGraph().getNodes();
}

const std::vector<mx::ShaderOutput*>& getShaderInputs(mx::Shader& self)
{
    return self.getGraph().getInputSockets();
}

const std::vector<mx::ShaderInput*>& getShaderOutputs(mx::Shader& self)
{
    return self.getGraph().getOutputSockets();  
}

void bindPyShader(py::module& mod)
{
    // Note: py::return_value_policy::reference was needed because getStage returns a
    // ShaderStage& and without this parameter it would return a copy and not a
    // reference
    py::class_<mx::Shader, mx::ShaderPtr>(mod, "Shader")
        .def(py::init<const std::string&, mx::ShaderGraphPtr>())
        .def("getName", &mx::Shader::getName)
        .def("hasStage", &mx::Shader::hasStage)
        .def("numStages", &mx::Shader::numStages)
        .def("getStage", static_cast<mx::ShaderStage & (mx::Shader::*)(size_t)>(&mx::Shader::getStage), py::return_value_policy::reference)
        .def("getStage", static_cast<mx::ShaderStage & (mx::Shader::*)(const std::string&)>(&mx::Shader::getStage), py::return_value_policy::reference)
        .def("getNodes", &getShaderNodes, py::return_value_policy::reference)
        .def("getInputSockets", &getShaderInputs, py::return_value_policy::reference)   
        .def("getOutputSockets", &getShaderOutputs, py::return_value_policy::reference)
        .def("getSourceCode", &mx::Shader::getSourceCode)
        .def("hasAttribute", &mx::Shader::hasAttribute)
        .def("getAttribute", &mx::Shader::getAttribute)
        .def("setAttribute", static_cast<void (mx::Shader::*)(const std::string&)>(&mx::Shader::setAttribute))
        .def("setAttribute", static_cast<void (mx::Shader::*)(const std::string&, mx::ValuePtr)>(&mx::Shader::setAttribute));

    py::class_<mx::ShaderNode, mx::ShaderNodePtr>(mod, "ShaderNode")
        .def_readonly_static("TEXTURE", &mx::ShaderNode::Classification::TEXTURE)
        .def_readonly_static("CLOSURE", &mx::ShaderNode::Classification::CLOSURE)
        .def_readonly_static("SHADER", &mx::ShaderNode::Classification::SHADER)
        .def_readonly_static("MATERIAL", &mx::ShaderNode::Classification::MATERIAL)
        .def_readonly_static("FILETEXTURE", &mx::ShaderNode::Classification::FILETEXTURE)
        .def_readonly_static("CONDITIONAL", &mx::ShaderNode::Classification::CONDITIONAL)
        .def_readonly_static("CONSTANT", &mx::ShaderNode::Classification::CONSTANT)
        .def_readonly_static("BSDF", &mx::ShaderNode::Classification::BSDF)
        .def_readonly_static("BSDF_R", &mx::ShaderNode::Classification::BSDF_R)
        .def_readonly_static("BSDF_T", &mx::ShaderNode::Classification::BSDF_T)
        .def_readonly_static("EDF", &mx::ShaderNode::Classification::EDF)
        .def_readonly_static("VDF", &mx::ShaderNode::Classification::VDF)
        .def_readonly_static("LAYER", &mx::ShaderNode::Classification::LAYER)
        .def_readonly_static("MIX", &mx::ShaderNode::Classification::MIX)
        .def_readonly_static("SURFACE", &mx::ShaderNode::Classification::SURFACE)
        .def_readonly_static("VOLUME", &mx::ShaderNode::Classification::VOLUME)
        .def_readonly_static("LIGHT", &mx::ShaderNode::Classification::LIGHT)
        .def_readonly_static("UNLIT", &mx::ShaderNode::Classification::UNLIT)
        .def_readonly_static("SAMPLE2D", &mx::ShaderNode::Classification::SAMPLE2D)
        .def_readonly_static("SAMPLE3D", &mx::ShaderNode::Classification::SAMPLE3D)
        .def_readonly_static("GEOMETRIC", &mx::ShaderNode::Classification::GEOMETRIC)
        .def_readonly_static("DOT", &mx::ShaderNode::Classification::DOT)
        .def("getName", &mx::ShaderNode::getName)
        .def("getUniqueId", &mx::ShaderNode::getUniqueId)
        .def("getClassification", &mx::ShaderNode::getClassification)
        .def("getInputs", &mx::ShaderNode::getInputs)
        .def("getOutputs", &mx::ShaderNode::getOutputs);
}
