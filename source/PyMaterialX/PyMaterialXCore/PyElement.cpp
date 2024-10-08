//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Traversal.h>

#define BIND_ELEMENT_FUNC_INSTANCE(T)                                                                           \
.def("_addChild" #T, &mx::Element::addChild<mx::T>)                                                             \
.def("_getChildOfType" #T, &mx::Element::getChildOfType<mx::T>)                                                 \
.def("_getChildrenOfType" #T, &mx::Element::getChildrenOfType<mx::T>, py::arg("category") = mx::EMPTY_STRING)   \
.def("_removeChildOfType" #T, &mx::Element::removeChildOfType<mx::T>)

#define BIND_VALUE_ELEMENT_FUNC_INSTANCE(NAME, T)                                                               \
.def("_setValue" #NAME, &mx::ValueElement::setValue<T>, py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyElement(py::module& mod)
{
    py::class_<mx::Element, mx::ElementPtr>(mod, "Element")
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("isEquivalent", [](const mx::Element& elem, mx::ConstElementPtr& rhs, const mx::ElementEquivalenceOptions& options)
        {
            mx::ElementEquivalenceResultVec results;
            bool res = elem.isEquivalent(rhs, options, &results);
            return std::pair<bool, mx::ElementEquivalenceResultVec>(res, results);
        })        
        .def("setCategory", &mx::Element::setCategory)
        .def("getCategory", &mx::Element::getCategory)
        .def("setName", &mx::Element::setName)
        .def("getName", &mx::Element::getName)
        .def("getNamePath", &mx::Element::getNamePath,
            py::arg("relativeTo") = nullptr)
        .def("getDescendant", &mx::Element::getDescendant)
        .def("setFilePrefix", &mx::Element::setFilePrefix)
        .def("hasFilePrefix", &mx::Element::hasFilePrefix)
        .def("getFilePrefix", &mx::Element::getFilePrefix)
        .def("getActiveFilePrefix", &mx::Element::getActiveFilePrefix)
        .def("setGeomPrefix", &mx::Element::setGeomPrefix)
        .def("hasGeomPrefix", &mx::Element::hasGeomPrefix)
        .def("getGeomPrefix", &mx::Element::getGeomPrefix)
        .def("getActiveGeomPrefix", &mx::Element::getActiveGeomPrefix)
        .def("setColorSpace", &mx::Element::setColorSpace)
        .def("hasColorSpace", &mx::Element::hasColorSpace)
        .def("getColorSpace", &mx::Element::getColorSpace)
        .def("getActiveColorSpace", &mx::Element::getActiveColorSpace)
        .def("setInheritString", &mx::Element::setInheritString)
        .def("hasInheritString", &mx::Element::hasInheritString)
        .def("getInheritString", &mx::Element::getInheritString)
        .def("setInheritsFrom", &mx::Element::setInheritsFrom)
        .def("getInheritsFrom", &mx::Element::getInheritsFrom)
        .def("hasInheritedBase", &mx::Element::hasInheritedBase)
        .def("hasInheritanceCycle", &mx::Element::hasInheritanceCycle)
        .def("setNamespace", &mx::Element::setNamespace)
        .def("hasNamespace", &mx::Element::hasNamespace)
        .def("getNamespace", &mx::Element::getNamespace)
        .def("getQualifiedName", &mx::Element::getQualifiedName)
        .def("setDocString", &mx::Element::setDocString)
        .def("getDocString", &mx::Element::getDocString)
        .def("addChildOfCategory", &mx::Element::addChildOfCategory,
            py::arg("category"), py::arg("name") = mx::EMPTY_STRING)
        .def("changeChildCategory", &mx::Element::changeChildCategory)
        .def("_getChild", &mx::Element::getChild)
        .def("getChildren", &mx::Element::getChildren)
        .def("setChildIndex", &mx::Element::setChildIndex)
        .def("getChildIndex", &mx::Element::getChildIndex)
        .def("removeChild", &mx::Element::removeChild)
        .def("setAttribute", &mx::Element::setAttribute)
        .def("hasAttribute", &mx::Element::hasAttribute)
        .def("getAttribute", &mx::Element::getAttribute)
        .def("getAttributeNames", &mx::Element::getAttributeNames)
        .def("removeAttribute", &mx::Element::removeAttribute)
        .def("getSelf", static_cast<mx::ElementPtr (mx::Element::*)()>(&mx::Element::getSelf))
        .def("getParent", static_cast<mx::ElementPtr(mx::Element::*)()>(&mx::Element::getParent))
        .def("getRoot", static_cast<mx::ElementPtr(mx::Element::*)()>(&mx::Element::getRoot))
        .def("getDocument", static_cast<mx::DocumentPtr(mx::Element::*)()>(&mx::Element::getDocument))
        .def("traverseTree", &mx::Element::traverseTree)
        .def("traverseGraph", &mx::Element::traverseGraph)
        .def("getUpstreamEdge", &mx::Element::getUpstreamEdge,
            py::arg("index") = 0)
        .def("getUpstreamEdgeCount", &mx::Element::getUpstreamEdgeCount)
        .def("getUpstreamElement", &mx::Element::getUpstreamElement,
            py::arg("index") = 0)
        .def("traverseInheritance", &mx::Element::traverseInheritance)
        .def("setSourceUri", &mx::Element::setSourceUri)
        .def("hasSourceUri", &mx::Element::hasSourceUri)
        .def("getSourceUri", &mx::Element::getSourceUri)
        .def("getActiveSourceUri", &mx::Element::getActiveSourceUri)
        .def("validate", [](const mx::Element& elem)
            {
                std::string message;
                bool res = elem.validate(&message);
                return std::pair<bool, std::string>(res, message);
            })
        .def("copyContentFrom", &mx::Element::copyContentFrom)
        .def("clearContent", &mx::Element::clearContent)
        .def("createValidChildName", &mx::Element::createValidChildName)
        .def("createStringResolver", &mx::Element::createStringResolver,
             py::arg("geom") = mx::EMPTY_STRING)
        .def("asString", &mx::Element::asString)
        .def("__str__", &mx::Element::asString)
        .def_readonly_static("NAME_ATTRIBUTE", &mx::Element::NAME_ATTRIBUTE)
        .def_readonly_static("FILE_PREFIX_ATTRIBUTE", &mx::Element::FILE_PREFIX_ATTRIBUTE)
        .def_readonly_static("GEOM_PREFIX_ATTRIBUTE", &mx::Element::GEOM_PREFIX_ATTRIBUTE)
        .def_readonly_static("COLOR_SPACE_ATTRIBUTE", &mx::Element::COLOR_SPACE_ATTRIBUTE)
        .def_readonly_static("INHERIT_ATTRIBUTE", &mx::Element::INHERIT_ATTRIBUTE)
        .def_readonly_static("NAMESPACE_ATTRIBUTE", &mx::Element::NAMESPACE_ATTRIBUTE)
        .def_readonly_static("DOC_ATTRIBUTE", &mx::Element::DOC_ATTRIBUTE)
        .def_readonly_static("XPOS_ATTRIBUTE", &mx::Element::XPOS_ATTRIBUTE)
        .def_readonly_static("YPOS_ATTRIBUTE", &mx::Element::YPOS_ATTRIBUTE)
        BIND_ELEMENT_FUNC_INSTANCE(Collection)
        BIND_ELEMENT_FUNC_INSTANCE(Document)
        BIND_ELEMENT_FUNC_INSTANCE(GeomInfo)
        BIND_ELEMENT_FUNC_INSTANCE(GeomProp)
        BIND_ELEMENT_FUNC_INSTANCE(Implementation)
        BIND_ELEMENT_FUNC_INSTANCE(Look)
        BIND_ELEMENT_FUNC_INSTANCE(MaterialAssign)
        BIND_ELEMENT_FUNC_INSTANCE(Node)
        BIND_ELEMENT_FUNC_INSTANCE(NodeDef)
        BIND_ELEMENT_FUNC_INSTANCE(NodeGraph)
        BIND_ELEMENT_FUNC_INSTANCE(Property)
        BIND_ELEMENT_FUNC_INSTANCE(PropertySet)
        BIND_ELEMENT_FUNC_INSTANCE(PropertySetAssign)
        BIND_ELEMENT_FUNC_INSTANCE(Token)
        BIND_ELEMENT_FUNC_INSTANCE(TypeDef)
        BIND_ELEMENT_FUNC_INSTANCE(Visibility);
    mod.attr("Element").doc() = R"docstring(
    The base class for MaterialX elements.

    An `Element` is a named object within a `Document`, which may possess any
    number of child elements and attributes.

    Inherited by: `TypedElement`, `GeomElement`, `Backdrop`, `Collection`,
    `CommentElement`, `GenericElement`, `Look`, `LookGroup`, `NewlineElement`,
    `PropertySet`, `TypeDef`, `Unit`, `UnitDef`, `UnitTypeDef`, `VariantSet`,
    and `VariantAssign`.

    :see: https://materialx.org/docs/api/class_element.html)docstring";

    py::class_<mx::TypedElement, mx::TypedElementPtr, mx::Element>(mod, "TypedElement")
        .def("setType", &mx::TypedElement::setType)
        .def("hasType", &mx::TypedElement::hasType)
        .def("getType", &mx::TypedElement::getType)
        .def("isColorType", &mx::TypedElement::isColorType)
        .def("isMultiOutputType", &mx::TypedElement::isMultiOutputType)
        .def("getTypeDef", &mx::TypedElement::getTypeDef)
        .def_readonly_static("TYPE_ATTRIBUTE", &mx::TypedElement::TYPE_ATTRIBUTE);
    mod.attr("TypedElement").doc() = R"docstring(
    The base class for typed elements.

    Inherited by: `InterfaceElement`, `ValueElement`, `AttributeDef`,
    `GeomPropDef`, `Member`, and `TargetDef`.

    :see: https://materialx.org/docs/api/class_typed_element.html)docstring";

    py::class_<mx::ValueElement, mx::ValueElementPtr, mx::TypedElement>(mod, "ValueElement")
        .def("setValueString", &mx::ValueElement::setValueString)
        .def("hasValueString", &mx::ValueElement::hasValueString)
        .def("getValueString", &mx::ValueElement::getValueString)
        .def("getResolvedValueString", &mx::ValueElement::getResolvedValueString,
            py::arg("resolver") = nullptr)
        .def("setInterfaceName", &mx::ValueElement::setInterfaceName)
        .def("hasInterfaceName", &mx::ValueElement::hasInterfaceName)
        .def("getInterfaceName", &mx::ValueElement::getInterfaceName)
        .def("setImplementationName", &mx::ValueElement::setImplementationName)
        .def("hasImplementationName", &mx::ValueElement::hasImplementationName)
        .def("getImplementationName", &mx::ValueElement::getImplementationName)
        .def("_getValue", &mx::ValueElement::getValue)
        .def("_getDefaultValue", &mx::ValueElement::getDefaultValue)
        .def("setUnit", &mx::ValueElement::setUnit)
        .def("hasUnit", &mx::ValueElement::hasUnit)
        .def("getUnit", &mx::ValueElement::getUnit)
        .def("getActiveUnit", &mx::ValueElement::getActiveUnit)
        .def("setUnitType", &mx::ValueElement::setUnitType)
        .def("hasUnitType", &mx::ValueElement::hasUnitType)
        .def("getUnitType", &mx::ValueElement::getUnitType)
        .def("getIsUniform", &mx::ValueElement::getIsUniform)
        .def("setIsUniform", &mx::ValueElement::setIsUniform)
        .def_readonly_static("VALUE_ATTRIBUTE", &mx::ValueElement::VALUE_ATTRIBUTE)
        .def_readonly_static("INTERFACE_NAME_ATTRIBUTE", &mx::ValueElement::INTERFACE_NAME_ATTRIBUTE)
        .def_readonly_static("IMPLEMENTATION_NAME_ATTRIBUTE", &mx::ValueElement::IMPLEMENTATION_NAME_ATTRIBUTE)
        .def_readonly_static("IMPLEMENTATION_TYPE_ATTRIBUTE", &mx::ValueElement::IMPLEMENTATION_TYPE_ATTRIBUTE)
        .def_readonly_static("ENUM_ATTRIBUTE", &mx::ValueElement::ENUM_ATTRIBUTE)
        .def_readonly_static("ENUM_VALUES_ATTRIBUTE", &mx::ValueElement::ENUM_VALUES_ATTRIBUTE)
        .def_readonly_static("UNIT_ATTRIBUTE", &mx::ValueElement::UNIT_ATTRIBUTE)
        .def_readonly_static("UI_NAME_ATTRIBUTE", &mx::ValueElement::UI_NAME_ATTRIBUTE)
        .def_readonly_static("UI_FOLDER_ATTRIBUTE", &mx::ValueElement::UI_FOLDER_ATTRIBUTE)
        .def_readonly_static("UI_MIN_ATTRIBUTE", &mx::ValueElement::UI_MIN_ATTRIBUTE)
        .def_readonly_static("UI_MAX_ATTRIBUTE", &mx::ValueElement::UI_MAX_ATTRIBUTE)
        .def_readonly_static("UI_SOFT_MIN_ATTRIBUTE", &mx::ValueElement::UI_SOFT_MIN_ATTRIBUTE)
        .def_readonly_static("UI_SOFT_MAX_ATTRIBUTE", &mx::ValueElement::UI_SOFT_MAX_ATTRIBUTE)
        .def_readonly_static("UI_STEP_ATTRIBUTE", &mx::ValueElement::UI_STEP_ATTRIBUTE)
        .def_readonly_static("UI_ADVANCED_ATTRIBUTE", &mx::ValueElement::UI_ADVANCED_ATTRIBUTE)

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
    mod.attr("ValueElement").doc() = R"docstring(
    The base class for elements that support typed values.

    Inherited by: `PortElement`, `GeomProp`, `Property`, `PropertyAssign`, and
    `Token`.

    :see: https://materialx.org/docs/api/class_value_element.html)docstring";

    py::class_<mx::Token, mx::TokenPtr, mx::ValueElement>(mod, "Token")
        .def_readonly_static("CATEGORY", &mx::Token::CATEGORY);
    mod.attr("Token").doc() = R"docstring(
    A token element representing a string value.

    Token elements are used to define input and output values for string
    substitutions in image filenames.

    :see: https://materialx.org/docs/api/class_token.html)docstring";

    py::class_<mx::CommentElement, mx::CommentElementPtr, mx::Element>(mod, "CommentElement")
        .def_readonly_static("CATEGORY", &mx::CommentElement::CATEGORY);
    mod.attr("CommentElement").doc() = R"docstring(
    An element representing a block of descriptive text within a `Document`,
    which will be stored as a comment when the document is written out.

    The comment text may be accessed with the methods `Element.getDocString()`
    and `Element.setDocString()`.

    :see: https://materialx.org/docs/api/class_comment_element.html)docstring";

    py::class_<mx::NewlineElement, mx::NewlineElementPtr, mx::Element>(mod, "NewlineElement")
        .def_readonly_static("CATEGORY", &mx::NewlineElement::CATEGORY);
    mod.attr("NewlineElement").doc() = R"docstring(
    An element representing a newline within a `Document`.

    :see: https://materialx.org/docs/api/class_newline_element.html)docstring";

    py::class_<mx::GenericElement, mx::GenericElementPtr, mx::Element>(mod, "GenericElement")
        .def_readonly_static("CATEGORY", &mx::GenericElement::CATEGORY);
    mod.attr("GenericElement").doc() = R"docstring(
    A generic element subclass, for instantiating elements with unrecognized
    categories.

    :see: https://materialx.org/docs/api/class_generic_element.html)docstring";

    py::class_<mx::ElementEquivalenceResult>(mod, "ElementEquivalenceResult")
        .def_readonly_static("ATTRIBUTE", &mx::ElementEquivalenceResult::ATTRIBUTE)
        .def_readonly_static("ATTRIBUTE_NAMES", &mx::ElementEquivalenceResult::ATTRIBUTE_NAMES)
        .def_readonly_static("CHILD_COUNT", &mx::ElementEquivalenceResult::CHILD_COUNT)
        .def_readonly_static("CHILD_NAME", &mx::ElementEquivalenceResult::CHILD_NAME)
        .def_readonly_static("NAME", &mx::ElementEquivalenceResult::NAME)
        .def_readonly_static("CATEGORY", &mx::ElementEquivalenceResult::CATEGORY)
        .def_readwrite("path1", &mx::ElementEquivalenceResult::path1)
        .def_readwrite("path2", &mx::ElementEquivalenceResult::path2)
        .def_readwrite("differenceType", &mx::ElementEquivalenceResult::differenceType)
        .def_readwrite("attributeName", &mx::ElementEquivalenceResult::attributeName);
    mod.attr("ElementEquivalenceResult").doc() = R"docstring(
    A comparison result for the functional equivalence of two elements.

    :see: https://materialx.org/docs/api/class_element_equivalence_result.html)docstring";

    py::class_<mx::ElementEquivalenceOptions>(mod, "ElementEquivalenceOptions")
        .def_readwrite("format", &mx::ElementEquivalenceOptions::format)
        .def_readwrite("precision", &mx::ElementEquivalenceOptions::precision)
        .def_readwrite("skipAttributes", &mx::ElementEquivalenceOptions::skipAttributes)
        .def_readwrite("skipValueComparisons", &mx::ElementEquivalenceOptions::skipValueComparisons)
        .def(py::init<>());
    mod.attr("ElementEquivalenceOptions").doc() = R"docstring(
    A set of options for comparing the functional equivalence of elements.

    :see: https://materialx.org/docs/api/class_element_equivalence_options.html)docstring";

    py::class_<mx::StringResolver, mx::StringResolverPtr>(mod, "StringResolver")
        .def("setFilePrefix", &mx::StringResolver::setFilePrefix)
        .def("getFilePrefix", &mx::StringResolver::getFilePrefix)
        .def("setGeomPrefix", &mx::StringResolver::setGeomPrefix)
        .def("getGeomPrefix", &mx::StringResolver::getGeomPrefix)
        .def("setUdimString", &mx::StringResolver::setUdimString)
        .def("setUvTileString", &mx::StringResolver::setUvTileString)
        .def("setFilenameSubstitution", &mx::StringResolver::setFilenameSubstitution)
        .def("getFilenameSubstitutions", &mx::StringResolver::getFilenameSubstitutions)
        .def("setGeomNameSubstitution", &mx::StringResolver::setGeomNameSubstitution)
        .def("getGeomNameSubstitutions", &mx::StringResolver::getGeomNameSubstitutions)
        .def("resolve", &mx::StringResolver::resolve);
    mod.attr("StringResolver").doc() = R"docstring(
    A helper object for applying string modifiers to data values in the context
    of a specific element and geometry.

    A `StringResolver` may be constructed through the `Element.createStringResolver()`
    method, which initializes it in the context of a specific `Element`, geometry,
    and material.

    Calling the `StringResolver.resolve()` method applies all modifiers to a
    particular string value.

    Methods such as `StringResolver.setFilePrefix()` may be used to edit the
    stored string modifiers before calling `StringResolver.resolve()`.

    :see: https://materialx.org/docs/api/class_string_resolver.html)docstring";

    py::class_<mx::ElementPredicate>(mod, "ElementPredicate");
    mod.attr("ElementPredicate").doc() = R"docstring(
    A function that takes an `Element` and returns a `bool`,
    to check whether some criteria has passed.)docstring";

    py::register_exception<mx::ExceptionOrphanedElement>(mod, "ExceptionOrphanedElement");
    mod.attr("ExceptionOrphanedElement").doc() = R"docstring(
    A type of exception that is raised when an `Element` is used after its
    owning `Document` has gone out of scope.

    :see: https://materialx.org/docs/api/class_exception_orphaned_element.html)docstring";

    mod.def("targetStringsMatch", &mx::targetStringsMatch);
    mod.def("prettyPrint", &mx::prettyPrint);
}
