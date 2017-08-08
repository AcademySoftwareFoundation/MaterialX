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

/// A shared pointer to a Parameter
using ParameterPtr = shared_ptr<class Parameter>;
/// A shared pointer to a PortElement
using PortElementPtr = shared_ptr<class PortElement>;
/// A shared pointer to an Input
using InputPtr = shared_ptr<class Input>;
/// A shared pointer to an Output
using OutputPtr = shared_ptr<class Output>;
/// A shared pointer to an InterfaceElement
using InterfaceElementPtr = shared_ptr<class InterfaceElement>;

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
    Edge getUpstreamEdge(MaterialPtr material = MaterialPtr(),
                         size_t index = 0) override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() override
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
    using NodePtr = shared_ptr<class Node>;

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
    /// @name Channels
    /// @{

    /// Set the channels string of this element, defining a channel swizzle
    /// that will be applied to this port.
    void setChannels(const string& channels)
    {
        setAttribute(CHANNELS_ATTRIBUTE, channels);
    }

    /// Return true if this element has a channels string.
    bool hasChannels() const
    {
        return hasAttribute(CHANNELS_ATTRIBUTE);
    }

    /// Return the channels string of this element.
    const string& getChannels() const
    {
        return getAttribute(CHANNELS_ATTRIBUTE);
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
    static const string CHANNELS_ATTRIBUTE;
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

  protected:
    using NodePtr = shared_ptr<class Node>;

  public:
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(MaterialPtr material = MaterialPtr(),
                         size_t index = 0) override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() override
    {
        return 1;
    }

    /// @}

  public:
    static const string CATEGORY;
};

/// @class Output
/// A spatially-varying output element within a NodeGraph.
class Output : public PortElement
{
  public:
    Output(ElementPtr parent, const string& name) :
        PortElement(parent, CATEGORY, name)
    {
    }
    virtual ~Output() { }

  protected:
    using NodePtr = shared_ptr<class Node>;

  public:
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(MaterialPtr material = MaterialPtr(),
                         size_t index = 0) override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() override
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
/// The base class for interface elements such as Node and NodeDef.
///
/// An InterfaceElement supports a set of Parameter and Input elements, with
/// an API for setting their values.
class InterfaceElement : public TypedElement
{
  protected:
    InterfaceElement(ElementPtr parent, const string& category, const string& name) :
        TypedElement(parent, category, name),
        _parameterCount(0),
        _inputCount(0)
    {
    }
  public:
    virtual ~InterfaceElement() { }

  protected:
    using NodePtr = shared_ptr<class Node>;

  public:
    /// @name Parameters
    /// @{
    
    /// Add a Parameter to this element.
    /// @param name The name of the new Parameter.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Parameter.
    ParameterPtr addParameter(const string& name, const string& type = DEFAULT_TYPE_STRING)
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

    /// Set the value of a parameter by its name, creating a child element
    /// to hold the parameter if needed.
    template<class T> ParameterPtr setParameterValue(const string& name,
                                                     const T& value,
                                                     const string& type = EMPTY_STRING);

    /// Return the value instance of a parameter by its name.  If the given parameter
    /// is not present, then an empty ValuePtr is returned.
    ValuePtr getParameterValue(const string& name) const;

    /// Return the value string of a parameter by its name.  If the given parameter
    /// is not present, then an empty string is returned.
    const string& getParameterValueString(const string& name) const;

    /// @}
    /// @name Inputs
    /// @{

    /// Add an Input to this element.
    /// @param name The name of the new Input.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Input.
    InputPtr addInput(const string& name, const string& type = DEFAULT_TYPE_STRING)
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

    /// @}

  protected:
    void registerChildElement(ElementPtr child) override;
    void unregisterChildElement(ElementPtr child) override;

  private:
    size_t _parameterCount;
    size_t _inputCount;
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

} // namespace MaterialX

#endif
