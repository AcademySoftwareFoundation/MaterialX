//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_INTERFACE_H
#define MATERIALX_INTERFACE_H

/// @file
/// Interface element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Element.h>

namespace MaterialX
{

class Parameter;
class PortElement;
class Input;
class Output;
class InterfaceElement;
class Node;
class NodeDef;

/// A shared pointer to a Parameter
using ParameterPtr = shared_ptr<Parameter>;
/// A shared pointer to a const Parameter
using ConstParameterPtr = shared_ptr<const Parameter>;

/// A shared pointer to a PortElement
using PortElementPtr = shared_ptr<PortElement>;
/// A shared pointer to a const PortElement
using ConstPortElementPtr = shared_ptr<const PortElement>;

/// A shared pointer to an Input
using InputPtr = shared_ptr<Input>;
/// A shared pointer to a const Input
using ConstInputPtr = shared_ptr<const Input>;

/// A shared pointer to an Output
using OutputPtr = shared_ptr<Output>;
/// A shared pointer to a const Output
using ConstOutputPtr = shared_ptr<const Output>;

/// A shared pointer to an InterfaceElement
using InterfaceElementPtr = shared_ptr<InterfaceElement>;
/// A shared pointer to a const InterfaceElement
using ConstInterfaceElementPtr = shared_ptr<const InterfaceElement>;

/// @class Parameter
/// A parameter element within a Node or NodeDef.
///
/// A Parameter holds a single uniform value, which may be modified within the
/// scope of a Material.
class Parameter : public ValueElement
{
  public:
    Parameter(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~Parameter() { }

    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(ConstMaterialPtr material = nullptr,
                         size_t index = 0) const override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() const override
    {
        return 1;
    }

    /// @}

  public:
    static const string CATEGORY;
};

/// @class PortElement
/// The base class for port elements such as Input and Output.
///
/// Port elements support spatially-varying upstream connections to nodes.
class PortElement : public ValueElement
{
  protected:
    PortElement(ElementPtr parent, const string& category, const string& name) :
        ValueElement(parent, category, name)
    {
    }
  public:
    virtual ~PortElement() { }

  protected:
    using NodePtr = shared_ptr<Node>;

  public:
    /// @name Node Name
    /// @{

    /// Set the node name string of this element, creating a connection to
    /// the Node with the given name within the same NodeGraph.
    void setNodeName(const string& node)
    {
        setAttribute(NODE_NAME_ATTRIBUTE, node);
    }

    /// Return true if this element has a node name string.
    bool hasNodeName() const
    {
        return hasAttribute(NODE_NAME_ATTRIBUTE);
    }

    /// Return the node name string of this element.
    const string& getNodeName() const
    {
        return getAttribute(NODE_NAME_ATTRIBUTE);
    }

    /// @}
    /// @name Output
    /// @{

    /// Set the output string of this element.
    void setOutputString(const string& output)
    {
        setAttribute(OUTPUT_ATTRIBUTE, output);
    }

    /// Return true if this element has an output string.
    bool hasOutputString() const
    {
        return hasAttribute(OUTPUT_ATTRIBUTE);
    }

    /// Return the output string of this element.
    const string& getOutputString() const
    {
        return getAttribute(OUTPUT_ATTRIBUTE);
    }

    /// @}
    /// @name Connections
    /// @{

    /// Set the node to which this element is connected.  The given node must
    /// belong to the same node graph.
    void setConnectedNode(NodePtr node);

    /// Return the node, if any, to which this element is connected.
    NodePtr getConnectedNode() const;

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    bool validate(string* message = nullptr) const override;

    /// @}

  public:
    static const string NODE_NAME_ATTRIBUTE;
    static const string OUTPUT_ATTRIBUTE;
};

/// @class Input
/// An input element within a Node or NodeDef.
///
/// An Input holds either a uniform value or a connection to a spatially-varying
/// Output, either of which may be modified within the scope of a Material.
class Input : public PortElement
{
  public:
    Input(ElementPtr parent, const string& name) :
        PortElement(parent, CATEGORY, name)
    {
    }
    virtual ~Input() { }

  public:
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(ConstMaterialPtr material = nullptr,
                         size_t index = 0) const override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() const override
    {
        return 1;
    }

    /// @}

  public:
    static const string CATEGORY;
};

/// @class Output
/// A spatially-varying output element within a NodeGraph or NodeDef.
class Output : public PortElement
{
  public:
    Output(ElementPtr parent, const string& name) :
        PortElement(parent, CATEGORY, name)
    {
    }
    virtual ~Output() { }

  public:
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(ConstMaterialPtr material = nullptr,
                         size_t index = 0) const override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() const override
    {
        return 1;
    }

    /// Return true if a cycle exists in any upstream path from this element.
    bool hasUpstreamCycle() const;

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    bool validate(string* message = nullptr) const override;

    /// @}

