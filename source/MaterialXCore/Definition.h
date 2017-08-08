//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_DEFINITION_H
#define MATERIALX_DEFINITION_H

/// @file
/// Definition element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Interface.h>

namespace MaterialX
{

/// A shared pointer to a NodeDef
using NodeDefPtr = shared_ptr<class NodeDef>;
/// A shared pointer to a TypeDef
using TypeDefPtr = shared_ptr<class TypeDef>;
/// A shared pointer to an Implementation
using ImplementationPtr = shared_ptr<class Implementation>;

/// @class NodeDef
/// A node definition element within a Document.
///
/// A NodeDef provides the declaration of a node interface, which may then
/// be instantiated as a Node or a ShaderRef.
class NodeDef : public InterfaceElement
{
  public:
    NodeDef(ElementPtr parent, const string& name) :
        InterfaceElement(parent, CATEGORY, name)
    {
    }
    virtual ~NodeDef() { }

    using ShaderRefPtr = shared_ptr<class ShaderRef>;

    /// @name Node String
    /// @{

    /// Set the node string of the NodeDef.
    void setNode(const string& node)
    {
        setAttribute(NODE_ATTRIBUTE, node);
    }

    /// Return true if the given NodeDef has a node string.
    bool hasNode() const
    {
        return hasAttribute(NODE_ATTRIBUTE);
    }

    /// Return the node string of the NodeDef.
    const string& getNode() const
    {
        return getAttribute(NODE_ATTRIBUTE);
    }

    /// @}
    /// @name Connections
    /// @{

    /// Return all ShaderRef elements that instantiate this NodeDef.
    vector<ShaderRefPtr> getInstantiatingShaderRefs() const;

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    bool validate(string* message = nullptr) const override;

    /// @}

  public:
    static const string CATEGORY;
    static const string NODE_ATTRIBUTE;
};

/// @class TypeDef
/// A type definition element within a Document.
class TypeDef : public Element
{
  public:
    TypeDef(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~TypeDef() { }

  public:
    static const string CATEGORY;
};

/// @class Implementation
/// An implementation element within a Document.
///
/// An Implementation is used to associate external source code with a specific
/// NodeDef, providing a definition for the node that may eiher be universal or
/// restricted to a specific target.
class Implementation : public InterfaceElement
{
  public:
    Implementation(ElementPtr parent, const string& name) :
        InterfaceElement(parent, CATEGORY, name)
    {
    }
    virtual ~Implementation() { }

    /// @name NodeDef String
    /// @{

    /// Set the NodeDef string for the Implementation.
    void setNodeDef(const string& nodeDef)
    {
        setAttribute(NODE_DEF_ATTRIBUTE, nodeDef);
    }

    /// Return true if the given Implementation has a NodeDef string.
    bool hasNodeDef() const
    {
        return hasAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// Return the NodeDef string for the Implementation.
    const string& getNodeDef() const
    {
        return getAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// @}
    /// @name File String
    /// @{

    /// Set the file string for the Implementation.
    void setFile(const string& file)
    {
        setAttribute(FILE_ATTRIBUTE, file);
    }

    /// Return true if the given Implementation has a file string.
    bool hasFile() const
    {
        return hasAttribute(FILE_ATTRIBUTE);
    }

    /// Return the file string for the Implementation.
    const string& getFile() const
    {
        return getAttribute(FILE_ATTRIBUTE);
    }

    /// @}
    /// @name Function String
    /// @{

    /// Set the function string for the Implementation.
    void setFunction(const string& function)
    {
        setAttribute(FUNCTION_ATTRIBUTE, function);
    }

    /// Return true if the given Implementation has a function string.
    bool hasFunction() const
    {
        return hasAttribute(FUNCTION_ATTRIBUTE);
    }

    /// Return the function string for the Implementation.
    const string& getFunction() const
    {
        return getAttribute(FUNCTION_ATTRIBUTE);
    }

    /// @}
    /// @name Language String
    /// @{

    /// Set the language string for the Implementation.
    void setLanguage(const string& language)
    {
        setAttribute(LANGUAGE_ATTRIBUTE, language);
    }

    /// Return true if the given Implementation has a language string.
    bool hasLanguage() const
    {
        return hasAttribute(LANGUAGE_ATTRIBUTE);
    }

    /// Return the language string for the Implementation.
    const string& getLanguage() const
    {
        return getAttribute(LANGUAGE_ATTRIBUTE);
    }

    /// @}

public:
    static const string CATEGORY;
    static const string NODE_DEF_ATTRIBUTE;
    static const string FILE_ATTRIBUTE;
    static const string FUNCTION_ATTRIBUTE;
    static const string LANGUAGE_ATTRIBUTE;
};

} // namespace MaterialX

#endif
