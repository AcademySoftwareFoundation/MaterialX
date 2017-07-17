//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ELEMENT_H
#define MATERIALX_ELEMENT_H

/// @file
/// Base and generic element classes

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Traversal.h>
#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

/// A shared pointer to an Element
using ElementPtr = shared_ptr<class Element>;
/// A shared pointer to a const Element
using ConstElementPtr = shared_ptr<const class Element>;

/// A shared pointer to a TypedElement
using TypedElementPtr = shared_ptr<class TypedElement>;
/// A shared pointer to a const TypedElement
using ConstTypedElementPtr = shared_ptr<const class TypedElement>;

/// A shared pointer to a ValueElement
using ValueElementPtr = shared_ptr<class ValueElement>;
/// A shared pointer to a const ValueElement
using ConstValueElementPtr = shared_ptr<const class ValueElement>;

/// A hash map from strings to elements
using ElementMap = std::unordered_map<string, ElementPtr>;

/// A standard function taking an ElementPtr and returning a boolean.
using ElementPredicate = std::function<bool(ElementPtr)>;

/// @class Element
/// The base class for MaterialX elements.
///
/// An Element is a named object within a Document, which may possess any
/// number of child elements and attributes.
class Element : public enable_shared_from_this<Element>
{
  protected:
    Element(ElementPtr parent, const string& category, const string& name) :
        _category(category),
        _name(name),
        _parent(parent),
        _root(parent ? parent->getRoot() : nullptr)
    {
    }
  public:
    virtual ~Element() { }

  protected:
    using DocumentPtr = shared_ptr<class Document>;
    using ConstDocumentPtr = shared_ptr<const class Document>;
    using MaterialPtr = shared_ptr<class Material>;

    template <class T> friend class ElementRegistry;

  public:
    /// Return true if the given element tree, including all descendants,
    /// is identical to this one.
    bool operator==(const Element& rhs) const;

    /// Return true if the given element tree, including all descendants,
    /// differs from this one.
    bool operator!=(const Element& rhs) const;

    /// @name Category
    /// @{

    /// Set the element's category string.
    void setCategory(const string& category)
    {
        _category = category;
    }

    /// Return the element's category string.  The category of a MaterialX
    /// element represents its role within the document, with common examples
    /// being "material", "nodegraph", and "image".
    const string& getCategory() const
    {
        return _category;
    }

    /// @}
    /// @name Name
    /// @{

    /// Return the element's name string.  The name of a MaterialX element
    /// must be unique among all elements at the same scope, and cannot be
    /// modified after the element is created.
    /// @todo The MaterialX notion of namespaces is not yet supported.
    const string& getName() const
    {
        return _name;
    }

    /// Return the element's hierarchical name path, relative to the root
    /// document.  The name of each ancestor will be prepended in turn,
    /// separated by forward slashes.
    /// @param relativeTo If a valid ancestor element is specified, then
    ///    the returned path will be relative to this ancestor.
    string getNamePath(ConstElementPtr relativeTo = ConstElementPtr()) const;

    /// @}
    /// @name File Prefix
    /// @{

    /// Set the element's file prefix string.
    void setFilePrefix(const string& prefix)
    {
        setAttribute(FILE_PREFIX_ATTRIBUTE, prefix);
    }

    /// Return true if the given element has a file prefix string.
    bool hasFilePrefix() const
    {
        return hasAttribute(FILE_PREFIX_ATTRIBUTE);
    }

    /// Return the element's file prefix string.
    const string& getFilePrefix() const
    {
        return getAttribute(FILE_PREFIX_ATTRIBUTE);
    }

    /// Return the active file prefix string at the scope of this element.
    const string& getActiveFilePrefix() const
    {
        for (ConstElementPtr elem : traverseAncestors())
        {
            if (elem->hasFilePrefix())
            {
                return elem->getFilePrefix();
            }
        }
        return EMPTY_STRING;
    }