  public:
    static const string CATEGORY;
};

/// @class InterfaceElement
/// The base class for interface elements such as Node, NodeDef, and NodeGraph.
///
/// An InterfaceElement supports a set of Parameter, Input, and Output elements,
/// with an API for setting their values.
class InterfaceElement : public TypedElement
{
  protected:
    InterfaceElement(ElementPtr parent, const string& category, const string& name) :
        TypedElement(parent, category, name),
        _parameterCount(0),
        _inputCount(0),
        _outputCount(0)
    {
    }
  public:
    virtual ~InterfaceElement() { }

  protected:
    using NodeDefPtr = shared_ptr<NodeDef>;
    using ConstNodeDefPtr = shared_ptr<const NodeDef>;

  public:
    /// @name NodeDef String
    /// @{

    /// Set the NodeDef string for the interface.
    void setNodeDefString(const string& nodeDef)
    {
        setAttribute(NODE_DEF_ATTRIBUTE, nodeDef);
    }

    /// Return true if the given interface has a NodeDef string.
    bool hasNodeDefString() const
    {
        return hasAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// Return the NodeDef string for the interface.
    const string& getNodeDefString() const
    {
        return getAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// Set the NodeDef element for the interface.
    void setNodeDef(ConstNodeDefPtr nodeDef);

    /// Return the NodeDef element for the interface.
    NodeDefPtr getNodeDef() const;

    /// @name Parameters
    /// @{
    
    /// Add a Parameter to this interface.
    /// @param name The name of the new Parameter.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Parameter.
    ParameterPtr addParameter(const string& name = DEFAULT_TYPE_STRING,
                              const string& type = DEFAULT_TYPE_STRING)
    {
        ParameterPtr child = addChild<Parameter>(name);
        child->setType(type);
        return child;
    }

    /// Return the Parameter, if any, with the given name.
    ParameterPtr getParameter(const string& name) const
    {
        return getChildOfType<Parameter>(name);
    }

    /// Return a vector of all Parameter elements.
    vector<ParameterPtr> getParameters() const
    {
        return getChildrenOfType<Parameter>();
    }

    /// Return the number of Parameter elements.
    size_t getParameterCount() const
    {
        return _parameterCount;
    }

    /// Remove the Parameter, if any, with the given name.
    void removeParameter(const string& name)
    {
        removeChildOfType<Parameter>(name);
    }

    /// Return the first Parameter with the given name that belongs to this
    /// interface, taking interface inheritance into account.
    ParameterPtr getActiveParameter(const string& name) const;

    /// Return a vector of all Parameter elements that belong to this interface,
    /// taking interface inheritance into account.
    vector<ParameterPtr> getActiveParameters() const;

    /// @}
    /// @name Inputs
    /// @{

    /// Add an Input to this interface.
    /// @param name The name of the new Input.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Input.
    InputPtr addInput(const string& name = DEFAULT_TYPE_STRING,
                      const string& type = DEFAULT_TYPE_STRING)
    {
        InputPtr child = addChild<Input>(name);
        child->setType(type);
        return child;
    }

    /// Return the Input, if any, with the given name.
    InputPtr getInput(const string& name) const
    {
        return getChildOfType<Input>(name);
    }

    /// Return a vector of all Input elements.
    vector<InputPtr> getInputs() const
    {
        return getChildrenOfType<Input>();
    }

    /// Return the number of Input elements.
    size_t getInputCount() const
    {
        return _inputCount;
    }

    /// Remove the Input, if any, with the given name.
    void removeInput(const string& name)
    {
        removeChildOfType<Input>(name);
    }

    /// Return the first Input with the given name that belongs to this
    /// interface, taking interface inheritance into account.
    InputPtr getActiveInput(const string& name) const;

    /// Return a vector of all Input elements that belong to this interface,
    /// taking inheritance into account.
    vector<InputPtr> getActiveInputs() const;

    /// @}
    /// @name Outputs
    /// @{

    /// Add an Output to this interface.
    /// @param name The name of the new Output.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Output.
    OutputPtr addOutput(const string& name = EMPTY_STRING,
                        const string& type = DEFAULT_TYPE_STRING)
    {
        OutputPtr output = addChild<Output>(name);
        output->setType(type);
        return output;
    }

    /// Return the Output, if any, with the given name.
    OutputPtr getOutput(const string& name) const
    {
        return getChildOfType<Output>(name);
    }

    /// Return a vector of all Output elements.
    vector<OutputPtr> getOutputs() const
    {
        return getChildrenOfType<Output>();
    }

    /// Return the number of Output elements.
    size_t getOutputCount() const
    {
        return _outputCount;
    }

    /// Remove the Output, if any, with the given name.
    void removeOutput(const string& name)
    {
        removeChildOfType<Output>(name);
    }

    /// Return the first Output with the given name that belongs to this
    /// interface, taking interface inheritance into account.
    OutputPtr getActiveOutput(const string& name) const;

    /// Return a vector of all Output elements that belong to this interface,
    /// taking inheritance into account.
    vector<OutputPtr> getActiveOutputs() const;

    /// @}
    /// @name Tokens
    /// @{

    /// Add a Token to this interface.
    /// @param name The name of the new Token.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @return A shared pointer to the new Token.
    TokenPtr addToken(const string& name = EMPTY_STRING)
    {
        return addChild<Token>(name);
    }

    /// Return the Token, if any, with the given name.
    TokenPtr getToken(const string& name) const
    {
        return getChildOfType<Token>(name);
    }

    /// Return a vector of all Token elements.
    vector<TokenPtr> getTokens() const
    {
        return getChildrenOfType<Token>();
    }

    /// Remove the Token, if any, with the given name.
    void removeToken(const string& name)
    {
        removeChildOfType<Token>(name);
    }

    /// Return the first Token with the given name that belongs to this
    /// interface, taking interface inheritance into account.
    TokenPtr getActiveToken(const string& name) const;

    /// Return a vector of all Token elements that belong to this interface,
    /// taking inheritance into account.
    vector<TokenPtr> getActiveTokens() const;

    /// @}
    /// @name Value Elements
    /// @{

    /// Return the first value element with the given name that belongs to this
    /// interface, taking interface inheritance into account.
    /// Examples of value elements are Parameter, Input, Output, and Token.
    ValueElementPtr getActiveValueElement(const string& name) const;

    /// Return a vector of all value elements that belong to this interface,
    /// taking inheritance into account.
    /// Examples of value elements are Parameter, Input, Output, and Token.
    vector<ValueElementPtr> getActiveValueElements() const;

    /// @}
    /// @name Values
    /// @{

    /// Set the typed value of a parameter by its name, creating a child element
    /// to hold the parameter if needed.
    template<class T> ParameterPtr setParameterValue(const string& name,
                                                     const T& value,
                                                     const string& type = EMPTY_STRING);

    /// Return the typed value of a parameter by its name, taking both the
    /// calling element and its declaration into account.
    /// @param name The name of the parameter to be evaluated.
    /// @param target An optional target name, which will be used to filter
    ///    the declarations that are considered.
    /// @return If the given parameter is found in this interface or its
    ///    declaration, then a shared pointer to its value is returned;
    ///    otherwise, an empty shared pointer is returned.
    ValuePtr getParameterValue(const string& name, const string& target = EMPTY_STRING) const;

    /// Set the typed value of an input by its name, creating a child element
    /// to hold the input if needed.
    template<class T> InputPtr setInputValue(const string& name,
                                             const T& value,
                                             const string& type = EMPTY_STRING);

    /// Return the typed value of an input by its name, taking both the calling
    /// element and its declaration into account.
    /// @param name The name of the input to be evaluated.
    /// @param target An optional target name, which will be used to filter
    ///    the declarations that are considered.
    /// @return If the given parameter is found in this interface or its
    ///    declaration, then a shared pointer to its value is returned;
    ///    otherwise, an empty shared pointer is returned.
    ValuePtr getInputValue(const string& name, const string& target = EMPTY_STRING) const;

    /// Set the string value of a Token by its name, creating a child element
    /// to hold the Token if needed.
    TokenPtr setTokenValue(const string& name, const string& value)
    {
        TokenPtr token = getToken(name);
        if (!token)
            token = addToken(name);
        token->setValue<std::string>(value);
        return token;
    }

    /// Return the string value of a Token by its name, or an empty string if
    /// the given Token is not present.
    string getTokenValue(const string& name)
    {
        TokenPtr token = getToken(name);
        return token ? token->getValueString() : EMPTY_STRING;
    }

    /// @}
    /// @name Utility
    /// @{

    /// Return the first declaration of this interface, optionally filtered
    ///    by the given target name.
    /// @param target An optional target name, which will be used to filter
    ///    the declarations that are considered.
    /// @return A shared pointer to nodedef, or an empty shared pointer if
    ///    no declaration was found.
    NodeDefPtr getDeclaration(const string& target = EMPTY_STRING) const;

    /// Return true if the given interface element is type compatible with
    /// this one.  This may be used to test, for example, whether a NodeDef
    /// and Node may be used together.
    ///
    /// If the type string of the given interface element differs from this
    /// one, then false is returned.
    ///
    /// If the two interface elements have child Parameter or Input elements
    /// with identical names but different types, then false is returned.  Note
    /// that a Parameter or Input that is present in only one of the two
    /// interfaces does not affect their type compatibility.
    bool isTypeCompatible(ConstInterfaceElementPtr rhs) const;

    /// @}

  public:
    static const string NODE_DEF_ATTRIBUTE;

  protected:
    void registerChildElement(ElementPtr child) override;
    void unregisterChildElement(ElementPtr child) override;

  private:
    size_t _parameterCount;
    size_t _inputCount;
    size_t _outputCount;
};

template<class T> ParameterPtr InterfaceElement::setParameterValue(const string& name,
                                                                   const T& value,
                                                                   const string& type)
{
    ParameterPtr param = getChildOfType<Parameter>(name);
    if (!param)
        param = addParameter(name);
    param->setValue(value, type);
    return param;
}

template<class T> InputPtr InterfaceElement::setInputValue(const string& name,
                                                           const T& value,
                                                           const string& type)
{
    InputPtr input = getChildOfType<Input>(name);
    if (!input)
        input = addInput(name);
    input->setValue(value, type);
    return input;
}

} // namespace MaterialX

#endif
