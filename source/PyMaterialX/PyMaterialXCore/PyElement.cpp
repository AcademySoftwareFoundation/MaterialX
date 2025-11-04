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
    py::class_<mx::Element, mx::ElementPtr>(mod, "Element", "The base class for MaterialX elements.\n\nAn Element is a named object within a Document, which may possess any number of child elements and attributes.")
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("isEquivalent", [](const mx::Element& elem, mx::ConstElementPtr& rhs, const mx::ElementEquivalenceOptions& options)
        {
            std::string message;
            bool res = elem.isEquivalent(rhs, options, &message);
            return std::pair<bool, std::string>(res, message);
        }, "Return true if the given element tree, including all descendents, is considered to be equivalent to this one based on the equivalence criteria provided.\n\nArgs:\n    rhs: Element to compare against\n    options: Equivalence criteria\n    message: Optional text description of differences\n\nReturns:\n    True if the elements are equivalent. False otherwise.")        
        .def("setCategory", &mx::Element::setCategory, "Set the element's category string.")
        .def("getCategory", &mx::Element::getCategory, "Return the element's category string.\n\nThe category of a MaterialX element represents its role within the document, with common examples being \"material\", \"nodegraph\", and \"image\".")
        .def("setName", &mx::Element::setName, "Set the element's name string.")
        .def("getName", &mx::Element::getName, "Return the ColorManagementSystem name.")
        .def("getNamePath", &mx::Element::getNamePath,
            py::arg("relativeTo") = nullptr, "Return the element's hierarchical name path, relative to the root document.\n\nArgs:\n    relativeTo: If a valid ancestor element is specified, then the returned path will be relative to this ancestor.")
        .def("getDescendant", &mx::Element::getDescendant, "Return the element specified by the given hierarchical name path, relative to the current element.\n\nArgs:\n    namePath: The relative name path of the specified element.")
        .def("setFilePrefix", &mx::Element::setFilePrefix, "Set the element's file prefix string.")
        .def("hasFilePrefix", &mx::Element::hasFilePrefix, "Return true if the given element has a file prefix string.")
        .def("getFilePrefix", &mx::Element::getFilePrefix, "Return the element's file prefix string.")
        .def("getActiveFilePrefix", &mx::Element::getActiveFilePrefix, "Return the file prefix string that is active at the scope of this element, taking all ancestor elements into account.")
        .def("setGeomPrefix", &mx::Element::setGeomPrefix, "Set the element's geom prefix string.")
        .def("hasGeomPrefix", &mx::Element::hasGeomPrefix, "Return true if the given element has a geom prefix string.")
        .def("getGeomPrefix", &mx::Element::getGeomPrefix, "Return the element's geom prefix string.")
        .def("getActiveGeomPrefix", &mx::Element::getActiveGeomPrefix, "Return the geom prefix string that is active at the scope of this element, taking all ancestor elements into account.")
        .def("setColorSpace", &mx::Element::setColorSpace, "Set the element's color space string.")
        .def("hasColorSpace", &mx::Element::hasColorSpace, "Return true if the given element has a color space string.")
        .def("getColorSpace", &mx::Element::getColorSpace, "Return the element's color space string.")
        .def("getActiveColorSpace", &mx::Element::getActiveColorSpace, "Return the color space string that is active at the scope of this element, taking all ancestor elements into account.")
        .def("setInheritString", &mx::Element::setInheritString, "Set the inherit string of this element.")
        .def("hasInheritString", &mx::Element::hasInheritString, "Return true if this element has an inherit string.")
        .def("getInheritString", &mx::Element::getInheritString, "Return the inherit string of this element.")
        .def("setInheritsFrom", &mx::Element::setInheritsFrom, "Set the element that this one directly inherits from.")
        .def("getInheritsFrom", &mx::Element::getInheritsFrom, "Return the element, if any, that this one directly inherits from.")
        .def("hasInheritedBase", &mx::Element::hasInheritedBase, "Return true if this element has the given element as an inherited base, taking the full inheritance chain into account.")
        .def("hasInheritanceCycle", &mx::Element::hasInheritanceCycle, "Return true if the inheritance chain for this element contains a cycle.")
        .def("setNamespace", &mx::Element::setNamespace, "Set the namespace string of this element.")
        .def("hasNamespace", &mx::Element::hasNamespace, "Return true if this element has a namespace string.")
        .def("getNamespace", &mx::Element::getNamespace, "Return the namespace string of this element.")
        .def("getQualifiedName", &mx::Element::getQualifiedName, "Return a qualified version of the given name, taking the namespace at the scope of this element into account.")
        .def("setDocString", &mx::Element::setDocString, "Set the documentation string of this element.")
        .def("getDocString", &mx::Element::getDocString, "Return the documentation string of this element.")
        .def("addChildOfCategory", &mx::Element::addChildOfCategory,
            py::arg("category"), py::arg("name") = mx::EMPTY_STRING, "Add a child element of the given category and name.\n\nArgs:\n    category: The category string of the new child element. If the category string is recognized, then the corresponding Element subclass is generated; otherwise, a GenericElement is generated.\n    name: The name of the new child element. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new child element.")
        .def("changeChildCategory", &mx::Element::changeChildCategory, "Change the category of the given child element.\n\nArgs:\n    child: The child element that will be modified.\n    category: The new category string for the child element.\n\nReturns:\n    A shared pointer to a new child element, containing the contents of the original child but with a new category and subclass.")
        .def("_getChild", &mx::Element::getChild, "Return the child element, if any, with the given name.")
        .def("getChildren", &mx::Element::getChildren, "Return a constant vector of all child elements.\n\nThe returned vector maintains the order in which children were added.")
        .def("setChildIndex", &mx::Element::setChildIndex, "Set the index of the child, if any, with the given name.\n\nIf the given index is out of bounds, then an exception is thrown.")
        .def("getChildIndex", &mx::Element::getChildIndex, "Return the index of the child, if any, with the given name.\n\nIf no child with the given name is found, then -1 is returned.")
        .def("removeChild", &mx::Element::removeChild, "Remove the child element, if any, with the given name.")
        .def("setAttribute", &mx::Element::setAttribute, "Set the value string of the given attribute.")
        .def("hasAttribute", &mx::Element::hasAttribute, "Return true if the given attribute is present.")
        .def("getAttribute", &mx::Element::getAttribute, "Return the value string of the given attribute.\n\nIf the given attribute is not present, then an empty string is returned.")
        .def("getAttributeNames", &mx::Element::getAttributeNames, "Return a vector of stored attribute names, in the order they were set.")
        .def("removeAttribute", &mx::Element::removeAttribute, "Remove the given attribute, if present.")
        .def("getSelf", static_cast<mx::ElementPtr (mx::Element::*)()>(&mx::Element::getSelf), "Return our self pointer.")
        .def("getParent", static_cast<mx::ElementPtr(mx::Element::*)()>(&mx::Element::getParent), "Return our parent element.")
        .def("getRoot", static_cast<mx::ElementPtr(mx::Element::*)()>(&mx::Element::getRoot), "Return the root element of our tree.")
        .def("getDocument", static_cast<mx::DocumentPtr(mx::Element::*)()>(&mx::Element::getDocument), "Return the root document of our tree.")
        .def("traverseTree", &mx::Element::traverseTree, "Traverse the tree from the given element to each of its descendants in depth-first order, using pre-order visitation.\n\nReturns:\n    A TreeIterator object.")
        .def("traverseGraph", &mx::Element::traverseGraph, "Traverse the dataflow graph from the given element to each of its upstream sources in depth-first order, using pre-order visitation.\n\nReturns:\n    A GraphIterator object.")
        .def("getUpstreamEdge", &mx::Element::getUpstreamEdge,
            py::arg("index") = 0, "Return the Edge with the given index that lies directly upstream from this element in the dataflow graph.\n\nArgs:\n    index: An optional index of the edge to be returned, where the valid index range may be determined with getUpstreamEdgeCount.\n\nReturns:\n    The upstream Edge, if valid, or an empty Edge object.")
        .def("getUpstreamEdgeCount", &mx::Element::getUpstreamEdgeCount, "Return the number of queryable upstream edges for this element.")
        .def("getUpstreamElement", &mx::Element::getUpstreamElement,
            py::arg("index") = 0, "Return the upstream element of the edge.")
        .def("traverseInheritance", &mx::Element::traverseInheritance, "Traverse the inheritance chain from the given element to each element from which it inherits.\n\nReturns:\n    An InheritanceIterator object.")
        .def("setSourceUri", &mx::Element::setSourceUri, "Set the element's source URI.\n\nArgs:\n    sourceUri: A URI string representing the resource from which this element originates. This string may be used by serialization and deserialization routines to maintain hierarchies of include references.")
        .def("hasSourceUri", &mx::Element::hasSourceUri, "Return true if this element has a source URI.")
        .def("getSourceUri", &mx::Element::getSourceUri, "Return the element's source URI.")
        .def("getActiveSourceUri", &mx::Element::getActiveSourceUri, "Return the source URI that is active at the scope of this element, taking all ancestor elements into account.")
        .def("validate", [](const mx::Element& elem)
            {
                std::string message;
                bool res = elem.validate(&message);
                return std::pair<bool, std::string>(res, message);
            }, "Validate that the given element tree, including all descendants, is consistent with the MaterialX specification.")
        .def("copyContentFrom", &mx::Element::copyContentFrom, "Copy all attributes and descendants from the given element to this one.\n\nArgs:\n    source: The element from which content is copied.")
        .def("clearContent", &mx::Element::clearContent, "Clear all attributes and descendants from this element.")
        .def("createValidChildName", &mx::Element::createValidChildName, "Using the input name as a starting point, modify it to create a valid, unique name for a child element.")
        .def("createStringResolver", &mx::Element::createStringResolver,
             py::arg("geom") = mx::EMPTY_STRING, "Construct a StringResolver at the scope of this element.\n\nArgs:\n    geom: An optional geometry name, which will be used to select the applicable set of geometry token substitutions. By default, no geometry token substitutions are applied. If the universal geometry name \"/\" is given, then all geometry token substitutions are applied,\n\nReturns:\n    A shared pointer to a StringResolver.")
        .def("asString", &mx::Element::asString, "Return a single-line description of this element, including its category, name, and attributes.")
        .def("__str__", &mx::Element::asString, "Return a single-line description of this element, including its category, name, and attributes.")
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

    py::class_<mx::TypedElement, mx::TypedElementPtr, mx::Element>(mod, "TypedElement", "The base class for typed elements.")
        .def("setType", &mx::TypedElement::setType, "Set the data type for this port.")
        .def("hasType", &mx::TypedElement::hasType, "Return true if the given element has a type string.")
        .def("getType", &mx::TypedElement::getType, "Get stream attribute name.")
        .def("isColorType", &mx::TypedElement::isColorType, "Return true if the element is of color type.")
        .def("isMultiOutputType", &mx::TypedElement::isMultiOutputType, "Return true if the element is of multi-output type.")
        .def("getTypeDef", &mx::TypedElement::getTypeDef, "Return the TypeDef, if any, with the given name.")
        .def_readonly_static("TYPE_ATTRIBUTE", &mx::TypedElement::TYPE_ATTRIBUTE);

    py::class_<mx::ValueElement, mx::ValueElementPtr, mx::TypedElement>(mod, "ValueElement", "The base class for elements that support typed values.")
        .def("setValueString", &mx::ValueElement::setValueString, "Set the value string of an element.")
        .def("hasValueString", &mx::ValueElement::hasValueString, "Return true if the given element has a value string.")
        .def("getValueString", &mx::ValueElement::getValueString, "Return value string.")
        .def("getResolvedValueString", &mx::ValueElement::getResolvedValueString,
            py::arg("resolver") = nullptr, "Return the resolved value string of an element, applying any string substitutions that are defined at the element's scope.\n\nArgs:\n    resolver: An optional string resolver, which will be used to apply string substitutions. By default, a new string resolver will be created at this scope and applied to the return value.")
        .def("setInterfaceName", &mx::ValueElement::setInterfaceName, "Set the interface name of an element.")
        .def("hasInterfaceName", &mx::ValueElement::hasInterfaceName, "Return true if the given element has an interface name.")
        .def("getInterfaceName", &mx::ValueElement::getInterfaceName, "Return the interface name of an element.")
        .def("setImplementationName", &mx::ValueElement::setImplementationName, "Set the implementation name of an element.")
        .def("hasImplementationName", &mx::ValueElement::hasImplementationName, "Return true if the given element has an implementation name.")
        .def("getImplementationName", &mx::ValueElement::getImplementationName, "Return the implementation name of an element.")
        .def("_getValue", &mx::ValueElement::getValue, "Returns a value formatted according to this type syntax.\n\nThe value is constructed from the given value object.")
        .def("_getDefaultValue", &mx::ValueElement::getDefaultValue)
        .def("setUnit", &mx::ValueElement::setUnit, "Set a unit type for the value on this port.")
        .def("hasUnit", &mx::ValueElement::hasUnit, "Return true if the given element has a unit string.")
        .def("getUnit", &mx::ValueElement::getUnit, "Return the unit type for the value on this port.")
        .def("getActiveUnit", &mx::ValueElement::getActiveUnit, "Return the unit defined by the associated NodeDef if this element is a child of a Node.")
        .def("setUnitType", &mx::ValueElement::setUnitType, "Set the element's unittype string.")
        .def("hasUnitType", &mx::ValueElement::hasUnitType, "Return true if the given element has a unittype string.")
        .def("getUnitType", &mx::ValueElement::getUnitType, "Return the unit type string.")
        .def("getIsUniform", &mx::ValueElement::getIsUniform, "The the uniform attribute flag for this element.")
        .def("setIsUniform", &mx::ValueElement::setIsUniform, "Set the uniform attribute flag on this element.")
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

    py::class_<mx::Token, mx::TokenPtr, mx::ValueElement>(mod, "Token", "A token element representing a string value.\n\nToken elements are used to define input and output values for string substitutions in image filenames.")
        .def_readonly_static("CATEGORY", &mx::Token::CATEGORY);

    py::class_<mx::CommentElement, mx::CommentElementPtr, mx::Element>(mod, "CommentElement", "An element representing a block of descriptive text within a document, which will be stored a comment when the document is written out.\n\nThe comment text may be accessed with the methods Element::setDocString and Element::getDocString.")
        .def_readonly_static("CATEGORY", &mx::CommentElement::CATEGORY);

    py::class_<mx::NewlineElement, mx::NewlineElementPtr, mx::Element>(mod, "NewlineElement", "An element representing a newline within a document.")
        .def_readonly_static("CATEGORY", &mx::NewlineElement::CATEGORY);

    py::class_<mx::GenericElement, mx::GenericElementPtr, mx::Element>(mod, "GenericElement", "A generic element subclass, for instantiating elements with unrecognized categories.")
        .def_readonly_static("CATEGORY", &mx::GenericElement::CATEGORY);

    py::class_<mx::ElementEquivalenceOptions>(mod, "ElementEquivalenceOptions", "A set of options for comparing the functional equivalence of elements.")
        .def_readwrite("performValueComparisons", &mx::ElementEquivalenceOptions::performValueComparisons)
        .def_readwrite("floatFormat", &mx::ElementEquivalenceOptions::floatFormat)
        .def_readwrite("floatPrecision", &mx::ElementEquivalenceOptions::floatPrecision)
        .def_readwrite("attributeExclusionList", &mx::ElementEquivalenceOptions::attributeExclusionList)
        .def(py::init<>());

    py::class_<mx::StringResolver, mx::StringResolverPtr>(mod, "StringResolver", "A helper object for applying string modifiers to data values in the context of a specific element and geometry.\n\nA StringResolver may be constructed through the Element::createStringResolver method, which initializes it in the context of a specific element, geometry, and material.\n\nCalling the StringResolver::resolve method applies all modifiers to a particular string value.\n\nMethods such as StringResolver::setFilePrefix may be used to edit the stored string modifiers before calling StringResolver::resolve.")
        .def("setFilePrefix", &mx::StringResolver::setFilePrefix, "Set the element's file prefix string.")
        .def("getFilePrefix", &mx::StringResolver::getFilePrefix, "Return the element's file prefix string.")
        .def("setGeomPrefix", &mx::StringResolver::setGeomPrefix, "Set the element's geom prefix string.")
        .def("getGeomPrefix", &mx::StringResolver::getGeomPrefix, "Return the element's geom prefix string.")
        .def("setUdimString", &mx::StringResolver::setUdimString, "Set the UDIM substring substitution for filename data values.\n\nThis string will be used to replace the standard <UDIM> token.")
        .def("setUvTileString", &mx::StringResolver::setUvTileString, "Set the UV-tile substring substitution for filename data values.\n\nThis string will be used to replace the standard <UVTILE> token.")
        .def("setFilenameSubstitution", &mx::StringResolver::setFilenameSubstitution, "Set an arbitrary substring substitution for filename data values.")
        .def("getFilenameSubstitutions", &mx::StringResolver::getFilenameSubstitutions, "Return the map of filename substring substitutions.")
        .def("setGeomNameSubstitution", &mx::StringResolver::setGeomNameSubstitution, "Set an arbitrary substring substitution for geometry name data values.")
        .def("getGeomNameSubstitutions", &mx::StringResolver::getGeomNameSubstitutions, "Return the map of geometry name substring substitutions.")
        .def("resolve", &mx::StringResolver::resolve, "Given an input string and type, apply all appropriate modifiers and return the resulting string.");

    py::class_<mx::ElementPredicate>(mod, "ElementPredicate");

    py::register_exception<mx::ExceptionOrphanedElement>(mod, "ExceptionOrphanedElement");

    mod.def("targetStringsMatch", &mx::targetStringsMatch);
    mod.def("prettyPrint", &mx::prettyPrint);
}
