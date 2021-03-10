//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCONNECTABLEAPI_H
#define MATERIALX_RTCONNECTABLEAPI_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPort.h>
#include <MaterialXRuntime/RtRelationship.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class RtConnectableApi;
using RtConnectableApiPtr = shared_ptr<RtConnectableApi>;

/// @class RtConnectableApi
/// API for validating attribute connections and relationships for a prim type.
class RtConnectableApi
{
public:
    /// Destructor.
    virtual ~RtConnectableApi() {}

    /// Return true if the given input accepts a connection from the given output.
    /// Default implementation accept outputs with a matching datatype. Derived classes
    /// can override this to change connectability.
    virtual bool acceptConnection(const RtInput& input, const RtOutput& output) const;

    /// Return true if this relationship accepts the given object as a target.
    /// Default implementation accept all targets. Derived classes can override 
    /// this to change connectability.
    virtual bool acceptRelationship(const RtRelationship& rel, const RtObject& target) const;

    /// Register a connectable API for a given prim typename.
    static void registerApi(const RtToken& typeName, const RtConnectableApiPtr& api);

    /// Unregister a connectable API for a given prim typename.
    static void unregisterApi(const RtToken& typeName);

    /// Register a connectable API for a templated prim schema.
    template<class PrimSchema, class ConnectableApi = RtConnectableApi>
    inline static void registerApi()
    {
        registerApi(PrimSchema::typeName(), shared_ptr<ConnectableApi>(new ConnectableApi));
    }

    /// Unregister a connectable API for a templated prim schema.
    template<class PrimSchema>
    inline static void unregisterApi()
    {
        unregisterApi(PrimSchema::typeName());
    }

    /// Return a connectable API for a given prim.
    static RtConnectableApi* get(const RtPrim& prim);
};

}

#endif
