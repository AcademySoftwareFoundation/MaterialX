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
    SET_ATTRIBUTE,
    MAKE_CONNECTION,
    BREAK_CONNECTION,
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
using PvtSetAttributeObserver = PvtObserver<PvtMessageType::SET_ATTRIBUTE, RtSetAttributeCallbackFunc>;
using PvtMakeConnectionObserver = PvtObserver<PvtMessageType::MAKE_CONNECTION, RtMakeConnectionCallbackFunc>;
using PvtBreakConnectionObserver = PvtObserver<PvtMessageType::BREAK_CONNECTION, RtBreakConnectionCallbackFunc>;

class PvtMessageHandler
{
public:
    PvtMessageHandler();

    RtCallbackId addCreatePrimCallback(RtCreatePrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addRemovePrimCallback(RtRemovePrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addRenamePrimCallback(RtRenamePrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addReparentPrimCallback(RtReparentPrimCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addMakeConnectionCallback(RtMakeConnectionCallbackFunc callback, void* userData = nullptr);
    RtCallbackId addBreakConnectionCallback(RtBreakConnectionCallbackFunc callback, void* userData = nullptr);

    void removeCallback(RtCallbackId id);

    void sendCreatePrimMessage(RtStagePtr stage, const RtPrim& prim);
    void sendRemovePrimMessage(RtStagePtr stage, const RtPrim& prim);
    void sendRenamePrimMessage(RtStagePtr stage, const RtPrim& prim, const RtToken& newName);
    void sendReparentPrimMessage(RtStagePtr stage, const RtPrim& prim, const RtPath& newParentPath);
    void sendSetAttributeMessage(const RtAttribute& attr, const RtValue& value);
    void sendMakeConnectionMessage(const RtOutput& src, const RtInput& dest);
    void sendBreakConnectionMessage(const RtOutput& src, const RtInput& dest);

private:
    RtCallbackId _callbackIdCounter;

    // Keep track of the type pf message for each callback id
    PvtCallbackIdMap<PvtMessageType> _callbackIdToType;

    // Maps of observers for each message type
    PvtCallbackIdMap<PvtCreatePrimObserver> _createPrimObservers;
    PvtCallbackIdMap<PvtRemovePrimObserver> _removePrimObservers;
    PvtCallbackIdMap<PvtRenamePrimObserver> _renamePrimObservers;
    PvtCallbackIdMap<PvtReparentPrimObserver> _reparentPrimObservers;
    PvtCallbackIdMap<PvtSetAttributeObserver> _setAttrObservers;
    PvtCallbackIdMap<PvtMakeConnectionObserver> _makeConnectionObservers;
    PvtCallbackIdMap<PvtBreakConnectionObserver> _breakConnectionObservers;
};

}

#endif
