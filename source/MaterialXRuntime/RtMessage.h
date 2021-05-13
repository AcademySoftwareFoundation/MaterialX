//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTMESSAGE_H
#define MATERIALX_RTMESSAGE_H

/// @file
/// Classes for notification of data model changes.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTypeInfo.h>
#include <MaterialXRuntime/RtPort.h>
#include <MaterialXRuntime/RtRelationship.h>

namespace MaterialX
{

class RtRelationshipIterator;
class RtPrimIterator;
class RtSchemaBase;

/// Type for storing callback IDs.
using RtCallbackId = size_t;

/// Function type for callback notifying when a prim has been created.
using RtCreatePrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, void* userData)>;

/// Function type for callback notifying when a prim is about to be removed.
using RtRemovePrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, void* userData)>;

/// Function type for callback notifying when a prim is about to be renamed.
using RtRenamePrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, const RtString& newName, void* userData)>;

/// Function type for callback notifying when a prim is about to be reparented.
using RtReparentPrimCallbackFunc = std::function<void(RtStagePtr stage, const RtPrim& prim, const RtPath& newPath, void* userData)>;

/// Function type for callback notifying when a port value is set.
using RtSetPortValueCallbackFunc = std::function<void(const RtPort& port, const RtValue& value, void* userData)>;

/// Function type for callback notifying when an attribute is set.
using RtSetAttributeCallbackFunc = std::function<void(const RtObject& obj, const RtString& name, const RtValue& value, void* userData)>;

/// Function type for callback notifying when a metadata value is removed.
using RtRemoveAttributeCallbackFunc = std::function<void(const RtObject& obj, const RtString& name, void* userData)>;

/// Function type for callback notifying when a connection is changed.
using RtConnectionCallbackFunc = std::function<void(const RtOutput& src, const RtInput& dest, ConnectionChange change, void* userData)>;

/// Function type for callback notifying when a relationship is changed.
using RtRelationshipCallbackFunc = std::function<void(const RtRelationship& rel, const RtObject& target, ConnectionChange change, void* userData)>;

/// @class RtMessage
class RtMessage
{
public:
    /// Register a callback to get notified when a prim has been created.
    static RtCallbackId addCreatePrimCallback(RtCreatePrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a prim is about to be removed.
    static RtCallbackId addRemovePrimCallback(RtRemovePrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a prim is about to be renamed.
    static RtCallbackId addRenamePrimCallback(RtRenamePrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a prim is about to be reparented.
    static RtCallbackId addReparentPrimCallback(RtReparentPrimCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a port value is set.
    static RtCallbackId addSetPortValueCallback(RtSetPortValueCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when an attribute is set.
    static RtCallbackId addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when an attribute is removed.
    static RtCallbackId addRemoveAttributeCallback(RtRemoveAttributeCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a connection is changed.
    static RtCallbackId addConnectionCallback(RtConnectionCallbackFunc callback, void* userData = nullptr);

    /// Register a callback to get notified when a relationship is changed.
    static RtCallbackId addRelationshipCallback(RtRelationshipCallbackFunc callback, void* userData = nullptr);

    /// Remove the callback with given id.
    static void removeCallback(RtCallbackId id);
};

}

#endif
