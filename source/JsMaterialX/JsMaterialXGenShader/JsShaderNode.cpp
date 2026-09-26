//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <JsMaterialX/VectorHelper.h>

#include <MaterialXGenShader/ShaderNode.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(ShaderPort)
{
    ems::class_<mx::ShaderPort>("ShaderPort")
        .smart_ptr<std::shared_ptr<mx::ShaderPort>>("ShaderPortPtr")
        .function("getNode", static_cast<mx::ShaderNode* (mx::ShaderPort::*)()>(&mx::ShaderPort::getNode), ems::allow_raw_pointers())
        .function("getName", &mx::ShaderPort::getName)
        .function("getFullName", &mx::ShaderPort::getFullName)
        .function("getVariable", &mx::ShaderPort::getVariable)
        .function("getType", &mx::ShaderPort::getType)
        .function("getValue", &mx::ShaderPort::getValue)
        .function("getValueString", &mx::ShaderPort::getValueString)
        .function("getPath", &mx::ShaderPort::getPath)
        .function("getUnit", &mx::ShaderPort::getUnit)
        .function("getColorSpace", &mx::ShaderPort::getColorSpace)
        .function("getGeomProp", &mx::ShaderPort::getGeomProp)
        .function("getFlags", &mx::ShaderPort::getFlags)
        .function("hasAuthoredValue", &mx::ShaderPort::hasAuthoredValue)
        .function("setGeomProp", &mx::ShaderPort::setGeomProp)
        ;

    ems::class_<mx::ShaderInput, ems::base<mx::ShaderPort>>("ShaderInput")
        .smart_ptr<std::shared_ptr<mx::ShaderInput>>("ShaderInputPtr")
        .function("getConnection", static_cast<const mx::ShaderOutput* (mx::ShaderInput::*)() const>(&mx::ShaderInput::getConnection), ems::allow_raw_pointers())
        ;

    ems::class_<mx::ShaderOutput, ems::base<mx::ShaderPort>>("ShaderOutput")
        .smart_ptr<std::shared_ptr<mx::ShaderOutput>>("ShaderOutputPtr")
        .function("getConnections", &mx::ShaderOutput::getConnections)
        ;

    ems::class_<mx::ShaderNode>("ShaderNode")
        .smart_ptr<std::shared_ptr<mx::ShaderNode>>("ShaderNodePtr")
        .smart_ptr<std::shared_ptr<const mx::ShaderNode>>("ShaderNode")
        .class_property("TEXTURE", &mx::ShaderNode::Classification::TEXTURE)
        .class_property("CLOSURE", &mx::ShaderNode::Classification::CLOSURE)
        .class_property("SHADER", &mx::ShaderNode::Classification::SHADER)
        .class_property("MATERIAL", &mx::ShaderNode::Classification::MATERIAL)
        .class_property("FILETEXTURE", &mx::ShaderNode::Classification::FILETEXTURE)
        .class_property("CONDITIONAL", &mx::ShaderNode::Classification::CONDITIONAL)
        .class_property("CONSTANT", &mx::ShaderNode::Classification::CONSTANT)
        .class_property("BSDF", &mx::ShaderNode::Classification::BSDF)
        .class_property("BSDF_R", &mx::ShaderNode::Classification::BSDF_R)
        .class_property("BSDF_T", &mx::ShaderNode::Classification::BSDF_T)
        .class_property("EDF", &mx::ShaderNode::Classification::EDF)
        .class_property("VDF", &mx::ShaderNode::Classification::VDF)
        .class_property("LAYER", &mx::ShaderNode::Classification::LAYER)
        .class_property("MIX", &mx::ShaderNode::Classification::MIX)
        .class_property("SURFACE", &mx::ShaderNode::Classification::SURFACE)
        .class_property("VOLUME", &mx::ShaderNode::Classification::VOLUME)
        .class_property("LIGHT", &mx::ShaderNode::Classification::LIGHT)
        .class_property("UNLIT", &mx::ShaderNode::Classification::UNLIT)
        .class_property("SAMPLE2D", &mx::ShaderNode::Classification::SAMPLE2D)
        .class_property("SAMPLE3D", &mx::ShaderNode::Classification::SAMPLE3D)
        .class_property("GEOMETRIC", &mx::ShaderNode::Classification::GEOMETRIC)
        .class_property("DOT", &mx::ShaderNode::Classification::DOT)
        .function("getName", &mx::ShaderNode::getName)
        .function("getUniqueId", &mx::ShaderNode::getUniqueId)
        .function("getClassification", &mx::ShaderNode::getClassification)
        .function("getInputs", &mx::ShaderNode::getInputs)
        .function("getOutputs", &mx::ShaderNode::getOutputs)
        ;
}