    /// @}
    /// @name Geom Prefix
    /// @{

    /// Set the element's geom prefix string.
    void setGeomPrefix(const string& prefix)
    {
        setAttribute(GEOM_PREFIX_ATTRIBUTE, prefix);
    }

    /// Return the element's geom prefix string.
    const string& getGeomPrefix() const
    {
        return getAttribute(GEOM_PREFIX_ATTRIBUTE);
    }

    /// @}
    /// @name Color Space
    /// @{

    /// Set the element's color space string.
    void setColorSpace(const string& colorSpace)
    {
        setAttribute(COLOR_SPACE_ATTRIBUTE, colorSpace);
    }

    /// Return true if the given element has a color space string.
    bool hasColorSpace() const
    {
        return hasAttribute(COLOR_SPACE_ATTRIBUTE);
    }

    /// Return the element's color space string.
    const string& getColorSpace() const
    {
        return getAttribute(COLOR_SPACE_ATTRIBUTE);
    }

    /// Return the active color space string at the scope of this element.
    const string& getActiveColorSpace() const
    {
        for (ConstElementPtr elem : traverseAncestors())
        {
            if (elem->hasColorSpace())
            {
                return elem->getColorSpace();
            }
        }
        return EMPTY_STRING;
    }

    /// @}
    /// @name Target
    /// @{

    /// Set the element's target string.
    void setTarget(const string& target)
    {
        setAttribute(TARGET_ATTRIBUTE, target);
    }

    /// Return true if the given element has a target string.
    bool hasTarget() const
    {
        return hasAttribute(TARGET_ATTRIBUTE);
    }

    /// Return the element's target string.
    const string& getTarget() const
    {
        return getAttribute(TARGET_ATTRIBUTE);
    }

    /// @}
    /// @name Subclass
    /// @{

    /// Return true if this element belongs to the given subclass.
    /// If a category string is specified, then both subclass and category
    /// matches are required.
    template<class T> bool isA(const string& category = EMPTY_STRING) const
    {
        if (!asA<T>())
            return false;
        if (!category.empty() && getCategory() != category)
            return false;
        return true;
    }

    /// Dynamic cast to an instance of the given subclass.
    template<class T> shared_ptr<T> asA();

    /// Dynamic cast to a const instance of the given subclass.
    template<class T> shared_ptr<const T> asA() const;

    /// @}
    /// @name Child Elements
    /// @{

    /// Add a child element of the given subclass and name.
    /// @param name The name of the new child element.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new child element.
    template<class T> shared_ptr<T> addChild(const string& name = EMPTY_STRING);

    /// Add a child element of the given category and name.
    /// @param category The category string of the new child element.
    ///     If the category string is recognized, then the correponding Element
    ///     subclass is generated; otherwise, a GenericElement is generated.
    /// @param name The name of the new child element.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new child element.
    ElementPtr addChildOfCategory(const string& category,
                                  const string& name = EMPTY_STRING);

    /// Return the child element, if any, with the given name.
    ElementPtr getChild(const string& name) const
    {
        ElementMap::const_iterator it = _childMap.find(name);
        if (it == _childMap.end())
            return ElementPtr();
        else
            return it->second;
    }

    /// Rename a child element.
    /// Note that this will just rename the element, it will not update
    /// any named references to this element kept on other elements.
    /// @param name The name of the child element to rename.
    /// @param newName The new name of the child element.
    void renameChild(const string& name, const string& newName);

    /// Return the child element, if any, with the given name and subclass.
    /// If a child with the given name exists, but belongs to a different
    /// subclass, then an empty shared pointer is returned.
    template<class T> shared_ptr<T> getChildOfType(const string& name) const
    {
        ElementPtr child = getChild(name);
        return child ? child->asA<T>() : shared_ptr<T>();
    }

