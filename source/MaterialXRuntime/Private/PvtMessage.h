//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTMESSAGE_H
#define MATERIALX_PVTMESSAGE_H

#include <MaterialXRuntime/RtMessage.h>

namespace MaterialX
{

enum class PvtMessageType
{
    CREATE_PRIM,
    REMOVE_PRIM,
    RENAME_PRIM,
    REPARENT_PRIM,
    SET_PORT_VALUE,
    SET_ATTRIBUTE,
    REMOVE_ATTRIBUTE,
    CHANGE_CONNECTION,
    CHANGE_RELATIONSHIP,
    NUM_TYPES,
};

template<typename T>
using PvtCallbackIdMap = std::unordered_map<RtCallbackId, T>;

template<PvtMessageType TYPE, typename Callback>
struct PvtObserver
{
    PvtMessageType type;
    Callback callback;
    void* userData;

    PvtObserver() : type(PvtMessageType::NUM_TYPES), callback(nullptr), userData(nullptr) {}
    PvtObserver(Callback cb, void* data) : type(TYPE), callback(cb), userData(data) {}
};

using PvtCreatePrimObserver = PvtObserver<PvtMessageType::CREATE_PRIM, RtCreatePrimCallbackFunc>;
using PvtRemovePrimObserver = PvtObserver<PvtMessageType::REMOVE_PRIM, RtRemovePrimCallbackFunc>;
using PvtRenamePrimObserver = PvtObserver<PvtMessageType::RENAME_PRIM, RtRenamePrimCallbackFunc>;
using PvtReparentPrimObserver = PvtObserver<PvtMessageType::REPARENT_PRIM, RtReparentPrimCallbackFunc>;
using PvtSetPortValueObserver = PvtObserver<PvtMessageType::SET_PORT_VALUE, RtSetPortValueCallbackFunc>;
using PvtSetAttributeObserver = PvtObserver<PvtMessageType::SET_ATTRIBUTE, RtSetAttributeCallbackFunc>;
using PvtRemoveAttributeObserver = PvtObserver<PvtMessageType::REMOVE_ATTRIBUTE, RtRemoveAttributeCallbackFunc>;
using PvtConnectionObserver = PvtObserver<PvtMessageType::CHANGE_CONNECTION, RtConnectionCallbackFunc>;
using PvtRelationshipObserver = PvtObserver<PvtMessageType::CHANGE_RELATIONSHIP, RtRelationshipCallbackFunc>;

class PvtMessageHandler
{
public:
    PvtMessageHandler();

    RtCallbackId addCreatePrimCallback(RtCreatePrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addRemovePrimCallback(RtRemovePrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addRenamePrimCallback(RtRenamePrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addReparentPrimCallback(RtReparentPrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addSetPortValueCallback(RtSetPortValueCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addRemoveAttributeCallback(RtRemoveAttributeCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addConnectionCallback(RtConnectionCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addRelationshipCallback(RtRelationshipCallbackFunc callback, void* userData = nullptr);

    void removeCallback(RtCallbackId id);

    void sendCreatePrimMessage(RtStagePtr stage, const RtPrim& prim);
    void sendRemovePrimMessage(RtStagePtr stage, const RtPrim& prim);
    void sendRenamePrimMessage(RtStagePtr stage, const RtPrim& prim, const RtString& newName);
    void sendReparentPrimMessage(RtStagePtr stage, const RtPrim& prim, const RtPath& newParentPath);
    void sendSetPortValueMessage(const RtPort& port, const RtValue& value);
    void sendSetAttributeMessage(const RtObject &obj, const RtString& name, const RtValue& value);
    void sendRemoveAttributeMessage(const RtObject& obj, const RtString& name);
    void sendConnectionMessage(const RtOutput& src, const RtInput& dest, ConnectionChange change);
    void sendRelationshipMessage(const RtRelationship& rel, const RtObject& target, ConnectionChange change);

private:
    RtCallbackId _callbackIdCounter;

    // Keep track of the type pf message for each callback id
    PvtCallbackIdMap<PvtMessageType> _callbackIdToType;

    // Maps of observers for each message type
    PvtCallbackIdMap<PvtCreatePrimObserver> _createPrimObservers;
    PvtCallbackIdMap<PvtRemovePrimObserver> _removePrimObservers;
    PvtCallbackIdMap<PvtRenamePrimObserver> _renamePrimObservers;
    PvtCallbackIdMap<PvtReparentPrimObserver> _reparentPrimObservers;
    PvtCallbackIdMap<PvtSetPortValueObserver> _setPortValueObservers;
    PvtCallbackIdMap<PvtSetAttributeObserver> _setAttrObservers;
    PvtCallbackIdMap<PvtRemoveAttributeObserver> _removeAttributeObservers;
    PvtCallbackIdMap<PvtConnectionObserver> _connectionObservers;
    PvtCallbackIdMap<PvtRelationshipObserver> _relationshipObservers;
};

}

#endif
