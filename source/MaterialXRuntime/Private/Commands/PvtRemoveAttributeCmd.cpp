//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtRemoveAttributeCmd.h>
#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

PvtRemoveAttributeCmd::PvtRemoveAttributeCmd(const RtObject& obj, const RtString& name) :
    PvtCommand(),
    _obj(obj),
    _name(name),
    _oldValue()
{
}

PvtCommandPtr PvtRemoveAttributeCmd::create(const RtObject& obj, const RtString& name)
{
    return std::make_shared<PvtRemoveAttributeCmd>(obj, name);
}

void PvtRemoveAttributeCmd::execute(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            RtTypedValue* md = _obj.getAttribute(_name, RtType::STRING);
            if (md)
            {
                // Send message that the attribute is being removed
                msg().sendRemoveAttributeMessage(_obj, _name);

                // Save old value for undo/redo
                _oldValue = RtValue::clone(RtType::STRING, md->getValue(), _obj.getParent());

                // Remove the attribute value
                _obj.removeAttribute(_name);
                result = RtCommandResult(true);
            }
            else
            {
                result = RtCommandResult(false, string("Object does not have specified attribute"));
            }
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Object to remove attribute from is no longer valid"));
    }
}

void PvtRemoveAttributeCmd::undo(RtCommandResult& result)
{
    if (_obj.isValid())
    {
        try
        {
            // Send message that the attribute is being set
            msg().sendSetAttributeMessage(_obj, _name, _oldValue);

            RtTypedValue* md = _obj.createAttribute(_name, RtType::STRING);
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
        result = RtCommandResult(false, string("Node to add attribute to is no longer valid"));
    }
}

void PvtRemoveAttributeCmd::redo(RtCommandResult& result)
{
   if (_obj.isValid())
    {
        try
        {
            // Send message that the attribute is being removed
            msg().sendRemoveAttributeMessage(_obj, _name);

            // Remove the attribute value
            _obj.removeAttribute(_name);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Node to remove attribute from is no longer valid"));
    }
}

}