    /// Return a constant vector of all child elements.
    /// The returned vector maintains the order in which children were added.
    const vector<ElementPtr>& getChildren() const
    {
        return _childOrder;
    }

    /// Return a vector of all child elements that are instances of the given type.
    /// The returned vector maintains the order in which children were added.
    template<class T> vector< shared_ptr<T> > getChildrenOfType(const string& category = EMPTY_STRING) const
    {
        vector< shared_ptr<T> > children;
        for (ElementPtr child : _childOrder)
        {
            shared_ptr<T> instance = child->asA<T>();
            if (!instance)
                continue;
            if (!category.empty() && child->getCategory() != category)
                continue;
            children.push_back(instance);
        }
        return children;
    }

    /// Set the index of the child, if any, with the given name.
    /// If the given index is out of bounds, then an exception is thrown.
    void setChildIndex(const string& name, int index);

    /// Return the index of the child, if any, with the given name.
    /// If no child with the given name is found, then -1 is returned.
    int getChildIndex(const string& name) const;

    /// Remove the child element, if any, with the given name.
    void removeChild(const string& name);

    /// Remove the child element, if any, with the given name and subclass.
    /// If a child with the given name exists, but belongs to a different
    /// subclass, then this method has no effect.
    template<class T> void removeChildOfType(const string& name)
    {
        if (getChildOfType<T>(name))
            removeChild(name);
    }

    /// @}
    /// @name Attributes
    /// @{

    /// Set the value string of the given attribute.
    void setAttribute(const string& attrib, const string& value);

    /// Return true if the given attribute is present.
    bool hasAttribute(const string& attrib) const
    {
        return _attributeMap.count(attrib) != 0;
    }

    /// Return the value string of the given attribute.  If the given attribute
    /// is not present, then an empty string is returned.
    const string& getAttribute(const string& attrib) const
    {
        StringMap::const_iterator it = _attributeMap.find(attrib);
        if (it == _attributeMap.end())
            return EMPTY_STRING;
        else
            return it->second;
    }

    /// Return a vector of stored attribute names, in the order they were set.
    const vector<string>& getAttributeNames() const
    {
        return _attributeOrder;
    }

    /// Set the value of an implicitly typed attribute.  Since an attribute
    /// stores no explicit type, the same type argument must be used in
    /// corresponding calls to getTypedAttribute.
    template<class T> void setTypedAttribute(const string& attrib, const T& value)
    {
        setAttribute(attrib, Value::createValue(value)->getValueString());
    }

    /// Return the the value of an implicitly typed attribute.  If the given
    /// attribute is not present, or cannot be converted to the given data
    /// type, then the zero value for the given data type is returned.
    template<class T> const T getTypedAttribute(const string& attrib) const
    {
        ValuePtr value = Value::createValueFromStrings(getAttribute(attrib), getTypeString<T>());
        return value->asA<T>();
    }

    /// Remove the given attribute, if present.
    void removeAttribute(const string& attrib);

    /// @}
    /// @name Self And Ancestor Elements
    /// @{

    /// Return our self pointer.
    ElementPtr getSelf()
    {
        return shared_from_this();
    }

    /// Return our self pointer.
    ConstElementPtr getSelf() const
    {
        return shared_from_this();
    }

    /// Return our parent element.
    ElementPtr getParent()
    {
        return _parent.lock();
    }

    /// Return our parent element.
    ConstElementPtr getParent() const
    {
        return _parent.lock();
    }

    /// Return the root element of our tree.
    ElementPtr getRoot();

    /// Return the root element of our tree.
    ConstElementPtr getRoot() const;

    /// Return the root document of our tree.
    DocumentPtr getDocument()
    {
        return getRoot()->asA<Document>();
    }

    /// Return the root document of our tree.
    ConstDocumentPtr getDocument() const
    {
        return getRoot()->asA<Document>();
    }

    /// @}
    /// @name Traversal
    /// @{

