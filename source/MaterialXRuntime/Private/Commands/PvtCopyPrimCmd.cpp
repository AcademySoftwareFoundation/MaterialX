//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtCopyPrimCmd.h>
#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPort.h>
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
    copyMetadata(PvtObject::cast(prim), PvtObject::cast(copy));

    // Copy any inputs & outputs that don't exists by default.
    RtNodeGraph ng(prim);
    if (ng)
    {
        // Special handling for nodegraphs which needs to call the nodegraph version of createInput / createOutput.
        RtNodeGraph ngCopy(copy);
        for (size_t i = 0; i < ng.prim()->numInputs(); ++i)
        {
            const PvtInput* port = ng.prim()->getInput(i);
            RtInput portCopy = ngCopy.getInput(port->getName());
            if (!portCopy)
            {
                portCopy = ngCopy.createInput(port->getName(), port->getType(), port->getFlags());
            }
            RtValue::copy(port->getType(), port->getValue(), portCopy.getValue());
            copyMetadata(port, PvtObject::cast(portCopy));
        }
        for (size_t i = 0; i < ng.prim()->numOutputs(); ++i)
        {
            const PvtOutput* port = ng.prim()->getOutput(i);
            RtOutput portCopy = ngCopy.getOutput(port->getName());
            if (!portCopy)
            {
                portCopy = ngCopy.createOutput(port->getName(), port->getType(), port->getFlags());
            }
            RtValue::copy(port->getType(), port->getValue(), portCopy.getValue());
            copyMetadata(port, PvtObject::cast(portCopy));
        }
    }
    else
    {
        for (size_t i = 0; i < prim.numInputs(); ++i)
        {
            const PvtInput* port = PvtPrim::cast<PvtInput>(prim.getInput(i));
            RtInput portCopy = copy.getInput(port->getName());
            if (!portCopy)
            {
                portCopy = copy.createInput(port->getName(), port->getType(), port->getFlags());
            }
            RtValue::copy(port->getType(), port->getValue(), portCopy.getValue());
            copyMetadata(port, PvtObject::cast(portCopy));
        }
        for (size_t i = 0; i < prim.numOutputs(); ++i)
        {
            const PvtOutput* port = PvtPrim::cast<PvtOutput>(prim.getOutput(i));
            RtOutput portCopy = copy.getOutput(port->getName());
            if (!portCopy)
            {
                portCopy = copy.createOutput(port->getName(), port->getType(), port->getFlags());
            }
            RtValue::copy(port->getType(), port->getValue(), portCopy.getValue());
            copyMetadata(port, PvtObject::cast(portCopy));
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

        RtNodeGraph ng2(copy);

        // Copy connections inbetween nodes
        // and between nodes and input interface.
        for (RtPrim child1 : prim.getChildren())
        {
            RtPrim child2 = copy.getChild(child1.getName());

            for (RtInput dest1 : child1.getInputs())
            {
                const RtOutput src1 = dest1.getConnection();
                if (src1)
                {
                    RtInput dest2 = child2.getInput(dest1.getName());
                    if (!dest2.isConnected())
                    {
                        if (src1.isSocket())
                        {
                            // Prim must be a nodegraph so connect to the corresponding socket.
                            RtOutput src2 = ng2.getInputSocket(src1.getName());
                            src2.connect(dest2);
                        }
                        else
                        {
                            RtPrim src2Node = copy.getChild(src1.getParent().getName());
                            RtOutput src2 = src2Node.getOutput(src1.getName());
                            src2.connect(dest2);
                        }
                    }
                }
            }
        }

        // Copy connections between nodes and output sockets.
        if (ng)
        {
            for (RtOutput output : ng.getOutputs())
            {
                RtInput dest1 = ng.getOutputSocket(output.getName());
                if (dest1.isConnected())
                {
                    RtOutput src1 = dest1.getConnection();
                    RtInput dest2 = ng2.getOutputSocket(output.getName());
                    RtPrim src2Node = ng2.getNode(src1.getParent().getName());
                    RtOutput src2 = src2Node.getOutput(src1.getName());
                    src2.connect(dest2);
                }
            }
        }
    }

    return copy;
}

void PvtCopyPrimCmd::copyMetadata(const PvtObject* src, PvtObject* dest)
{
    for (const RtString& name : src->getAttributeNames())
    {
        const RtTypedValue* srcAttr = src->getAttribute(name);
        RtTypedValue* destAttr = dest->createAttribute(name, srcAttr->getType());
        RtValue::copy(srcAttr->getType(), srcAttr->getValue(), destAttr->getValue());
    }
}

}
