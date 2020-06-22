#include "../helpers.h"
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Material.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(material)
    {
        ems::class_<mx::Material, ems::base<mx::Element>>("Material")
            .smart_ptr_constructor("Material", &std::make_shared<mx::Material, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Material>>("Material")
            .function("addShaderRef", &mx::Material::addShaderRef)
            .function("getShaderRef", &mx::Material::getShaderRef)
            .function("getShaderRefs", &mx::Material::getShaderRefs)
            .function("getActiveShaderRefs", &mx::Material::getActiveShaderRefs)
            .function("removeShaderRef", &mx::Material::removeShaderRef)
            .function("getShaderNodeDefs", &mx::Material::getShaderNodeDefs)
            .function("getPrimaryShaderNodeDef", &mx::Material::getPrimaryShaderNodeDef)
            .function("getPrimaryShaderName", &mx::Material::getPrimaryShaderName)
            .function("getPrimaryShaderParameters", &mx::Material::getPrimaryShaderParameters)
            .function("getPrimaryShaderInputs", &mx::Material::getPrimaryShaderInputs)
            .function("getPrimaryShaderTokens", &mx::Material::getPrimaryShaderTokens)
            .function("getGeometryBindings", &mx::Material::getGeometryBindings)
            .class_property("CATEGORY", &mx::Material::CATEGORY);

        ems::class_<mx::BindParam, ems::base<mx::ValueElement>>("BindParam")
            .smart_ptr_constructor("BindParam", &std::make_shared<mx::BindParam, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::BindParam>>("BindParam")
            .class_property("CATEGORY", &mx::BindParam::CATEGORY);

        ems::class_<mx::BindInput, ems::base<mx::ValueElement>>("BindInput")
            .smart_ptr_constructor("BindInput", &std::make_shared<mx::BindInput, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::BindInput>>("BindInput")
            .function("setNodeGraphString", &mx::BindInput::setNodeGraphString)
            .function("hasNodeGraphString", &mx::BindInput::hasNodeGraphString)
            .function("getNodeGraphString", &mx::BindInput::getNodeGraphString)
            .function("setOutputString", &mx::BindInput::setOutputString)
            .function("hasOutputString", &mx::BindInput::hasOutputString)
            .function("getOutputString", &mx::BindInput::getOutputString)
            .function("setConnectedOutput", &mx::BindInput::setConnectedOutput)
            .function("getConnectedOutput", &mx::BindInput::getConnectedOutput)
            .class_property("CATEGORY", &mx::BindInput::CATEGORY);

        ems::class_<mx::BindToken, ems::base<mx::ValueElement>>("BindToken")
            .smart_ptr_constructor("BindToken", &std::make_shared<mx::BindToken, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::BindToken>>("BindToken")
            .class_property("CATEGORY", &mx::BindToken::CATEGORY);

        ems::class_<mx::ShaderRef, ems::base<mx::TypedElement>>("ShaderRef")
            .smart_ptr_constructor("ShaderRef", &std::make_shared<mx::ShaderRef, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::ShaderRef>>("ShaderRef")
            .function("setNodeString", &mx::ShaderRef::setNodeString)
            .function("hasNodeString", &mx::ShaderRef::hasNodeString)
            .function("getNodeString", &mx::ShaderRef::getNodeString)
            .function("setNodeDefString", &mx::ShaderRef::setNodeDefString)
            .function("hasNodeDefString", &mx::ShaderRef::hasNodeDefString)
            .function("getNodeDefString", &mx::ShaderRef::getNodeDefString)
            .function("getNodeDef", &mx::ShaderRef::getNodeDef)

            .function("addBindParam", &mx::ShaderRef::addBindParam)
            .function("getBindParam", &mx::ShaderRef::getBindParam)
            .function("getBindParams", &mx::ShaderRef::getBindParams)
            .function("removeBindParam", &mx::ShaderRef::removeBindParam)

            .function("addBindInput", &mx::ShaderRef::addBindInput)
            .function("getBindInput", &mx::ShaderRef::getBindInput)
            .function("getBindInputs", &mx::ShaderRef::getBindInputs)
            .function("removeBindInput", &mx::ShaderRef::removeBindInput)

            .function("addBindToken", &mx::ShaderRef::addBindToken)
            .function("getBindToken", &mx::ShaderRef::getBindToken)
            .function("getBindTokens", &mx::ShaderRef::getBindTokens)
            .function("removeBindToken", &mx::ShaderRef::removeBindToken)
            .function("getReferencedOutputs", &mx::ShaderRef::getReferencedOutputs)
            .class_property("CATEGORY", &mx::ShaderRef::CATEGORY);
    }
}