    /// Traverse the tree from the given element to each of its descendants in
    /// depth-first order, using pre-order visitation.
    /// @return A TreeIterator object.
    /// @details Example usage with an implicit iterator:
    /// @code
    /// for (ElementPtr elem : doc->traverseTree())
    /// {
    ///     cout << elem->asString() << endl;
    /// }
    /// @endcode
    /// Example usage with an explicit iterator:
    /// @code
    /// for (mx::TreeIterator it = doc->traverseTree().begin(); it != mx::TreeIterator::end(); ++it)
    /// {
    ///     mx::ElementPtr elem = it.getElement();
    ///     cout << elem->asString() << " at depth " << it.getElementDepth() << endl;
    /// }
    /// @endcode
    TreeIterator traverseTree() const;

    /// Traverse the dataflow graph from the given element to each of its
    /// upstream sources in depth-first order, using pre-order visitation.
    /// @param material An optional material element, whose data bindings and
    ///    overrides will be applied to the traversal.
    /// @return A GraphIterator object.
    /// @details Example usage with an implicit iterator:
    /// @code
    /// for (Edge edge : doc->traverseGraph())
    /// {
    ///     cout << edge.getUpstreamElement()->asString() << endl;
    /// }
    /// @endcode
    /// Example usage with an explicit iterator:
    /// @code
    /// for (mx::GraphIterator it = doc->traverseGraph().begin(); it != mx::GraphIterator::end(); ++it)
    /// {
    ///     mx::ElementPtr elem = it.getUpstreamElement();
    ///     cout << elem->asString() << " at depth " << it.getElementDepth() << endl;
    /// }
    /// @endcode
    /// @todo This method doesn't yet support material inheritance for
    ///     its material argument.
    /// @sa getUpstreamEdge
    /// @sa getUpstreamElement
    GraphIterator traverseGraph(MaterialPtr material = MaterialPtr()) const;

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    /// @param material An optional material element, whose data bindings and
    ///    overrides will be applied to the query.
    /// @param index An optional index of the edge to be returned, where the
    ///    valid index range may be determined with getUpstreamEdgeCount.
    /// @return The upstream Edge, if valid, or an empty Edge object.
    virtual Edge getUpstreamEdge(MaterialPtr material = MaterialPtr(),
                                 size_t index = 0);

    /// Return the number of queriable upstream edges for this element.
    virtual size_t getUpstreamEdgeCount()
    {
        return 0;
    }

    /// Return the Element with the given index that lies directly upstream
    /// from this one in the dataflow graph.
    /// @param material An optional material element, whose data bindings and
    ///    overrides will be applied to the query.
    /// @param index An optional index of the element to be returned, where the
    ///    valid index range may be determined with getUpstreamEdgeCount.
    /// @return The upstream Element, if valid, or an empty ElementPtr.
    ElementPtr getUpstreamElement(MaterialPtr material = MaterialPtr(),
                                  size_t index = 0);

    /// Traverse the tree from the given element to each of its ancestors.
    /// @return An AncestorIterator object.
    /// @details Example usage:
    /// @code
    /// for (ConstElementPtr elem : doc->traverseAncestors())
    /// {
    ///     cout << elem->asString() << endl;
    /// }
    /// @endcode
    AncestorIterator traverseAncestors() const;

    /// @}
    /// @name Source URI
    /// @{

    /// Set the element's source URI.
    /// @param sourceUri A URI string representing the resource from which
    ///    this element originates.  This string may be used by serialization
    ///    and deserialization routines to maintain hierarchies of include
    ///    references.
    void setSourceUri(const string& sourceUri)
    {
        _sourceUri = sourceUri;
    }

    /// Return true if this element has a source URI.
    bool hasSourceUri() const
    {
        return !_sourceUri.empty();
    }

    /// Return the element's source URI.
    const string& getSourceUri() const
    {
        return _sourceUri;
    }

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    virtual bool validate(string* message = nullptr) const;

