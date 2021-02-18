//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtMessage.h>

namespace MaterialX
{

PvtMessageHandler::PvtMessageHandler() :
    _callbackIdCounter(1)
{}

RtCallbackId PvtMessageHandler::addCreatePrimCallback(RtCreatePrimCallbackFunc callback, void* userData)
{
    PvtCreatePrimObserver observer = PvtCreatePrimObserver(callback, userData);
    _createPrimObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addRemovePrimCallback(RtRemovePrimCallbackFunc callback, void* userData)
{
    PvtRemovePrimObserver observer = PvtRemovePrimObserver(callback, userData);
    _removePrimObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addRenamePrimCallback(RtRenamePrimCallbackFunc callback, void* userData)
{
    PvtRenamePrimObserver observer = PvtRenamePrimObserver(callback, userData);
    _renamePrimObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addReparentPrimCallback(RtReparentPrimCallbackFunc callback, void* userData)
{
    PvtReparentPrimObserver observer = PvtReparentPrimObserver(callback, userData);
    _reparentPrimObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData)
{
    PvtSetAttributeObserver observer = PvtSetAttributeObserver(callback, userData);
    _setAttrObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addSetMetadataCallback(RtSetMetadataCallbackFunc callback, void* userData)
{
    PvtSetMetadataObserver observer = PvtSetMetadataObserver(callback, userData);
    _setMetadataObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addRemoveMetadataCallback(RtRemoveMetadataCallbackFunc callback, void* userData)
{
    PvtRemoveMetadataObserver observer = PvtRemoveMetadataObserver(callback, userData);
    _removeMetadataObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addConnectionCallback(RtConnectionCallbackFunc callback, void* userData)
{
    PvtConnectionObserver observer = PvtConnectionObserver(callback, userData);
    _connectionObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

RtCallbackId PvtMessageHandler::addRelationshipCallback(RtRelationshipCallbackFunc callback, void* userData)
{
    PvtRelationshipObserver observer = PvtRelationshipObserver(callback, userData);
    _relationshipObservers[_callbackIdCounter] = observer;
    _callbackIdToType[_callbackIdCounter] = observer.type;
    return _callbackIdCounter++;
}

void PvtMessageHandler::removeCallback(RtCallbackId id)
{
    auto it = _callbackIdToType.find(id);
    if (it != _callbackIdToType.end())
    {
        // Remove the corresponding observer
        switch (it->second)
        {
        case PvtMessageType::CREATE_PRIM:
            _createPrimObservers.erase(id);
            break;
        case PvtMessageType::REMOVE_PRIM:
            _removePrimObservers.erase(id);
            break;
        case PvtMessageType::RENAME_PRIM:
            _renamePrimObservers.erase(id);
            break;
        case PvtMessageType::REPARENT_PRIM:
            _reparentPrimObservers.erase(id);
            break;
        case PvtMessageType::SET_ATTRIBUTE:
            _setAttrObservers.erase(id);
            break;
        case PvtMessageType::SET_METADATA:
            _setMetadataObservers.erase(id);
            break;
        case PvtMessageType::REMOVE_METADATA:
            _removeMetadataObservers.erase(id);
            break;
        case PvtMessageType::CHANGE_CONNECTION:
            _connectionObservers.erase(id);
            break;
        case PvtMessageType::CHANGE_RELATIONSHIP:
            _relationshipObservers.erase(id);
            break;
        default:
            throw ExceptionRuntimeError("Removal of callback faild! Message type '" + std::to_string(size_t(it->second)) + "' has not been implemented properly.");
        }

        // Remove from the message type map
        _callbackIdToType.erase(id);
    }
}

void PvtMessageHandler::sendCreatePrimMessage(RtStagePtr stage, const RtPrim& prim)
{
    for (auto observer : _createPrimObservers)
    {
        observer.second.callback(stage, prim, observer.second.userData);
    }
}

void PvtMessageHandler::sendRemovePrimMessage(RtStagePtr stage, const RtPrim& prim)
{
    for (auto observer : _removePrimObservers)
    {
        observer.second.callback(stage, prim, observer.second.userData);
    }
}

void PvtMessageHandler::sendRenamePrimMessage(RtStagePtr stage, const RtPrim& prim, const RtToken& newName)
{
    for (auto observer : _renamePrimObservers)
    {
        observer.second.callback(stage, prim, newName, observer.second.userData);
    }
}

void PvtMessageHandler::sendReparentPrimMessage(RtStagePtr stage, const RtPrim& prim, const RtPath& newPath)
{
    for (auto observer : _reparentPrimObservers)
    {
        observer.second.callback(stage, prim, newPath, observer.second.userData);
    }
}

void PvtMessageHandler::sendSetAttributeMessage(const RtAttribute& attr, const RtValue& value)
{
    for (auto observer : _setAttrObservers)
    {
        observer.second.callback(attr, value, observer.second.userData);
    }
}

void PvtMessageHandler::sendSetMetadataMessage(const RtObject &obj, const RtToken& name, const RtValue& value)
{
    for (auto observer : _setMetadataObservers)
    {
        observer.second.callback(obj, name, value, observer.second.userData);
    }
}

void PvtMessageHandler::sendRemoveMetadataMessage(const RtObject &obj, const RtToken& name)
{
    for (auto observer : _removeMetadataObservers)
    {
        observer.second.callback(obj, name, observer.second.userData);
    }
}

void PvtMessageHandler::sendConnectionMessage(const RtOutput& src, const RtInput& dest, ConnectionChange change)
{
    for (auto observer : _connectionObservers)
    {
        observer.second.callback(src, dest, change, observer.second.userData);
    }
}

void PvtMessageHandler::sendRelationshipMessage(const RtRelationship& rel, const RtObject& target, ConnectionChange change)
{
    for (auto observer : _relationshipObservers)
    {
        observer.second.callback(rel, target, change, observer.second.userData);
    }
}

}
