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
class MaterialInherit;
class MaterialAssign;
class Collection;

/// A shared pointer to a Material
using MaterialPtr = shared_ptr<Material>;
/// A shared pointer to a const Material
using ConstMaterialPtr = shared_ptr<const Material>;

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

    /// Return the tokens of the first shader referenced by this material,
    /// optionally filtered by the given target and shader type.
    /// @param target An optional target name, which will be used to filter
    ///    the shader nodedefs that are considered.
    /// @param type An optional shader type (e.g. "surfaceshader"), which will
    ///    be used to filter the shader nodedefs that are considered.
    /// @return The tokens of the first matching shader referenced by this
    ///    material, or an empty vector if no matching shader was found.
    vector<TokenPtr> getPrimaryShaderTokens(const string& target = EMPTY_STRING,
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

  public:
    static const string CATEGORY;
};

} // namespace MaterialX

#endif
