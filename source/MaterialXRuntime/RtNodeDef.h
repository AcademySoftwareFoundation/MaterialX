//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEDEF_H
#define MATERIALX_RTNODEDEF_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{

/// @class RtNodeDef
/// API for accessing a node definition. This API can only be
/// attached to objects of type NODEDEF.
class RtNodeDef : public RtPrim
{
public:
    /// Constructor attaching an object to the API.
    RtNodeDef(const RtObject& obj);

    /// Return the type name for nodedefs.
    static const RtToken& typeName();

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Set the node type name.
    void setNodeTypeName(const RtToken& nodeTypeName);

    /// Return the node type name.
    const RtToken& getNodeTypeName() const;

    /// Add an attribute to the definition.
    RtObject createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an attribute from the definition.
    void removeAttribute(const RtToken& name);
};

}

#endif
