//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
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
    py::class_<mx::CopyOptions>(mod, "CopyOptions")
        .def(py::init())
        .def_readwrite("skipDuplicateElements", &mx::CopyOptions::skipDuplicateElements);

    py::class_<mx::Element, mx::ElementPtr>(mod, "Element")
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("setCategory", &mx::Element::setCategory)
        .def("getCategory", &mx::Element::getCategory)
        .def("setName", &mx::Element::setName)
        .def("getName", &mx::Element::getName)
        .def("getNamePath", &mx::Element::getNamePath,
            py::arg("relativeTo") = nullptr)
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
        .def("setTarget", &mx::Element::setTarget)
        .def("hasTarget", &mx::Element::hasTarget)
        .def("getTarget", &mx::Element::getTarget)
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
        .def("setVersionString", &mx::Element::setVersionString)
        .def("hasVersionString", &mx::Element::hasVersionString)
        .def("getVersionString", &mx::Element::getVersionString)
        .def("getVersionIntegers", &mx::Element::getVersionIntegers)
        .def("setDefaultVersion", &mx::Element::setDefaultVersion)
        .def("getDefaultVersion", &mx::Element::getDefaultVersion)
        .def("addChildOfCategory", &mx::Element::addChildOfCategory)
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
        .def("traverseGraph", &mx::Element::traverseGraph,
            py::arg("material") = nullptr)
        .def("getUpstreamEdge", &mx::Element::getUpstreamEdge,
            py::arg("material") = nullptr, py::arg("index") = 0)
        .def("getUpstreamEdgeCount", &mx::Element::getUpstreamEdgeCount)
        .def("getUpstreamElement", &mx::Element::getUpstreamElement,
            py::arg("material") = nullptr, py::arg("index") = 0)
        .def("traverseInheritance", &mx::Element::traverseInheritance)
        .def("setSourceUri", &mx::Element::setSourceUri)
        .def("hasSourceUri", &mx::Element::hasSourceUri)
        .def("getSourceUri", &mx::Element::getSourceUri)
        .def("getActiveSourceUri", &mx::Element::getActiveSourceUri)
        .def("validate", [](mx::Element& elem)
            {
                std::string message;
                bool res = elem.validate(&message);
                return std::pair<bool, std::string>(res, message);
            })
        .def("copyContentFrom", &mx::Element::copyContentFrom,
            py::arg("source"), py::arg("copyOptions") = (const mx::CopyOptions*) nullptr)
        .def("clearContent", &mx::Element::clearContent)
        .def("createValidChildName", &mx::Element::createValidChildName)
        .def("createStringResolver", &mx::Element::createStringResolver,
             py::arg("geom") = mx::EMPTY_STRING, py::arg("material") = nullptr, py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("asString", &mx::Element::asString)
        .def("__str__", &mx::Element::asString)
        BIND_ELEMENT_FUNC_INSTANCE(BindParam)
        BIND_ELEMENT_FUNC_INSTANCE(BindInput)
        BIND_ELEMENT_FUNC_INSTANCE(BindToken)
        BIND_ELEMENT_FUNC_INSTANCE(Collection)
        BIND_ELEMENT_FUNC_INSTANCE(Document)
        BIND_ELEMENT_FUNC_INSTANCE(GeomAttr)
        BIND_ELEMENT_FUNC_INSTANCE(GeomInfo)
        BIND_ELEMENT_FUNC_INSTANCE(Implementation)
        BIND_ELEMENT_FUNC_INSTANCE(Look)
        BIND_ELEMENT_FUNC_INSTANCE(Material)
        BIND_ELEMENT_FUNC_INSTANCE(MaterialAssign)
        BIND_ELEMENT_FUNC_INSTANCE(Node)
        BIND_ELEMENT_FUNC_INSTANCE(NodeDef)
        BIND_ELEMENT_FUNC_INSTANCE(NodeGraph)
        BIND_ELEMENT_FUNC_INSTANCE(Parameter)
        BIND_ELEMENT_FUNC_INSTANCE(Property)
        BIND_ELEMENT_FUNC_INSTANCE(PropertySet)
        BIND_ELEMENT_FUNC_INSTANCE(PropertySetAssign)
        BIND_ELEMENT_FUNC_INSTANCE(ShaderRef)
        BIND_ELEMENT_FUNC_INSTANCE(Token)
        BIND_ELEMENT_FUNC_INSTANCE(TypeDef)
        BIND_ELEMENT_FUNC_INSTANCE(Visibility);

    py::class_<mx::TypedElement, mx::TypedElementPtr, mx::Element>(mod, "TypedElement")
        .def("setType", &mx::TypedElement::setType)
        .def("hasType", &mx::TypedElement::hasType)
        .def("getType", &mx::TypedElement::getType)
        .def("isMultiOutputType", &mx::TypedElement::isMultiOutputType)
        .def("getTypeDef", &mx::TypedElement::getTypeDef);

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
        .def("_getBoundValue", &mx::ValueElement::getBoundValue)
        .def("_getDefaultValue", &mx::ValueElement::getDefaultValue)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(integer, int)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(boolean, bool)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(float, float)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(color2, mx::Color2)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(color3, mx::Color3)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(color4, mx::Color4)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(matrix33, mx::Matrix33)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(matrix44, mx::Matrix44)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(string, std::string);

    py::class_<mx::Token, mx::TokenPtr, mx::ValueElement>(mod, "Token")
        .def_readonly_static("CATEGORY", &mx::Token::CATEGORY);

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

    py::class_<mx::ElementPredicate>(mod, "ElementPredicate");

    py::register_exception<mx::ExceptionOrphanedElement>(mod, "ExceptionOrphanedElement");

    mod.def("targetStringsMatch", &mx::targetStringsMatch);
}