    /// @}
    /// @name Utility
    /// @{

    /// Copy all attributes and descendants from the given element to this one.
    /// @param source The element from which content is copied.
    /// @param sourceUris If true, then source URIs from the given element
    ///    and its descendants are also copied.  Defaults to false.
    void copyContentFrom(ConstElementPtr source, bool sourceUris = false);

    /// Clear all attributes and descendants from this element.
    void clearContent();

    /// Using the input name as a starting point, modify it to create a valid,
    /// unique name for a child element.
    string createValidChildName(string name)
    {
        name = createValidName(name);
        while (_childMap.count(name))
        {
            name = incrementName(name);
        }
        return name;
    }

    /// Return a single-line description of this element, including its category,
    /// name, type, and value.
    string asString() const;

    /// @}

  protected:
    // Enforce a requirement within a validate method, updating the validation
    // state and optional output text if the requirement is not met.
    void validateRequire(bool expression, bool& res, string* message, string errorDesc) const;

  public:
    static const string TYPE_ATTRIBUTE;
    static const string FILE_PREFIX_ATTRIBUTE;
    static const string GEOM_PREFIX_ATTRIBUTE;
    static const string COLOR_SPACE_ATTRIBUTE;
    static const string TARGET_ATTRIBUTE;

  protected:
    virtual void registerChildElement(ElementPtr child);
    virtual void unregisterChildElement(ElementPtr child);

  protected:
    string _category;
    string _name;
    string _sourceUri;

    ElementMap _childMap;
    vector<ElementPtr> _childOrder;

    StringMap _attributeMap;
    vector<string> _attributeOrder;

    weak_ptr<Element> _parent;
    weak_ptr<Element> _root;

  private:
    Element(const Element&) = delete;
    Element& operator=(const Element&) = delete;

    template <class T> static ElementPtr createElement(ElementPtr parent, const string& name)
    {
        return std::make_shared<T>(parent, name);
    }

  private:
    using CreatorFunction = ElementPtr (*)(ElementPtr, const string&);
    using CreatorMap = std::unordered_map<string, CreatorFunction>;

    static CreatorMap _creatorMap;
};

/// @class TypedElement
/// The base class for typed elements.
class TypedElement : public Element
{
  protected:
    TypedElement(ElementPtr parent, const string& category, const string& name) :
        Element(parent, category, name)
    {
    }
  public:
    virtual ~TypedElement() { }

    /// Set the element's type string.
    void setType(const string& type)
    {
        setAttribute(TYPE_ATTRIBUTE, type);
    }

    /// Return true if the given element has a type string.
    bool hasType() const
    {
        return hasAttribute(TYPE_ATTRIBUTE);
    }

    /// Return the element's type string.
    const string& getType() const
    {
        return getAttribute(TYPE_ATTRIBUTE);
    }
};

/// @class ValueElement
/// The base class for elements that support typed values.
class ValueElement : public TypedElement
{
  protected:
    ValueElement(ElementPtr parent, const string& category, const string& name) :
        TypedElement(parent, category, name)
    {
    }
  public:
    virtual ~ValueElement() { }

    /// @name Values
    /// @{

    /// Set the value string of an element.
    void setValueString(const string& value)
    {
        setAttribute(VALUE_ATTRIBUTE, value);
    }

    /// Return true if the given element has a value string.
    bool hasValueString() const
    {
        return hasAttribute(VALUE_ATTRIBUTE);
    }

    /// Get the value string of a element.
    const string& getValueString() const
    {
        return getAttribute(VALUE_ATTRIBUTE);
    }

    /// Return the resolved value string of an element, taking all active string
    /// substitutions into account.  Filename values, for example, will receive
    /// the fileprefix, if any, that is active at the scope of this element.
    string getResolvedValueString() const;

