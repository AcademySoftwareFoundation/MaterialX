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

class Element;
class TypedElement;
class ValueElement;
class Token;
class StringResolver;
class Document;
class Material;

/// A shared pointer to an Element
using ElementPtr = shared_ptr<Element>;
/// A shared pointer to a const Element
using ConstElementPtr = shared_ptr<const Element>;

/// A shared pointer to a TypedElement
using TypedElementPtr = shared_ptr<TypedElement>;
/// A shared pointer to a const TypedElement
using ConstTypedElementPtr = shared_ptr<const TypedElement>;

/// A shared pointer to a ValueElement
using ValueElementPtr = shared_ptr<ValueElement>;
/// A shared pointer to a const ValueElement
using ConstValueElementPtr = shared_ptr<const ValueElement>;

/// A shared pointer to a Token
using TokenPtr = shared_ptr<Token>;
/// A shared pointer to a const Token
using ConstTokenPtr = shared_ptr<const Token>;

/// A shared pointer to a StringResolver
using StringResolverPtr = shared_ptr<StringResolver>;

/// A hash map from strings to elements
using ElementMap = std::unordered_map<string, ElementPtr>;

/// A standard function taking an ElementPtr and returning a boolean.
using ElementPredicate = std::function<bool(ElementPtr)>;

