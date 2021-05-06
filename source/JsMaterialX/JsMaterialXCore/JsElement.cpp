#include "../helpers.h"
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Traversal.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

#define BIND_VALUE_ELEMENT_FUNC_INSTANCE(NAME, T)          \
    .function("setValue" #NAME, &mx::ValueElement::setValue<T>)

#define BIND_ELEMENT_FUNC_INSTANCE(T)                                  \
    .function("_addChild" #T, &mx::Element::addChild<T>)                   \
    .function("_getChildOfType" #T, &mx::Element::getChildOfType<T>)       \
    .function("_getChildrenOfType" #T, &mx::Element::getChildrenOfType<T>) \
    .function("_removeChildOfType" #T, &mx::Element::removeChildOfType<T>)

extern "C"
{
    EMSCRIPTEN_BINDINGS(element)
    {
        ems::class_<mx::Element>("Element")
            .smart_ptr<std::shared_ptr<mx::Element>>("Element")
            .smart_ptr<std::shared_ptr<const mx::Element>>("Element") // mx::ConstElementPtr
            .function("setCategory", &mx::Element::setCategory)
            .function("getCategory", &mx::Element::getCategory)
            .function("getName", &mx::Element::getName)
            .function("setName", &mx::Element::setName)
            .function("getNamePath", &mx::Element::getNamePath) // might need to do something with the mx::ConstElementPtr relativeTo parameter
            .function("setInheritsFrom", &mx::Element::setInheritsFrom)
            .function("hasInheritedBase", &mx::Element::hasInheritedBase)
            .function("setFilePrefix", &mx::Element::setFilePrefix)
            .function("setColorSpace", &mx::Element::setColorSpace)
            .function("setGeomPrefix", &mx::Element::setGeomPrefix)
            .function("setInheritString", &mx::Element::setInheritString)
            .function("setNamespace", &mx::Element::setNamespace)
            .function("setDocString", &mx::Element::setDocString)
            .function("addChildOfCategory", &mx::Element::addChildOfCategory)
            .function("getActiveFilePrefix", &mx::Element::getActiveFilePrefix)
            .function("getActiveGeomPrefix", &mx::Element::getActiveGeomPrefix)
            .function("getActiveColorSpace", &mx::Element::getActiveColorSpace)
            .function("getInheritsFrom", &mx::Element::getInheritsFrom)
            .function("hasInheritanceCycle", &mx::Element::hasInheritanceCycle)
            .function("getQualifiedName", &mx::Element::getQualifiedName)
            .function("hasFilePrefix", &mx::Element::hasFilePrefix)
            .function("hasGeomPrefix", &mx::Element::hasGeomPrefix)
            .function("hasColorSpace", &mx::Element::hasColorSpace)
            .function("hasInheritString", &mx::Element::hasInheritString)
            .function("hasNamespace", &mx::Element::hasNamespace)
            .function("getFilePrefix", &mx::Element::getFilePrefix)
            .function("getGeomPrefix", &mx::Element::getGeomPrefix)
            .function("getColorSpace", &mx::Element::getColorSpace)
            .function("getInheritString", &mx::Element::getInheritString)
            .function("getNamespace", &mx::Element::getNamespace)
            .function("getDocString", &mx::Element::getDocString)

            .function("getChild", &mx::Element::getChild)
            .function("getChildren", &mx::Element::getChildren)
            .function("setChildIndex", &mx::Element::setChildIndex)
            .function("getChildIndex", &mx::Element::getChildIndex)
            .function("removeChild", &mx::Element::removeChild)

            .function("setAttribute", &mx::Element::setAttribute)
            .function("hasAttribute", &mx::Element::hasAttribute)
            .function("getAttribute", &mx::Element::getAttribute)
            .function("getAttributeNames", &mx::Element::getAttributeNames)
            .function("removeAttribute", &mx::Element::removeAttribute)
            .function("getSelf", ems::optional_override([](mx::Element &self) {
                          return self.mx::Element::getSelf();
                      }))
            .function("getParent", ems::optional_override([](mx::Element &self) {
                          return self.mx::Element::getParent();
                      }))
            .function("getRoot", ems::optional_override([](mx::Element &self) {
                          return self.mx::Element::getRoot();
                      }))
            .function("getDocument", ems::optional_override([](mx::Element &self) {
                          return self.mx::Element::getDocument();
                      }))
            .function("traverseTree", &mx::Element::traverseTree)

            .function("traverseGraph", &mx::Element::traverseGraph)
            .function("getUpstreamEdge", &mx::Element::getUpstreamEdge)
            .function("getUpstreamEdgeCount", &mx::Element::getUpstreamEdgeCount)
            .function("getUpstreamElement", &mx::Element::getUpstreamElement)

            .function("traverseInheritance", &mx::Element::traverseInheritance)
            .function("setSourceUri", &mx::Element::setSourceUri)
            .function("hasSourceUri", &mx::Element::hasSourceUri)
            .function("getSourceUri", &mx::Element::getSourceUri)
            .function("getActiveSourceUri", &mx::Element::getActiveSourceUri)
            .function("validate", ems::optional_override([](mx::Element &self, std::string message) {
                          bool res = self.mx::Element::validate(&message);
                          return res;
                      }))
            .function("copyContentFrom", ems::optional_override([](mx::Element &self, mx::ConstElementPtr source) {
                          const mx::ConstElementPtr &source1 = source;
                          return self.mx::Element::copyContentFrom(source1);
                      }))
            .function("clearContent", &mx::Element::clearContent)
            .function("createValidChildName", &mx::Element::createValidChildName)
            .function("createStringResolver", &mx::Element::createStringResolver)
            .function("asString", &mx::Element::asString)
            .function("__str__", &mx::Element::asString)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Collection)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Document)
            BIND_ELEMENT_FUNC_INSTANCE(mx::GeomInfo)
            BIND_ELEMENT_FUNC_INSTANCE(mx::GeomProp)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Implementation)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Look)
            BIND_ELEMENT_FUNC_INSTANCE(mx::MaterialAssign)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Node)
            BIND_ELEMENT_FUNC_INSTANCE(mx::NodeDef)
            BIND_ELEMENT_FUNC_INSTANCE(mx::NodeGraph)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Property)
            BIND_ELEMENT_FUNC_INSTANCE(mx::PropertySet)
            BIND_ELEMENT_FUNC_INSTANCE(mx::PropertySetAssign)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Token)
            BIND_ELEMENT_FUNC_INSTANCE(mx::TypeDef)
            BIND_ELEMENT_FUNC_INSTANCE(mx::Visibility)
            .class_property("NAME_ATTRIBUTE", &mx::Element::NAME_ATTRIBUTE)
            .class_property("FILE_PREFIX_ATTRIBUTE", &mx::Element::FILE_PREFIX_ATTRIBUTE)
            .class_property("GEOM_PREFIX_ATTRIBUTE", &mx::Element::GEOM_PREFIX_ATTRIBUTE)
            .class_property("COLOR_SPACE_ATTRIBUTE", &mx::Element::COLOR_SPACE_ATTRIBUTE)
            .class_property("INHERIT_ATTRIBUTE", &mx::Element::INHERIT_ATTRIBUTE)
            .class_property("NAMESPACE_ATTRIBUTE", &mx::Element::NAMESPACE_ATTRIBUTE)
            .class_property("DOC_ATTRIBUTE", &mx::Element::DOC_ATTRIBUTE);

        ems::class_<mx::TypedElement, ems::base<mx::Element>>("TypedElement")
            .smart_ptr<std::shared_ptr<mx::TypedElement>>("TypedElement")
            .smart_ptr<std::shared_ptr<const mx::TypedElement>>("TypedElement")
            .function("setType", &mx::TypedElement::setType)
            .function("hasType", &mx::TypedElement::hasType)
            .function("getType", &mx::TypedElement::getType)
            .function("isMultiOutputType", &mx::TypedElement::isMultiOutputType)
            .function("getTypeDef", &mx::TypedElement::getTypeDef)
            .class_property("TYPE_ATTRIBUTE", &mx::TypedElement::TYPE_ATTRIBUTE);

        ems::class_<mx::ValueElement, ems::base<mx::TypedElement>>("ValueElement")
            .smart_ptr<std::shared_ptr<mx::ValueElement>>("ValueElement")
            .smart_ptr<std::shared_ptr<const mx::ValueElement>>("ValueElement")
            .function("setValueString", &mx::ValueElement::setValueString)
            .function("hasValueString", &mx::ValueElement::hasValueString)
            .function("getValueString", &mx::ValueElement::getValueString)
            .function("getResolvedValueString", &mx::ValueElement::getResolvedValueString)
            .function("setInterfaceName", &mx::ValueElement::setInterfaceName)
            .function("hasInterfaceName", &mx::ValueElement::hasInterfaceName)
            .function("getInterfaceName", &mx::ValueElement::getInterfaceName)
            .function("setImplementationName", &mx::ValueElement::setImplementationName)
            .function("hasImplementationName", &mx::ValueElement::hasImplementationName)
            .function("getImplementationName", &mx::ValueElement::getImplementationName)
            .function("getValue", &mx::ValueElement::getValue)
            .function("getDefaultValue", &mx::ValueElement::getDefaultValue)
            .function("setUnit", &mx::ValueElement::setUnit)
            .function("hasUnit", &mx::ValueElement::hasUnit)
            .function("getUnit", &mx::ValueElement::getUnit)
            .function("getActiveUnit", &mx::ValueElement::getActiveUnit)
            .function("setUnitType", &mx::ValueElement::setUnitType)
            .function("hasUnitType", &mx::ValueElement::hasUnitType)
            .function("getUnitType", &mx::ValueElement::getUnitType)
            .function("setIsUniform", &mx::ValueElement::setIsUniform)
            .function("getIsUniform", &mx::ValueElement::getIsUniform)
            .class_property("VALUE_ATTRIBUTE", &mx::ValueElement::VALUE_ATTRIBUTE)
            .class_property("INTERFACE_NAME_ATTRIBUTE", &mx::ValueElement::INTERFACE_NAME_ATTRIBUTE)
            .class_property("IMPLEMENTATION_NAME_ATTRIBUTE", &mx::ValueElement::IMPLEMENTATION_NAME_ATTRIBUTE)
            .class_property("IMPLEMENTATION_TYPE_ATTRIBUTE", &mx::ValueElement::IMPLEMENTATION_TYPE_ATTRIBUTE)
            .class_property("ENUM_ATTRIBUTE", &mx::ValueElement::ENUM_ATTRIBUTE)
            .class_property("ENUM_VALUES_ATTRIBUTE", &mx::ValueElement::ENUM_VALUES_ATTRIBUTE)
            .class_property("UNIT_ATTRIBUTE", &mx::ValueElement::UNIT_ATTRIBUTE)
            .class_property("UI_NAME_ATTRIBUTE", &mx::ValueElement::UI_NAME_ATTRIBUTE)
            .class_property("UI_FOLDER_ATTRIBUTE", &mx::ValueElement::UI_FOLDER_ATTRIBUTE)
            .class_property("UI_MIN_ATTRIBUTE", &mx::ValueElement::UI_MIN_ATTRIBUTE)
            .class_property("UI_MAX_ATTRIBUTE", &mx::ValueElement::UI_MAX_ATTRIBUTE)
            .class_property("UI_SOFT_MIN_ATTRIBUTE", &mx::ValueElement::UI_SOFT_MIN_ATTRIBUTE)
            .class_property("UI_SOFT_MAX_ATTRIBUTE", &mx::ValueElement::UI_SOFT_MAX_ATTRIBUTE)
            .class_property("UI_STEP_ATTRIBUTE", &mx::ValueElement::UI_STEP_ATTRIBUTE)
            .class_property("UI_ADVANCED_ATTRIBUTE", &mx::ValueElement::UI_ADVANCED_ATTRIBUTE)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(integer, int)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(boolean, bool)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(float, float)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(color3, mx::Color3)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(color4, mx::Color4)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector2, mx::Vector2)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector3, mx::Vector3)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector4, mx::Vector4)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(matrix33, mx::Matrix33)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(matrix44, mx::Matrix44)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(string, std::string)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(integerarray, mx::IntVec)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(booleanarray, mx::BoolVec)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(floatarray, mx::FloatVec)
            BIND_VALUE_ELEMENT_FUNC_INSTANCE(stringarray, mx::StringVec);

        ems::class_<mx::Token, ems::base<mx::ValueElement>>("Token")
            .smart_ptr_constructor("Token", &std::make_shared<mx::Token, mx::ElementPtr, const std::string &>)
            .class_property("CATEGORY", &mx::Token::CATEGORY);

        ems::class_<mx::StringResolver>("StringResolver")
            .smart_ptr<std::shared_ptr<mx::StringResolver>>("StringResolver")
            .class_function("create", &mx::StringResolver::create) // Static function for creating a mx::StringResolver instance
            .function("setFilePrefix", &mx::StringResolver::setFilePrefix)
            .function("getFilePrefix", &mx::StringResolver::getFilePrefix)
            .function("setGeomPrefix", &mx::StringResolver::setGeomPrefix)
            .function("getGeomPrefix", &mx::StringResolver::getGeomPrefix)
            .function("setUdimString", &mx::StringResolver::setUdimString)
            .function("setUvTileString", &mx::StringResolver::setUvTileString)
            .function("setFilenameSubstitution", &mx::StringResolver::setFilenameSubstitution)
            .function("getFilenameSubstitutions", ems::optional_override([](mx::StringResolver &self) {
                          std::unordered_map<std::string, std::string> res = self.mx::StringResolver::getFilenameSubstitutions();
                          ems::val obj = ems::val::object();
                          for (const auto &[key, value] : res)
                          {
                              obj.set(key, value);
                          }

                          return obj;
                      }))
            .function("setGeomNameSubstitution", &mx::StringResolver::setGeomNameSubstitution)
            .function("getGeomNameSubstitutions", ems::optional_override([](mx::StringResolver &self) {
                          std::unordered_map<std::string, std::string> res = self.mx::StringResolver::getGeomNameSubstitutions();
                          ems::val obj = ems::val::object();
                          for (const auto &[key, value] : res)
                          {
                              obj.set(key, value);
                          }

                          return obj;
                      }))
            .function("resolve", &mx::StringResolver::resolve);

        ems::class_<mx::ElementPredicate>("ElementPredicate");
    }
}