    /// Set the typed value of an element.
    template<class T> void setValue(const T& value, const string& type = EMPTY_STRING)
    {
        ValuePtr valuePtr = Value::createValue<T>(value);
        if (!type.empty())
            setType(type);
        else
            setType(valuePtr->getTypeString());
        setValueString(valuePtr->getValueString());
    }

    /// Return true if the element possesses a valid value, which may be
    /// converted to a value object through the getValue method.
    bool hasValue() const
    {
        return getValue() != nullptr;
    }

    /// Return the typed value of an element as a generic value object, which
    /// may be queried to access its data.  If this element does not possess
    /// a typed value, then a then a value object containing an empty string
    /// is returned.
    ValuePtr getValue() const
    {
        return Value::createValueFromStrings(getValueString(), getType());
    }

    /// @}
    /// @name Public Names
    /// @{

    /// Set the public name of an element.
    void setPublicName(const string& name)
    {
        setAttribute(PUBLIC_NAME_ATTRIBUTE, name);
    }

    /// Return true if the given element has a public name.
    bool hasPublicName() const
    {
        return hasAttribute(PUBLIC_NAME_ATTRIBUTE);
    }

    /// Return the public name of an element.
    const string& getPublicName() const
    {
        return getAttribute(PUBLIC_NAME_ATTRIBUTE);
    }

    /// @}
    /// @name Interface Names
    /// @{

    /// Set the interface name of an element.
    void setInterfaceName(const string& name)
    {
        setAttribute(INTERFACE_NAME_ATTRIBUTE, name);
    }

    /// Return true if the given element has an interface name.
    bool hasInterfaceName() const
    {
        return hasAttribute(INTERFACE_NAME_ATTRIBUTE);
    }

    /// Return the interface name of an element.
    const string& getInterfaceName() const
    {
        return getAttribute(INTERFACE_NAME_ATTRIBUTE);
    }

    /// @}
    /// @name Implementation Names
    /// @{

    /// Set the implementation name of an element.
    void setImplementationName(const string& name)
    {
        setAttribute(IMPLEMENTATION_NAME_ATTRIBUTE, name);
    }

    /// Return true if the given element has an implementation name.
    bool hasImplementationName() const
    {
        return hasAttribute(IMPLEMENTATION_NAME_ATTRIBUTE);
    }

    /// Return the implementation name of an element.
    const string& getImplementationName() const
    {
        return getAttribute(IMPLEMENTATION_NAME_ATTRIBUTE);
    }

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    bool validate(string* message = nullptr) const override;

    /// @}

  public:
    static const string VALUE_ATTRIBUTE;
    static const string PUBLIC_NAME_ATTRIBUTE;
    static const string INTERFACE_NAME_ATTRIBUTE;
    static const string IMPLEMENTATION_NAME_ATTRIBUTE;
};

/// @class GenericElement
/// A generic element subclass, for instantiating elements with unrecognized categories.
class GenericElement : public Element
{
  public:
    GenericElement(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~GenericElement() { }

  public:
    static const string CATEGORY;
};

/// @class @ExceptionOrphanedElement
/// An exception that is thrown when an ElementPtr is used after its owning
/// Document has gone out of scope.
class ExceptionOrphanedElement : public Exception
{
  public:
    ExceptionOrphanedElement(const string& msg) :
        Exception(msg)
    {
    }

    ExceptionOrphanedElement(const ExceptionOrphanedElement& e) :
        Exception(e)
    {
    }

    virtual ~ExceptionOrphanedElement() throw()
    {
    }
};

template<class T> shared_ptr<T> Element::addChild(const string& name)
{
    string childName = name;
    if (childName.empty())
    {
        childName = createValidChildName(T::CATEGORY + "1");
    }

    if (_childMap.count(childName))
        throw Exception("Child name is not unique: " + childName);

    shared_ptr<T> child = std::make_shared<T>(getSelf(), childName);
    registerChildElement(child);

    return child;
}

} // namespace MaterialX

#endif
