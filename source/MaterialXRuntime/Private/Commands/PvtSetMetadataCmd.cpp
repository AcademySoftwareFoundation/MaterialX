//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtSetMetadataCmd.h>
#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

PvtSetMetadataCmd::PvtSetMetadataCmd(const RtObject& obj, const RtToken& name, const RtValue& value)
    : PvtCommand()
    , _obj(obj)
    , _name(name)
    , _value(value)
    , _oldValue()
    , _metadataCreated(false)
{
}

PvtCommandPtr PvtSetMetadataCmd::create(const RtObject& obj, const RtToken& name, const RtValue& value)
{
    return std::make_shared<PvtSetMetadataCmd>(obj, name, value);
}

void PvtSetMetadataCmd::execute(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the metadata is changing
            msg().sendSetMetadataMessage(_obj, _name, _value);

            RtTypedValue* md = _obj.getMetadata(_name, RtType::STRING);

            // Do we need to create the metadata or does it already exist?
            if (!md)
            {
                md = _obj.addMetadata(_name, RtType::STRING);
                _metadataCreated = true;
            }

            // Save old value for undo/redo
            _oldValue = RtValue::clone(RtType::STRING, md->getValue(), _obj.getParent());

            md->setValue(_value);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to set metadata on is no longer valid"));
    }
}

void PvtSetMetadataCmd::undo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {

            if (_metadataCreated)
            {
                // Send message that the metadata is being removed
                msg().sendRemoveMetadataMessage(_obj, _name);

                _obj.removeMetadata(_name);
                _metadataCreated = false;
                result = RtCommandResult(true);
            }
            else
            {
                // Send message that the metadata is changing
                msg().sendSetMetadataMessage(_obj, _name, _oldValue);

                // Reset the value
                RtTypedValue* md = _obj.getMetadata(_name, RtType::STRING);
                if (md)
                {
                    md->setValue(_oldValue);
                    result = RtCommandResult(true);
                }
                else
                {
                    result = RtCommandResult(false, "Metadata is no longer valid");
                }
            }
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to set metadata on is no longer valid"));
    }
}

void PvtSetMetadataCmd::redo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the metadata is changing
            msg().sendSetMetadataMessage(_obj, _name, _value);

            RtTypedValue* md = _obj.getMetadata(_name, RtType::STRING);

            // Do we need to create the metadata or does it already exist?
            if (!md)
            {
                md = _obj.addMetadata(_name, RtType::STRING);
                _metadataCreated = true;
            }

            // Reset the value
            md->setValue(_value);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to set metadata on is no longer valid"));
    }
}

}
