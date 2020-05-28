//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtCopyPrimCmd.h>
#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtAttribute.h>
#include <MaterialXRuntime/RtNodeGraph.h>

namespace MaterialX
{

PvtCommandPtr PvtCopyPrimCmd::create(RtStagePtr stage, const RtPrim& prim, const RtPath& parentPath)
{
    return std::make_shared<PvtCopyPrimCmd>(stage, prim, parentPath);
}

void PvtCopyPrimCmd::execute(RtCommandResult& result)
{
    try
    {
        _copy = createPrimCopy(_prim, _parentPath);

        // Send message that the prim has been created.
        msg().sendCreatePrimMessage(_stage, _copy);

        result = RtCommandResult(_copy.asA<RtObject>());
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtCopyPrimCmd::undo(RtCommandResult& result)
{
    try
    {
        // Send message that the prim is about to be removed.
        msg().sendRemovePrimMessage(_stage, _copy);

        // Remove the prim.
        _stage->disposePrim(_copy.getPath());

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtCopyPrimCmd::redo(RtCommandResult& result)
{
    try
    {
        // Create/restore the prim.
        _stage->restorePrim(_parentPath, _copy);

        // Send message that the prim has been created/restored.
        msg().sendCreatePrimMessage(_stage, _copy);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

RtPrim PvtCopyPrimCmd::createPrimCopy(const RtPrim& prim, const RtPath& parentPath)
{
    RtPrim copy;

    // Create the prim.
    if (prim.getTypeName() == RtNode::typeName())
    {
        RtNode node(prim);
        copy = _stage->createPrim(parentPath, prim.getName(), node.getNodeDef().getName());
    }
    else
    {
        copy = _stage->createPrim(parentPath, prim.getName(), prim.getTypeName());
    }

    // Copy metadata that don't exists by default.
    copyMetadata(PvtObject::hnd(prim)->asA<PvtObject>(), PvtObject::hnd(copy)->asA<PvtObject>());

    // Copy any inputs & outputs that don't exists by default.
    RtNodeGraph ng(prim);
    if (ng)
    {
        // Special handling for nodegraphs which needs to call the nodegraph version of create input/outputs.
        RtNodeGraph ngCopy(copy);
        for (RtAttribute attr : ng.getInputs())
        {
            const PvtInput* port = PvtObject::hnd(attr)->asA<PvtInput>();
            RtInput portCopy = ngCopy.getInput(port->getName());
            if (!portCopy)
            {
                portCopy = ngCopy.createInput(port->getName(), port->getType(), port->getFlags());
            }
            portCopy.setValue(port->getValue());
            copyMetadata(port, PvtObject::hnd(portCopy)->asA<PvtObject>());
        }
        for (RtAttribute attr : ng.getOutputs())
        {
            const PvtOutput* port = PvtObject::hnd(attr)->asA<PvtOutput>();
            RtOutput portCopy = ngCopy.getOutput(port->getName());
            if (!portCopy)
            {
                portCopy = ngCopy.createOutput(port->getName(), port->getType(), port->getFlags());
            }
            portCopy.setValue(port->getValue());
            copyMetadata(port, PvtObject::hnd(portCopy)->asA<PvtObject>());
        }
    }
    else
    {
        for (RtAttribute attr : prim.getInputs())
        {
            const PvtInput* port = PvtObject::hnd(attr)->asA<PvtInput>();
            RtInput portCopy = copy.getInput(port->getName());
            if (!portCopy)
            {
                portCopy = copy.createInput(port->getName(), port->getType(), port->getFlags());
            }
            portCopy.setValue(port->getValue());
            copyMetadata(port, PvtObject::hnd(portCopy)->asA<PvtObject>());
        }
        for (RtAttribute attr : prim.getOutputs())
        {
            const PvtOutput* port = PvtObject::hnd(attr)->asA<PvtOutput>();
            RtOutput portCopy = copy.getOutput(port->getName());
            if (!portCopy)
            {
                portCopy = copy.createOutput(port->getName(), port->getType(), port->getFlags());
            }
            portCopy.setValue(port->getValue());
            copyMetadata(port, PvtObject::hnd(portCopy)->asA<PvtObject>());
        }
    }

    // Copy any relationships that don't exists by default.
    for (RtObject rel : prim.getRelationships())
    {
        if (!copy.getRelationship(rel.getName()))
        {
            copy.createRelationship(rel.getName());
        }
    }

    if (prim.numChildren() > 0)
    {
        // Copy all child prims that don't exists by default.
        for (RtPrim child : prim.getChildren())
        {
            if (!copy.getChild(child.getName()))
            {
                createPrimCopy(child, copy.getPath());
            }
        }

        // Copy connections that don't exists by default.
        // Note that for a nodegraph prim this includes
        // any socket connections as the sockets are stored
        // on a dedicated socket child node.
        for (RtPrim child1 : prim.getChildren())
        {
            RtPrim child2 = copy.getChild(child1.getName());

            for (RtAttribute attr : child1.getInputs())
            {
                const RtInput dest1 = attr.asA<RtInput>();
                const RtOutput src1 = dest1.getConnection();
                if (src1)
                {
                    RtInput dest2 = child2.getInput(dest1.getName());
                    if (!dest2.isConnected())
                    {
                        RtPrim src2Node = copy.getChild(src1.getParent().getName());
                        RtOutput src2 = src2Node.getOutput(src1.getName());
                        src2.connect(dest2);
                    }
                }
            }
        }
    }

    return copy;
}

void PvtCopyPrimCmd::copyMetadata(const PvtObject* src, PvtObject* dest)
{
    const vector<RtToken>& metadataNames = src->getMetadataOrder();
    for (auto metadataName : metadataNames)
    {
        const RtTypedValue* metadataValue = src->getMetadata(metadataName);
        RtTypedValue* metadataValueCopy = dest->getMetadata(metadataName);
        if (!metadataValueCopy)
        {
            metadataValueCopy = dest->addMetadata(metadataName, metadataValue->getType());
        }
        metadataValueCopy->setValue(metadataValue->getValue());
    }
}

}
