//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtMessage.h>

#include <MaterialXRuntime/Private/PvtApi.h>

namespace MaterialX
{

RtCallbackId RtMessage::addCreatePrimCallback(RtCreatePrimCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addCreatePrimCallback(callback, userData);
}

RtCallbackId RtMessage::addRemovePrimCallback(RtRemovePrimCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addRemovePrimCallback(callback, userData);
}

RtCallbackId RtMessage::addRenamePrimCallback(RtRenamePrimCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addRenamePrimCallback(callback, userData);
}

RtCallbackId RtMessage::addReparentPrimCallback(RtReparentPrimCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addReparentPrimCallback(callback, userData);
}

RtCallbackId RtMessage::addSetAttributeCallback(RtSetAttributeCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addSetAttributeCallback(callback, userData);
}

RtCallbackId RtMessage::addConnectionCallback(RtConnectionCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addConnectionCallback(callback, userData);
}

RtCallbackId RtMessage::addRelationshipCallback(RtRelationshipCallbackFunc callback, void* userData)
{
    return PvtApi::cast(RtApi::get())->getMessageHandler().addRelationshipCallback(callback, userData);
}

void RtMessage::removeCallback(RtCallbackId id)
{
    PvtApi::cast(RtApi::get())->getMessageHandler().removeCallback(id);
}

}
