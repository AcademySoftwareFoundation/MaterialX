//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEDEF_H
#define MATERIALX_RTNODEDEF_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

/// @class RtNodeDef
class RtNodeDef : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNodeDef)

public:
    /// Return the node for this nodedef.
    const RtToken& getNode() const;

    /// Set the node for this nodedef.
    void setNode(const RtToken& node);

    /// Register this nodedef as a master prim
    /// to make it instantiable for node creation.
    void registerMasterPrim() const;

    /// Unregister this nodedef as a master prim.
    void unregisterMasterPrim() const;

    /// Return true if this nodedef is registerd as a master prim.
    bool isMasterPrim() const;
};

}

#endif
