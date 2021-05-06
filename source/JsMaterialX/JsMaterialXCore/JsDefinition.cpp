
#include "../helpers.h"
#include <MaterialXCore/Definition.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(definition)
    {
        ems::class_<mx::NodeDef, ems::base<mx::InterfaceElement>>("NodeDef")
            .smart_ptr_constructor("NodeDef", &std::make_shared<mx::NodeDef, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::NodeDef>>("NodeDef")
            .function("setNodeString", &mx::NodeDef::setNodeString)
            .function("hasNodeString", &mx::NodeDef::hasNodeString)
            .function("getNodeString", &mx::NodeDef::getNodeString)
            .function("setNodeGroup", &mx::NodeDef::setNodeGroup)
            .function("hasNodeGroup", &mx::NodeDef::hasNodeGroup)
            .function("getNodeGroup", &mx::NodeDef::getNodeGroup)
            .function("getImplementation", &mx::NodeDef::getImplementation)
            .function("isVersionCompatible", &mx::NodeDef::isVersionCompatible)
            .class_property("CATEGORY", &mx::NodeDef::CATEGORY)
            .class_property("NODE_ATTRIBUTE", &mx::NodeDef::NODE_ATTRIBUTE)
            .class_property("TEXTURE_NODE_GROUP", &mx::NodeDef::TEXTURE_NODE_GROUP)
            .class_property("PROCEDURAL_NODE_GROUP", &mx::NodeDef::PROCEDURAL_NODE_GROUP)
            .class_property("GEOMETRIC_NODE_GROUP", &mx::NodeDef::GEOMETRIC_NODE_GROUP)
            .class_property("ADJUSTMENT_NODE_GROUP", &mx::NodeDef::ADJUSTMENT_NODE_GROUP)
            .class_property("CONDITIONAL_NODE_GROUP", &mx::NodeDef::CONDITIONAL_NODE_GROUP)
            .class_property("ORGANIZATION_NODE_GROUP", &mx::NodeDef::ORGANIZATION_NODE_GROUP);

        ems::class_<mx::Implementation, ems::base<mx::InterfaceElement>>("Implementation")
            .smart_ptr_constructor("Implementation", &std::make_shared<mx::Implementation, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Implementation>>("Implementation")
            .function("setFile", &mx::Implementation::setFile)
            .function("hasFile", &mx::Implementation::hasFile)
            .function("getFile", &mx::Implementation::getFile)
            .function("setFunction", &mx::Implementation::setFunction)
            .function("hasFunction", &mx::Implementation::hasFunction)
            .function("getFunction", &mx::Implementation::getFunction)
            .function("setNodeDef", &mx::Implementation::setNodeDef)
            .function("getNodeDef", &mx::Implementation::getNodeDef)
            .class_property("CATEGORY", &mx::Implementation::CATEGORY)
            .class_property("FILE_ATTRIBUTE", &mx::Implementation::FILE_ATTRIBUTE)
            .class_property("FUNCTION_ATTRIBUTE", &mx::Implementation::FUNCTION_ATTRIBUTE);

        ems::class_<mx::TypeDef, ems::base<mx::Element>>("TypeDef")
            .smart_ptr_constructor("TypeDef", &std::make_shared<mx::TypeDef, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::TypeDef>>("TypeDef")
            .function("setSemantic", &mx::TypeDef::setSemantic)
            .function("hasSemantic", &mx::TypeDef::hasSemantic)
            .function("getSemantic", &mx::TypeDef::getSemantic)
            .function("setContext", &mx::TypeDef::setContext)
            .function("hasContext", &mx::TypeDef::hasContext)
            .function("getContext", &mx::TypeDef::getContext)
            .function("addMember", &mx::TypeDef::addMember)
            .function("getMember", &mx::TypeDef::getMember)
            .function("getMembers", &mx::TypeDef::getMembers)
            .function("removeMember", &mx::TypeDef::removeMember)
            .class_property("CATEGORY", &mx::TypeDef::CATEGORY)
            .class_property("SEMANTIC_ATTRIBUTE", &mx::TypeDef::SEMANTIC_ATTRIBUTE)
            .class_property("CONTEXT_ATTRIBUTE", &mx::TypeDef::CONTEXT_ATTRIBUTE);

        ems::class_<mx::Member, ems::base<mx::TypedElement>>("Member")
            .smart_ptr_constructor("Member", &std::make_shared<mx::Member, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Member>>("Member")
            .class_property("CATEGORY", &mx::Member::CATEGORY);

        ems::class_<mx::Unit, ems::base<mx::Element>>("Unit")
            .smart_ptr_constructor("Unit", &std::make_shared<mx::Unit, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Unit>>("Unit")
            .class_property("CATEGORY", &mx::Unit::CATEGORY);

        ems::class_<mx::UnitDef, ems::base<mx::Element>>("UnitDef")
            .smart_ptr_constructor("UnitDef", &std::make_shared<mx::UnitDef, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::UnitDef>>("UnitDef")
            .function("setUnitType", &mx::UnitDef::setUnitType)
            .function("hasUnitType", &mx::UnitDef::hasUnitType)
            .function("getUnitType", &mx::UnitDef::getUnitType)
            .function("addUnit", &mx::UnitDef::addUnit)
            .function("getUnit", &mx::UnitDef::getUnit)
            .function("getUnits", &mx::UnitDef::getUnits)
            .class_property("CATEGORY", &mx::UnitDef::CATEGORY)
            .class_property("UNITTYPE_ATTRIBUTE", &mx::UnitDef::UNITTYPE_ATTRIBUTE);

        ems::class_<mx::UnitTypeDef, ems::base<mx::Element>>("UnitTypeDef")
            .smart_ptr_constructor("UnitTypeDef", &std::make_shared<mx::UnitTypeDef, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::UnitTypeDef>>("UnitTypeDef")
            .function("getUnitDefs", &mx::UnitTypeDef::getUnitDefs)
            .class_property("CATEGORY", &mx::UnitTypeDef::CATEGORY);
    }
}