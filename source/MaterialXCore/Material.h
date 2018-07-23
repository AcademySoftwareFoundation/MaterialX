//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MATERIAL_H
#define MATERIALX_MATERIAL_H

/// @file
/// Material element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

class Material;
class ShaderRef;
class BindParam;
class BindInput;
class Override;
class MaterialInherit;
class MaterialAssign;
class Collection;

/// A shared pointer to a Material
using MaterialPtr = shared_ptr<Material>;
/// A shared pointer to a const Material
using ConstMaterialPtr = shared_ptr<const Material>;

/// A shared pointer to a ShaderRef
using ShaderRefPtr = shared_ptr<ShaderRef>;
/// A shared pointer to a const ShaderRef
using ConstShaderRefPtr = shared_ptr<const ShaderRef>;

/// A shared pointer to a BindParam
using BindParamPtr = shared_ptr<BindParam>;
/// A shared pointer to a const BindParam
using ConstBindParamPtr = shared_ptr<const BindParam>;

/// A shared pointer to a BindInput
using BindInputPtr = shared_ptr<BindInput>;
/// A shared pointer to a const BindInput
using ConstBindInputPtr = shared_ptr<const BindInput>;

/// @class Material
/// A material element within a Document.
/// 
/// A Material instantiates one or more shader nodes with a specific set of
/// data bindings.
class Material : public Element
{
  public:
    Material(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~Material() { }

  protected:
    using MaterialAssignPtr = shared_ptr<MaterialAssign>;
    using CollectionPtr = shared_ptr<Collection>;

  public:
    /// @name ShaderRef Elements
    /// @{

    /// Add a ShaderRef to the material.
    /// @param name The name of the new ShaderRef.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param node An optional node string, which should match the node
    ///     attribute of the NodeDef to be referenced.
    /// @return A shared pointer to the new ShaderRef.
    ShaderRefPtr addShaderRef(const string& name = EMPTY_STRING,
                              const string& node = EMPTY_STRING);

    /// Return the ShaderRef, if any, with the given name.
    ShaderRefPtr getShaderRef(const string& name) const
    {
        return getChildOfType<ShaderRef>(name);
    }

    /// Return a vector of all ShaderRef elements in the material.
    vector<ShaderRefPtr> getShaderRefs() const
    {
        return getChildrenOfType<ShaderRef>();
    }

    /// Return a vector of all ShaderRef elements that belong to this material,
    /// taking material inheritance into account.
    vector<ShaderRefPtr> getActiveShaderRefs() const;

    /// Remove the ShaderRef, if any, with the given name.
    void removeShaderRef(const string& name)
    {
        removeChildOfType<ShaderRef>(name);
    }

    /// @}
    /// @name NodeDef References
    /// @{

    /// Return a vector of all shader nodedefs referenced by this material,
    /// optionally filtered by the given target name and shader type.
    /// @param target An optional target name, which will be used to filter
    ///    the shader nodedefs that are returned.
    /// @param type An optional shader type (e.g. "surfaceshader"), which will
    ///    be used to filter the shader nodedefs that are returned.
    /// @return A vector of shared pointers to NodeDef elements.
    vector<NodeDefPtr> getShaderNodeDefs(const string& target = EMPTY_STRING,
                                         const string& type = EMPTY_STRING) const;

    /// @}
    /// @name Primary Shader
    /// @{

    /// Return the nodedef of the first shader referenced by this material,
    /// optionally filtered by the given target and shader type.
    /// @param target An optional target name, which will be used to filter
    ///    the shader nodedefs that are considered.
    /// @param type An optional shader type (e.g. "surfaceshader"), which will
    ///    be used to filter the shader nodedefs that are considered.
    /// @return The nodedef of the first matching shader referenced by this
    ///    material, or an empty shared pointer if no matching shader was found.
    NodeDefPtr getPrimaryShaderNodeDef(const string& target = EMPTY_STRING,
                                       const string& type = EMPTY_STRING) const
    {
        vector<NodeDefPtr> shaderDefs = getShaderNodeDefs(target, type);
        return shaderDefs.empty() ? NodeDefPtr() : shaderDefs[0];
    }

    /// Return the node name of the first shader referenced by this material,
    /// optionally filtered by the given target and shader type.
    /// @param target An optional target name, which will be used to filter
    ///    the shader nodedefs that are considered.
    /// @param type An optional shader type (e.g. "surfaceshader"), which will
    ///    be used to filter the shader nodedefs that are considered.
    /// @return The node name of the first matching shader referenced by this
    ///    material, or an empty string if no matching shader was found.
    string getPrimaryShaderName(const string& target = EMPTY_STRING,
                                const string& type = EMPTY_STRING) const
    {
        NodeDefPtr nodeDef = getPrimaryShaderNodeDef(target, type);
        return nodeDef ? nodeDef->getNodeString() : EMPTY_STRING;
    }

    /// Return the parameters of the first shader referenced by this material,
    /// optionally filtered by the given target and shader type.
    /// @param target An optional target name, which will be used to filter
    ///    the shader nodedefs that are considered.
    /// @param type An optional shader type (e.g. "surfaceshader"), which will
    ///    be used to filter the shader nodedefs that are considered.
    /// @return The parameters of the first matching shader referenced by this
    ///    material, or an empty vector if no matching shader was found.
    vector<ParameterPtr> getPrimaryShaderParameters(const string& target = EMPTY_STRING,
                                                    const string& type = EMPTY_STRING) const;

    /// Return the inputs of the first shader referenced by this material,
    /// optionally filtered by the given target and shader type.
    /// @param target An optional target name, which will be used to filter
    ///    the shader nodedefs that are considered.
    /// @param type An optional shader type (e.g. "surfaceshader"), which will
    ///    be used to filter the shader nodedefs that are considered.
    /// @return The inputs of the first matching shader referenced by this
    ///    material, or an empty vector if no matching shader was found.
    vector<InputPtr> getPrimaryShaderInputs(const string& target = EMPTY_STRING,
                                            const string& type = EMPTY_STRING) const;

    /// @}
    /// @name Geometry Bindings
    /// @{

    /// Return a vector of all MaterialAssign elements that bind this material
    /// to the given geometry string.
    /// @param geom The geometry for which material bindings should be returned.
    ///    By default, this argument is the universal geometry string "/", and
    ///    all material bindings are returned.
    vector<MaterialAssignPtr> getGeometryBindings(const string& geom = UNIVERSAL_GEOM_NAME) const;

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

/// @class BindParam
/// A bind parameter element within a ShaderRef.
///
/// A BindParam binds uniform data to a Parameter of a shader NodeDef within
/// the scope of a Material.
class BindParam : public ValueElement
{
  public:
    BindParam(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~BindParam() { }

  public:
    static const string CATEGORY;
};

/// @class BindInput
/// A bind input element within a ShaderRef.
///
/// A BindInput binds spatially-varying data to an Input of a shader NodeDef
/// within the scope of a material.
class BindInput : public ValueElement
{
  public:
    BindInput(ElementPtr parent, const string& name) :
        ValueElement(parent, CATEGORY, name)
    {
    }
    virtual ~BindInput() { }

    /// @}
    /// @name NodeGraph String
    /// @{

    /// Set the node graph string of this element.
    void setNodeGraphString(const string& graph)
    {
        setAttribute(NODE_GRAPH_ATTRIBUTE, graph);
    }

    /// Return true if this element has a node graph string.
    bool hasNodeGraphString() const
    {
        return hasAttribute(NODE_GRAPH_ATTRIBUTE);
    }

    /// Return the node graph string of this element.
    const string& getNodeGraphString() const
    {
        return getAttribute(NODE_GRAPH_ATTRIBUTE);
    }

    /// @}
    /// @name Output String
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

    /// Set the output to which the BindInput is connected.
    void setConnectedOutput(ConstOutputPtr output);

    /// Return the output, if any, to which the BindInput is connected.
    OutputPtr getConnectedOutput() const;

    /// @}

  public:
    static const string CATEGORY;
    static const string NODE_GRAPH_ATTRIBUTE;
    static const string OUTPUT_ATTRIBUTE;
};

/// @class ShaderRef
/// A shader reference element within a Material.
///
/// A ShaderRef instantiates a shader NodeDef within the context of a Material.
class ShaderRef : public TypedElement
{
  public:
    ShaderRef(ElementPtr parent, const string& name) :
        TypedElement(parent, CATEGORY, name)
    {
    }
    virtual ~ShaderRef() { }

    /// @name Node String
    /// @{

    /// Set the node string of the ShaderRef.  This attribute declares a
    /// ShaderRef as a reference to the first NodeDef with the matching
    /// node string.
    void setNodeString(const string& node)
    {
        setAttribute(NODE_ATTRIBUTE, node);
    }

    /// Return true if the given ShaderRef has a node string.
    bool hasNodeString() const
    {
        return hasAttribute(NODE_ATTRIBUTE);
    }

    /// Return the node string of the ShaderRef.
    const string& getNodeString() const
    {
        return getAttribute(NODE_ATTRIBUTE);
    }

    /// @}
    /// @name NodeDef String
    /// @{

    /// Set the NodeDef string for the ShaderRef.  This attribute declares a
    /// ShaderRef as a reference to the unique NodeDef with the given name.
    void setNodeDefString(const string& nodeDef)
    {
        setAttribute(NODE_DEF_ATTRIBUTE, nodeDef);
    }

    /// Return true if the given ShaderRef has a NodeDef string.
    bool hasNodeDefString() const
    {
        return hasAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// Return the NodeDef string for the ShaderRef.
    const string& getNodeDefString() const
    {
        return getAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// @}
    /// @name BindParam Elements
    /// @{

    /// Add a BindParam to the ShaderRef.
    /// @param name The name of the new BindParam.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new BindParam.
    BindParamPtr addBindParam(const string& name, const string& type = DEFAULT_TYPE_STRING)
    {
        BindParamPtr child = addChild<BindParam>(name);
        child->setType(type);
        return child;
    }

    /// Return the BindParam, if any, with the given name.
    BindParamPtr getBindParam(const string& name) const
    {
        return getChildOfType<BindParam>(name);
    }

    /// Return a vector of all BindParam elements in the ShaderRef.
    vector<BindParamPtr> getBindParams() const
    {
        return getChildrenOfType<BindParam>();
    }

    /// Remove the BindParam, if any, with the given name.
    void removeBindParam(const string& name)
    {
        removeChildOfType<BindParam>(name);
    }

    /// @}
    /// @name BindInput Elements
    /// @{

    /// Add a BindInput to the ShaderRef.
    /// @param name The name of the new BindInput.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new BindInput.
    BindInputPtr addBindInput(const string& name, const string& type = DEFAULT_TYPE_STRING)
    {
        BindInputPtr child = addChild<BindInput>(name);
        child->setType(type);
        return child;
    }

    /// Return the BindInput, if any, with the given name.
    BindInputPtr getBindInput(const string& name) const
    {
        return getChildOfType<BindInput>(name);
    }

    /// Return a vector of all BindInput elements in the ShaderRef.
    vector<BindInputPtr> getBindInputs() const
    {
        return getChildrenOfType<BindInput>();
    }

    /// Remove the BindInput, if any, with the given name.
    void removeBindInput(const string& name)
    {
        removeChildOfType<BindInput>(name);
    }

    /// @}
    /// @name NodeDef References
    /// @{

    /// Return the NodeDef, if any, that this element references.
    NodeDefPtr getNodeDef() const;

    /// @}
    /// @name Output References
    /// @{

    /// Return a vector of all outputs that this element references.
    vector<OutputPtr> getReferencedOutputs() const
    {
        std::set<OutputPtr> outputs;
        for (BindInputPtr bindInput : getBindInputs())
        {
            OutputPtr output = bindInput->getConnectedOutput();
            if (output)
            {
                outputs.insert(output);
            }
        }
        return vector<OutputPtr>(outputs.begin(), outputs.end());
    }

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    bool validate(string* message = nullptr) const override;

    /// @}
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(ConstMaterialPtr material = nullptr,
                         size_t index = 0) const override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() const override
    {
        return getBindInputs().size();
    }

    /// @}

  public:
    static const string CATEGORY;
    static const string NODE_ATTRIBUTE;
    static const string NODE_DEF_ATTRIBUTE;
};

} // namespace MaterialX

#endif
