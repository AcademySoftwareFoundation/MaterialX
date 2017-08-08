//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Element.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Util.h>

namespace MaterialX
{

const string Element::TYPE_ATTRIBUTE = "type";
const string Element::FILE_PREFIX_ATTRIBUTE = "fileprefix";
const string Element::GEOM_PREFIX_ATTRIBUTE = "geomprefix";
const string Element::COLOR_SPACE_ATTRIBUTE = "colorspace";
const string Element::TARGET_ATTRIBUTE = "target";
const string ValueElement::VALUE_ATTRIBUTE = "value";
const string ValueElement::PUBLIC_NAME_ATTRIBUTE = "publicname";
const string ValueElement::INTERFACE_NAME_ATTRIBUTE = "interfacename";
const string ValueElement::IMPLEMENTATION_NAME_ATTRIBUTE = "implname";

Element::CreatorMap Element::_creatorMap;

//
// Element methods
//

bool Element::operator==(const Element& rhs) const
{
    if (getCategory() != rhs.getCategory() ||
        getName() != rhs.getName())
    {
        return false;
    }

    // Compare attributes.
    if (getAttributeNames() != rhs.getAttributeNames())
        return false;
    for (const string& attr : rhs.getAttributeNames())
    {
        if (getAttribute(attr) != rhs.getAttribute(attr))
            return false;
    }

    // Compare children.
    const vector<ElementPtr>& c1 = getChildren();
    const vector<ElementPtr>& c2 = rhs.getChildren();
    if (c1.size() != c2.size())
        return false;
    for (size_t i = 0; i < c1.size(); i++)
    {
        if (*c1[i] != *c2[i])
            return false;
    }
    return true;
}

bool Element::operator!=(const Element& rhs) const
{
    return !(*this == rhs);
}

string Element::getNamePath(ConstElementPtr relativeTo) const
{
    if (!relativeTo)
    {
        relativeTo = getDocument();
    }

    string res;
    for (ConstElementPtr elem : traverseAncestors())
    {
        if (elem == relativeTo)
        {
            break;
        }
        res = res.empty() ? elem->getName() : elem->getName() + NAME_PATH_SEPARATOR + res;
    }
    return res;
}

void Element::registerChildElement(ElementPtr child)
{
    DocumentPtr doc = getDocument();

    // Handle change notifications.
    ScopedUpdate update(doc);
    doc->onAddElement(getSelf(), child);

    _childMap[child->getName()] = child;
    _childOrder.push_back(child);
}

void Element::unregisterChildElement(ElementPtr child)
{
    DocumentPtr doc = getDocument();

    // Handle change notifications.
    ScopedUpdate update(doc);
    doc->onRemoveElement(getSelf(), child);

    _childMap.erase(child->getName());
    _childOrder.erase(
        std::find(_childOrder.begin(), _childOrder.end(), child));
}

int Element::getChildIndex(const string& name) const
{
    ElementPtr child = getChild(name);
    vector<ElementPtr>::const_iterator it = std::find(_childOrder.begin(), _childOrder.end(), child);
    if (it == _childOrder.end())
    {
        return -1;
    }
    return (int) std::distance(_childOrder.begin(), it);
}

void Element::setChildIndex(const string& name, int index)
{
    ElementPtr child = getChild(name);
    vector<ElementPtr>::iterator it = std::find(_childOrder.begin(), _childOrder.end(), child);
    if (it == _childOrder.end())
    {
        return;
    }

    if (index < 0 || index > (int) _childOrder.size())
    {
        throw Exception("Invalid child index");
    }

    _childOrder.erase(it);
    _childOrder.insert(_childOrder.begin() + (size_t) index, child);
}

void Element::removeChild(const string& name)
{
    ElementMap::iterator it = _childMap.find(name);
    if (it == _childMap.end())
    {
        return;
    }

    unregisterChildElement(it->second);
}

void Element::setAttribute(const string& attrib, const string& value)
{
    DocumentPtr doc = getDocument();

    // Handle change notifications.
    ScopedUpdate update(doc);
    doc->onSetAttribute(getSelf(), attrib, value);

    if (!_attributeMap.count(attrib))
    {
        _attributeOrder.push_back(attrib);
    }
    _attributeMap[attrib] = value;
}

void Element::removeAttribute(const string& attrib)
{
    StringMap::iterator it = _attributeMap.find(attrib);
    if (it != _attributeMap.end())
    {
        DocumentPtr doc = getDocument();

        // Handle change notifications.
        ScopedUpdate update(doc);
        doc->onRemoveAttribute(getSelf(), attrib);

        _attributeMap.erase(it);
        _attributeOrder.erase(
            std::find(_attributeOrder.begin(), _attributeOrder.end(), attrib));
    }
}

template<class T> shared_ptr<T> Element::asA()
{
    return std::dynamic_pointer_cast<T>(getSelf());
}

template<class T> shared_ptr<const T> Element::asA() const
{
    return std::dynamic_pointer_cast<const T>(getSelf());
}

ElementPtr Element::addChildOfCategory(const string& category,
                                       const string& name)
{
    if (_childMap.count(name))
    {
        throw Exception("Child name is not unique: " + name);
    }

    ElementPtr child;

    // Check for this category in the creator map.
    CreatorMap::iterator it = _creatorMap.find(category);
    if (it != _creatorMap.end())
    {
        child = it->second(getSelf(), name);
    }

    // Check for a node within a graph.
    if (!child && getCategory() == NodeGraph::CATEGORY)
    {
        child = createElement<Node>(getSelf(), name);
        child->setCategory(category);
    }

    // If no match was found, then create a generic element.
    if (!child)
    {
        child = createElement<GenericElement>(getSelf(), name);
        child->setCategory(category);
    }

    // Register the child.
    registerChildElement(child);

    return child;
}

ElementPtr Element::getRoot()
{
    ElementPtr root = _root.lock();
    if (!root)
    {
        throw ExceptionOrphanedElement("Requested root of orphaned element: " + asString());
    }
    return root;
}

ConstElementPtr Element::getRoot() const
{
    ElementPtr root = _root.lock();
    if (!root)
    {
        throw ExceptionOrphanedElement("Requested root of orphaned element: " + asString());
    }
    return root;
}

TreeIterator Element::traverseTree() const
{
    return TreeIterator(std::const_pointer_cast<Element>(getSelf()));
}

GraphIterator Element::traverseGraph(MaterialPtr material) const
{
    return GraphIterator(std::const_pointer_cast<Element>(getSelf()), material);
}

Edge Element::getUpstreamEdge(MaterialPtr, size_t)
{
    return NULL_EDGE;
}

ElementPtr Element::getUpstreamElement(MaterialPtr material, size_t index)
{
    return getUpstreamEdge(material, index).getUpstreamElement();
}

AncestorIterator Element::traverseAncestors() const
{
    return AncestorIterator(getSelf());
}

void Element::copyContentFrom(ConstElementPtr source, bool sourceUris)
{
    if (sourceUris)
    {
        _sourceUri = source->_sourceUri;
    }
    for (const string& attr : source->getAttributeNames())
    {
        setAttribute(attr, source->getAttribute(attr));
    }
    for (ElementPtr child : source->getChildren())
    {
        addChildOfCategory(child->getCategory(), child->getName())->copyContentFrom(child);
    }
}

void Element::clearContent()
{
    _sourceUri = EMPTY_STRING;
    vector<string> attributeNames = getAttributeNames();
    vector<ElementPtr> children = getChildren();
    for (const string& attr : attributeNames)
    {
        removeAttribute(attr);
    }
    for (ElementPtr child : children)
    {
        removeChild(child->getName());
    }
}

bool Element::validate(string* message) const
{
    bool res = true;
    validateRequire(isValidName(getName()), res, message, "Invalid element name");
    for (ElementPtr child : getChildren())
    {
        res = child->validate(message) && res;
    }
    return res;
}

string Element::asString() const
{
    string res = "<" + getCategory();
    if (getName() != EMPTY_STRING)
    {
        res += " name=\"" + getName() + "\"";
    }
    for (const string& attrName : getAttributeNames())
    {
        res += " " + attrName + "=\"" + getAttribute(attrName) + "\"";
    }
    res += ">";
    return res;
}

void Element::validateRequire(bool expression, bool& res, string* message, string errorDesc) const
{
    if (!expression)
    {
        res = false;
        if (message)
        {
            *message += errorDesc + ": " + asString() + "\n";
        }
    }
}

//
// ValueElement methods
//

string ValueElement::getResolvedValueString() const
{
    string value = getValueString();
    if (getType() == FILENAME_TYPE_STRING)
        value = getActiveFilePrefix() + value;
    return value;
}

bool ValueElement::validate(string* message) const
{
    bool res = true;
    if (hasType() && hasValueString())
    {
        validateRequire(hasValue(), res, message, "Invalid value");
    }
    return TypedElement::validate(message) && res;
}

//
// Element registry class
//

template <class T> class ElementRegistry
{
  public:
    ElementRegistry()
    {
        Element::_creatorMap[T::CATEGORY] = Element::createElement<T>;
    }
    ~ElementRegistry() { }
};

//
// Template instantiations
//

#define INSTANTIATE_SUBCLASS(T)                         \
template shared_ptr<T> Element::asA<T>();               \
template shared_ptr<const T> Element::asA<T>() const;

INSTANTIATE_SUBCLASS(Element)
INSTANTIATE_SUBCLASS(GeomElement)
INSTANTIATE_SUBCLASS(InterfaceElement)
INSTANTIATE_SUBCLASS(PortElement)
INSTANTIATE_SUBCLASS(TypedElement)
INSTANTIATE_SUBCLASS(ValueElement)

#define INSTANTIATE_CONCRETE_SUBCLASS(T, category)      \
const string T::CATEGORY(category);                     \
ElementRegistry<T> registry##T;                         \
INSTANTIATE_SUBCLASS(T)

INSTANTIATE_CONCRETE_SUBCLASS(BindParam, "bindparam")
INSTANTIATE_CONCRETE_SUBCLASS(BindInput, "bindinput")
INSTANTIATE_CONCRETE_SUBCLASS(Collection, "collection")
INSTANTIATE_CONCRETE_SUBCLASS(CollectionAdd, "collectionadd")
INSTANTIATE_CONCRETE_SUBCLASS(CollectionRemove, "collectionremove")
INSTANTIATE_CONCRETE_SUBCLASS(Document, "materialx")
INSTANTIATE_CONCRETE_SUBCLASS(GenericElement, "generic")
INSTANTIATE_CONCRETE_SUBCLASS(GeomAttr, "geomattr")
INSTANTIATE_CONCRETE_SUBCLASS(GeomInfo, "geominfo")
INSTANTIATE_CONCRETE_SUBCLASS(Implementation, "implementation")
INSTANTIATE_CONCRETE_SUBCLASS(Input, "input")
INSTANTIATE_CONCRETE_SUBCLASS(Look, "look")
INSTANTIATE_CONCRETE_SUBCLASS(LookInherit, "lookinherit")
INSTANTIATE_CONCRETE_SUBCLASS(Material, "material")
INSTANTIATE_CONCRETE_SUBCLASS(MaterialAssign, "materialassign")
INSTANTIATE_CONCRETE_SUBCLASS(MaterialInherit, "materialinherit")
INSTANTIATE_CONCRETE_SUBCLASS(Node, "node")
INSTANTIATE_CONCRETE_SUBCLASS(NodeDef, "nodedef")
INSTANTIATE_CONCRETE_SUBCLASS(NodeGraph, "nodegraph")
INSTANTIATE_CONCRETE_SUBCLASS(Override, "override")
INSTANTIATE_CONCRETE_SUBCLASS(Output, "output")
INSTANTIATE_CONCRETE_SUBCLASS(Parameter, "parameter")
INSTANTIATE_CONCRETE_SUBCLASS(Property, "property")
INSTANTIATE_CONCRETE_SUBCLASS(PropertyAssign, "propertyassign")
INSTANTIATE_CONCRETE_SUBCLASS(PropertySet, "propertyset")
INSTANTIATE_CONCRETE_SUBCLASS(PropertySetAssign, "propertysetassign")
INSTANTIATE_CONCRETE_SUBCLASS(ShaderRef, "shaderref")
INSTANTIATE_CONCRETE_SUBCLASS(TypeDef, "typedef")
INSTANTIATE_CONCRETE_SUBCLASS(Visibility, "visibility")

} // namespace MaterialX