/// @class Element
/// The base class for MaterialX elements.
///
/// An Element is a named object within a Document, which may possess any
/// number of child elements and attributes.
class Element : public std::enable_shared_from_this<Element>
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
    using DocumentPtr = shared_ptr<Document>;
    using ConstDocumentPtr = shared_ptr<const Document>;
    using ConstMaterialPtr = shared_ptr<const Material>;

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

    /// Set the element's name string.  The name of a MaterialX element must be
    /// unique among all elements at the same scope.
    /// @throws Exception if an element at the same scope already possesses the
    ///    given name.
    void setName(const string& name);

    /// Return the element's name string.
    const string& getName() const
    {
        return _name;
    }

    /// Return the element's hierarchical name path, relative to the root
    /// document.  The name of each ancestor will be prepended in turn,
    /// separated by forward slashes.
    /// @param relativeTo If a valid ancestor element is specified, then
    ///    the returned path will be relative to this ancestor.
    string getNamePath(ConstElementPtr relativeTo = nullptr) const;

    /// Return the element specified by the given hierarchical name path,
    /// relative to the current element.  If the name path is empty then the
    /// current element is returned.  If no element is found at the given path,
    /// then an empty shared pointer is returned.
    /// @param namePath The relative name path of the specified element.
    ElementPtr getDescendant(const string& namePath);

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

    /// Return the file prefix string that is active at the scope of this
    /// element, taking all ancestor elements into account.
    const string& getActiveFilePrefix() const
    {
        for (ConstElementPtr elem = getSelf(); elem; elem = elem->getParent())
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

    /// Return true if the given element has a geom prefix string.
    bool hasGeomPrefix() const
    {
        return hasAttribute(GEOM_PREFIX_ATTRIBUTE);
    }

    /// Return the element's geom prefix string.
    const string& getGeomPrefix() const
    {
        return getAttribute(GEOM_PREFIX_ATTRIBUTE);
    }

    /// Return the geom prefix string that is active at the scope of this
    /// element, taking all ancestor elements into account.
    const string& getActiveGeomPrefix() const
    {
        for (ConstElementPtr elem = getSelf(); elem; elem = elem->getParent())
        {
            if (elem->hasGeomPrefix())
            {
                return elem->getGeomPrefix();
            }
        }
        return EMPTY_STRING;
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

    /// Return the color space string that is active at the scope of this
    /// element, taking all ancestor elements into account.
    const string& getActiveColorSpace() const
    {
        for (ConstElementPtr elem = getSelf(); elem; elem = elem->getParent())
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
    /// @name Inheritance
    /// @{

    /// Set the inherit string of this element.
    void setInheritString(const string& inherit)
    {
        setAttribute(INHERIT_ATTRIBUTE, inherit);
    }

    /// Return true if this element has an inherit string.
    bool hasInheritString() const
    {
        return hasAttribute(INHERIT_ATTRIBUTE);
    }

    /// Return the inherit string of this element.
    const string& getInheritString() const
    {
        return getAttribute(INHERIT_ATTRIBUTE);
    }

    /// Set the element that this one directly inherits from.
    void setInheritsFrom(ConstElementPtr super)
    {
        if (super)
        {
            setInheritString(super->getName());
        }
        else
        {
            removeAttribute(INHERIT_ATTRIBUTE);
        }
    }

    /// Return the element, if any, that this one directly inherits from.
    ElementPtr getInheritsFrom() const
    {
        return resolveRootNameReference<Element>(getInheritString());
    }

    /// Return true if this element has the given element as an inherited base,
    /// taking the full inheritance chain into account.
    bool hasInheritedBase(ConstElementPtr base) const;

    /// Return true if the inheritance chain for this element contains a cycle.
    bool hasInheritanceCycle() const;

    /// @}
    /// @name Namespace
    /// @{

    /// Set the namespace string of this element.
    void setNamespace(const string& space)
    {
        setAttribute(NAMESPACE_ATTRIBUTE, space);
    }

    /// Return true if this element has a namespace string.
    bool hasNamespace() const
    {
        return hasAttribute(NAMESPACE_ATTRIBUTE);
    }

    /// Return the namespace string of this element.
    const string& getNamespace() const
    {
        return getAttribute(NAMESPACE_ATTRIBUTE);
    }

    /// Return a qualified version of the given name, taking the namespace at the
    /// scope of this element into account.
    string getQualifiedName(const string& name) const
    {
        for (ConstElementPtr elem = getSelf(); elem; elem = elem->getParent())
        {
            if (elem->hasNamespace())
            {
                return elem->getNamespace() + NAME_PREFIX_SEPARATOR + name;
            }
        }
        return name;
    }

    /// @}
    /// @name Version
    /// @{

    /// Set the version string of this element.
    void setVersionString(const string& version)
    {
        setAttribute(VERSION_ATTRIBUTE, version);
    }

    /// Return true if this element has a version string.
    bool hasVersionString() const
    {
        return hasAttribute(VERSION_ATTRIBUTE);
    }

    /// Return the version string of this element.
    const string& getVersionString() const
    {
        return getAttribute(VERSION_ATTRIBUTE);
    }

    /// Return the major and minor versions as an integer pair.
    virtual std::pair<int, int> getVersionIntegers() const;

    /// @}
    /// @name Default Version
    /// @{

    /// Set the default version flag of this element.
    void setDefaultVersion(bool defaultVersion)
    {
        setTypedAttribute<bool>(DEFAULT_VERSION_ATTRIBUTE, defaultVersion);
    }

    /// Return the default version flag of this element.
    bool getDefaultVersion() const
    {
        return getTypedAttribute<bool>(DEFAULT_VERSION_ATTRIBUTE);
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
    /// @throws Exception if a child of this element already possesses the
    ///    given name.
    /// @return A shared pointer to the new child element.
    template<class T> shared_ptr<T> addChild(const string& name = EMPTY_STRING);

    /// Add a child element of the given category and name.
    /// @param category The category string of the new child element.
    ///     If the category string is recognized, then the correponding Element
    ///     subclass is generated; otherwise, a GenericElement is generated.
    /// @param name The name of the new child element.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @throws Exception if a child of this element already possesses the
    ///    given name.
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

    /// Return a vector of all child elements that are instances of the given type,
    /// optionally filtered by the given category string.  The returned vector
    /// maintains the order in which children were added.
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
    const StringVec& getAttributeNames() const
    {
        return _attributeOrder;
    }

    /// Set the value of an implicitly typed attribute.  Since an attribute
    /// stores no explicit type, the same type argument must be used in
    /// corresponding calls to getTypedAttribute.
    template<class T> void setTypedAttribute(const string& attrib, const T& data)
    {
        setAttribute(attrib, toValueString(data));
    }

    /// Return the the value of an implicitly typed attribute.  If the given
    /// attribute is not present, or cannot be converted to the given data
    /// type, then the zero value for the data type is returned.
    template<class T> const T getTypedAttribute(const string& attrib) const
    {
        try
        {
            return fromValueString<T>(getAttribute(attrib));
        }
        catch (ExceptionTypeError&)
        {
        }
        return {};
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
    /// for (ElementPtr elem : inputElem->traverseTree())
    /// {
    ///     cout << elem->asString() << endl;
    /// }
    /// @endcode
    /// Example usage with an explicit iterator:
    /// @code
    /// for (mx::TreeIterator it = inputElem->traverseTree().begin(); it != mx::TreeIterator::end(); ++it)
    /// {
    ///     mx::ElementPtr elem = it.getElement();
    ///     cout << elem->asString() << " at depth " << it.getElementDepth() << endl;
    /// }
    /// @endcode
    TreeIterator traverseTree() const;

    /// Traverse the dataflow graph from the given element to each of its
    /// upstream sources in depth-first order, using pre-order visitation.
    /// @param material An optional material element, whose data bindings will
    ///    be applied to the traversal.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    /// @return A GraphIterator object.
    /// @details Example usage with an implicit iterator:
    /// @code
    /// for (Edge edge : inputElem->traverseGraph())
    /// {
    ///     ElementPtr upElem = edge.getUpstreamElement();
    ///     ElementPtr downElem = edge.getDownstreamElement();
    ///     cout << upElem->asString() << " lies upstream from " << downElem->asString() << endl;
    /// }
    /// @endcode
    /// Example usage with an explicit iterator:
    /// @code
    /// for (mx::GraphIterator it = inputElem->traverseGraph().begin(); it != mx::GraphIterator::end(); ++it)
    /// {
    ///     mx::ElementPtr elem = it.getUpstreamElement();
    ///     cout << elem->asString() << " at depth " << it.getElementDepth() << endl;
    /// }
    /// @endcode
    /// @sa getUpstreamEdge
    /// @sa getUpstreamElement
    GraphIterator traverseGraph(ConstMaterialPtr material = nullptr) const;

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    /// @param material An optional material element, whose data bindings will
    ///    be applied to the query.
    /// @param index An optional index of the edge to be returned, where the
    ///    valid index range may be determined with getUpstreamEdgeCount.
    /// @return The upstream Edge, if valid, or an empty Edge object.
    virtual Edge getUpstreamEdge(ConstMaterialPtr material = nullptr,
                                 size_t index = 0) const;

    /// Return the number of queriable upstream edges for this element.
    virtual size_t getUpstreamEdgeCount() const
    {
        return 0;
    }

    /// Return the Element with the given index that lies directly upstream
    /// from this one in the dataflow graph.
    /// @param material An optional material element, whose data bindings will
    ///    be applied to the query.
    /// @param index An optional index of the element to be returned, where the
    ///    valid index range may be determined with getUpstreamEdgeCount.
    /// @return The upstream Element, if valid, or an empty ElementPtr.
    ElementPtr getUpstreamElement(ConstMaterialPtr material = nullptr,
                                  size_t index = 0) const;

    /// Traverse the inheritance chain from the given element to each element
    /// from which it inherits.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    /// @return An InheritanceIterator object.
    /// @details Example usage:
    /// @code
    /// ConstElementPtr derivedElem;
    /// for (ConstElementPtr elem : inputElem->traverseInheritance())
    /// {
    ///     if (derivedElem)
    ///         cout << derivedElem->asString() << " inherits from " << elem->asString() << endl;
    ///     derivedElem = elem;
    /// }
    /// @endcode
    InheritanceIterator traverseInheritance() const;

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

    /// Return the source URI that is active at the scope of this
    /// element, taking all ancestor elements into account.
    const string& getActiveSourceUri() const
    {
        for (ConstElementPtr elem = getSelf(); elem; elem = elem->getParent())
        {
            if (elem->hasSourceUri())
            {
                return elem->getSourceUri();
            }
        }
        return EMPTY_STRING;
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
    /// @param copyOptions An optional pointer to a CopyOptions object.
    ///    If provided, then the given options will affect the behavior of the
    ///    copy function.  Defaults to a null pointer.
    void copyContentFrom(ConstElementPtr source, const class CopyOptions* copyOptions = nullptr);

    /// Clear all attributes and descendants from this element.
    void clearContent();

    /// Using the input name as a starting point, modify it to create a valid,
    /// unique name for a child element.
    string createValidChildName(string name) const
    {
        name = createValidName(name);
        while (_childMap.count(name))
        {
            name = incrementName(name);
        }
        return name;
    }

    /// Construct a StringResolver at the scope of this element.  The returned
    /// object may be used to apply substring modifiers to data values in the
    /// context of a specific element and geometry.
    /// @param geom An optional geometry name, which will be used to select the
    ///    applicable set of geometric string substitutions.  By default, no
    ///    geometric string substitutions are applied.  If the universal geometry
    ///    name "/" is given, then all geometric string substitutions are applied,
    /// @return A shared pointer to a StringResolver.
    /// @todo The StringResolver returned by this method doesn't yet take
    ///    interface tokens into account.
    StringResolverPtr createStringResolver(const string& geom = EMPTY_STRING) const;

    /// Return a single-line description of this element, including its category,
    /// name, type, and value.
    string asString() const;

    /// @}

  protected:
    // Resolve a reference to a named element at the root scope of this document,
    // taking the namespace at the scope of this element into account.
    template<class T> shared_ptr<T> resolveRootNameReference(const string& name) const
    {
        ConstElementPtr root = getRoot();
        shared_ptr<T> child = root->getChildOfType<T>(getQualifiedName(name));
        return child ? child : root->getChildOfType<T>(name);
    }

    // Enforce a requirement within a validate method, updating the validation
    // state and optional output text if the requirement is not met.
    void validateRequire(bool expression, bool& res, string* message, string errorDesc) const;

  public:
    static const string NAME_ATTRIBUTE;
    static const string FILE_PREFIX_ATTRIBUTE;
    static const string GEOM_PREFIX_ATTRIBUTE;
    static const string COLOR_SPACE_ATTRIBUTE;
    static const string TARGET_ATTRIBUTE;
    static const string VERSION_ATTRIBUTE;
    static const string DEFAULT_VERSION_ATTRIBUTE;
    static const string INHERIT_ATTRIBUTE;
    static const string NAMESPACE_ATTRIBUTE;

  protected:
    virtual void registerChildElement(ElementPtr child);
    virtual void unregisterChildElement(ElementPtr child);

    // Return a non-const copy of our self pointer, for use in constructing
    // graph traversal objects that require non-const storage.
    ElementPtr getSelfNonConst() const
    {
        return std::const_pointer_cast<Element>(shared_from_this());
    }

  protected:
    string _category;
    string _name;
    string _sourceUri;

    ElementMap _childMap;
    vector<ElementPtr> _childOrder;

    StringMap _attributeMap;
    StringVec _attributeOrder;

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

  protected:
    using TypeDefPtr = shared_ptr<class TypeDef>;

  public:
    /// @}
    /// @name Type String

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

    /// Return true if the element is of multi-output type.
    bool isMultiOutputType() const
    {
        return getType() == MULTI_OUTPUT_TYPE_STRING;
    }

    /// @}
    /// @name TypeDef References
    /// @{

    /// Return the TypeDef declaring the type string of this element.  If no
    /// matching TypeDef is found, then an empty shared pointer is returned.
    TypeDefPtr getTypeDef() const;

    /// @}

public:
    static const string TYPE_ATTRIBUTE;
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

    /// @name Value String
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

    /// Return the resolved value string of an element, applying any string
    /// substitutions that are defined at the element's scope.
    /// @param resolver An optional string resolver, which will be used to
    ///    apply string substitutions.  By default, a new string resolver
    ///    will be created at this scope and applied to the return value.
    string getResolvedValueString(StringResolverPtr resolver = nullptr) const;

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
    /// @name Typed Value
    /// @{

    /// Set the typed value of an element.
    template<class T> void setValue(const T& value, const string& type = EMPTY_STRING)
    {
        setType(!type.empty() ? type : getTypeString<T>());
        setValueString(toValueString(value));
    }

    /// Return true if the element possesses a typed value.
    bool hasValue() const
    {
        return hasAttribute(VALUE_ATTRIBUTE);
    }

    /// Return the typed value of an element as a generic value object, which
    /// may be queried to access its data.
    ///
    /// @return A shared pointer to the typed value of this element, or an
    ///    empty shared pointer if no value is present.
    ValuePtr getValue() const
    {
        if (!hasValue())
            return ValuePtr();
        return Value::createValueFromStrings(getValueString(), getType());
    }

    /// @}
    /// @name Bound Value
    /// @{

    /// Return the value that is bound to this element within the context of a
    /// given material.  For example, a BindParam within the material will
    /// affect the value of its correponding Parameter, and a BindInput will
    /// affect the value of its corresponding Input.
    ///
    /// If this element is bound to an Output of a NodeGraph, rather than to a
    /// uniform value, then an empty shared pointer is returned.
    ///
    /// If no data binding is applied by the material, then the default value for
    /// this element is returned.
    ///
    /// @param material The material whose data bindings will be applied to
    ///    the evaluation.
    /// @return A shared pointer to a typed value, or an empty shared pointer if
    ///    no bound or default value was found.
    ValuePtr getBoundValue(ConstMaterialPtr material) const;

    /// Return the default value for this element, which will be used as its bound
    /// value when no external binding from a material is present.
    ///
    /// @return A shared pointer to a typed value, or an empty shared pointer if
    ///    no default value was found.
    ValuePtr getDefaultValue() const;

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

/// @class Token
/// A token element representing a string value.
///
/// Token elements are used to define input and output values for string
/// substitutions in image filenames.
class Token : public ValueElement
{
  public:
    Token(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~Token() { }

  public:
    static const string CATEGORY;
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

/// @class StringResolver
/// A helper object for applying string modifiers to data values in the context
/// of a specific element and geometry.
///
/// A StringResolver may be constructed through the Element::createStringResolver
/// method, which initializes it in the context of a specific element and
/// geometry.  The given element is used to select element-specific modifiers
/// such as filename and geomname prefixes, while the geometry is used to select
/// GeomAttr-based string substitutions.
///
/// Calling the StringResolver::resolve method applies all modifiers to a
/// particular string value.
///
/// Methods such as StringResolver::setFilePrefix may be used to edit the
/// stored string modifiers before calling StringResolver::resolve.
class StringResolver
{
  public:
    StringResolver() { }
    virtual ~StringResolver() { }

    /// @name File Prefix
    /// @{

    /// Set the file prefix for this context.
    void setFilePrefix(const string& filePrefix)
    {
        _filePrefix = filePrefix;
    }

    /// Return the file prefix for this context.
    const string& getFilePrefix() const
    {
        return _filePrefix;
    }

    /// @}
    /// @name Geom Prefix
    /// @{

    /// Set the geom prefix for this context.
    void setGeomPrefix(const string& geomPrefix)
    {
        _geomPrefix = geomPrefix;
    }

    /// Return the geom prefix for this context.
    const string& getGeomPrefix() const
    {
        return _geomPrefix;
    }

    /// @}
    /// @name Filename Substitutions
    /// @{

    /// Set the UDIM substring substitution for filename data values.
    /// This string will be used to replace the standard %UDIM token.
    void setUdimString(const string& udim);

    /// Set the UV-tile substring substitution for filename data values.
    /// This string will be used to replace the standard %UVTILE token.
    void setUvTileString(const string& uvTile);

    /// Set an arbitrary substring substitution for filename data values.
    void setFilenameSubstitution(const string& key, const string& value)
    {
        _filenameMap[key] = value;
    }

    /// Return the map of filename substring substitutions.
    const StringMap& getFilenameSubstitutions() const
    {
        return _filenameMap;
    }

    /// @}
    /// @name Geometry Name Substitutions
    /// @{

    /// Set an arbitrary substring substitution for geometry name data values.
    void setGeomNameSubstitution(const string& key, const string& value)
    {
        _geomNameMap[key] = value;
    }

    /// Return the map of geometry name substring substitutions.
    const StringMap& getGeomNameSubstitutions() const
    {
        return _geomNameMap;
    }

    /// @}
    /// @name Resolution
    /// @{

    /// Given an input string and type, apply all appropriate modifiers and
    /// return the resulting string.
    virtual string resolve(const string& str, const string& type) const;

    /// @}

  protected:
    string _filePrefix;
    string _geomPrefix;
    StringMap _filenameMap;
    StringMap _geomNameMap;
};

/// @class CopyOptions
/// A set of options for controlling the behavior of element copy operations.
class CopyOptions
{
  public:
    CopyOptions() :
        skipDuplicateElements(false)
    {
    }
    ~CopyOptions() { }

    /// If true, elements at the same scope with duplicate names will be skipped;
    /// otherwise, they will trigger an exception.  Defaults to false.
    bool skipDuplicateElements;
};

/// @class ExceptionOrphanedElement
/// An exception that is thrown when an ElementPtr is used after its owning
/// Document has gone out of scope.
class ExceptionOrphanedElement : public Exception
{
  public:
    using Exception::Exception;
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

/// Given two target strings, each containing a string array of target names,
/// return true if they have any targets in common.  An empty target string
/// matches all targets.
bool targetStringsMatch(const string& target1, const string& target2);

} // namespace MaterialX

#endif
