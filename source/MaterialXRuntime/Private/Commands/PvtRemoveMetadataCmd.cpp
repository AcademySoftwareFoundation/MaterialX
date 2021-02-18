//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtRemoveMetadataCmd.h>
#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

PvtRemoveMetadataCmd::PvtRemoveMetadataCmd(const RtObject& obj, const RtToken& name)
    : PvtCommand()
    , _obj(obj)
    , _name(name)
    , _oldValue()
{
}

PvtCommandPtr PvtRemoveMetadataCmd::create(const RtObject& obj, const RtToken& name)
{
    return std::make_shared<PvtRemoveMetadataCmd>(obj, name);
}

void PvtRemoveMetadataCmd::execute(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            RtTypedValue* md = _obj.getMetadata(_name, RtType::STRING);
            if (md)
            {
                // Send message that the metadata is being removed
                msg().sendRemoveMetadataMessage(_obj, _name);

                // Save old value for undo/redo
                _oldValue = RtValue::clone(RtType::STRING, md->getValue(), _obj.getParent());

                // Remove the metadata value
                _obj.removeMetadata(_name);
                result = RtCommandResult(true);
            }
            else
            {
                result = RtCommandResult(false, string("Object does not have specified metadata"));
            }
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to remove metadata from is no longer valid"));
    }
}

void PvtRemoveMetadataCmd::undo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the metadata is being set
            msg().sendSetMetadataMessage(_obj, _name, _oldValue);

            RtTypedValue* md = _obj.addMetadata(_name, RtType::STRING);
            md->setValue(_oldValue);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Node to add metadata to is no longer valid"));
    }
}

void PvtRemoveMetadataCmd::redo(RtCommandResult& result)
{
   if (_obj.isValid())
    {
        try
        {
            // Send message that the metadata is being removed
            msg().sendRemoveMetadataMessage(_obj, _name);

            // Remove the metadata value
            _obj.removeMetadata(_name);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Node to remove metadata from is no longer valid"));
    }
}

}
