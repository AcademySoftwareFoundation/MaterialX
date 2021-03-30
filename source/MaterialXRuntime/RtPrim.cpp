//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(RtPrim, RtObjType::PRIM, "RtPrim")

RtPrim::RtPrim(PvtObjHandle hnd) :
    RtObject(hnd)
{
}

RtPrim::RtPrim(RtObject obj) :
    RtObject(obj)
{
}

const RtTypeInfo* RtPrim::getTypeInfo() const
{
    return hnd()->asA<PvtPrim>()->getTypeInfo();
}

RtRelationship RtPrim::createRelationship(const RtIdentifier& name)
{
    return hnd()->asA<PvtPrim>()->createRelationship(name)->hnd();
}

void RtPrim::removeRelationship(const RtIdentifier& name)
{
    return hnd()->asA<PvtPrim>()->removeRelationship(name);
}

RtRelationship RtPrim::getRelationship(const RtIdentifier& name) const
{
    PvtRelationship* rel = hnd()->asA<PvtPrim>()->getRelationship(name);
    return rel ? rel->hnd() : RtRelationship();
}

RtRelationshipIterator RtPrim::getRelationships() const
{
    return RtRelationshipIterator(*this);
}

RtPort RtPrim::getPort(const RtIdentifier& name) const
{
    PvtPort* port = hnd()->asA<PvtPrim>()->getPort(name);
    return port ? port->hnd() : RtPort();
}

RtInput RtPrim::createInput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
{
    return hnd()->asA<PvtPrim>()->createInput(name, type, flags)->hnd();
}

void RtPrim::removeInput(const RtIdentifier& name)
{
    return hnd()->asA<PvtPrim>()->removeInput(name);
}

size_t RtPrim::numInputs() const
{
    return hnd()->asA<PvtPrim>()->numInputs();
}

RtInput RtPrim::getInput(size_t index) const
{
    PvtInput* input = hnd()->asA<PvtPrim>()->getInput(index);
    return input ? input->hnd() : RtInput();
}

RtInput RtPrim::getInput(const RtIdentifier& name) const
{
    PvtInput* input = hnd()->asA<PvtPrim>()->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtInputIterator RtPrim::getInputs() const
{
    return RtInputIterator(*this);
}

RtOutput RtPrim::createOutput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
{
    return hnd()->asA<PvtPrim>()->createOutput(name, type, flags)->hnd();
}

void RtPrim::removeOutput(const RtIdentifier& name)
{
    return hnd()->asA<PvtPrim>()->removeOutput(name);
}

size_t RtPrim::numOutputs() const
{
    return hnd()->asA<PvtPrim>()->numOutputs();
}

RtOutput RtPrim::getOutput(size_t index) const
{
    PvtOutput* output = hnd()->asA<PvtPrim>()->getOutput(index);
    return output ? output->hnd() : RtOutput();
}

RtOutput RtPrim::getOutput(const RtIdentifier& name) const
{
    PvtOutput* output = hnd()->asA<PvtPrim>()->getOutput(name);
    return output ? output->hnd() : RtOutput();
}

RtOutputIterator RtPrim::getOutputs() const
{
    return RtOutputIterator(*this);
}

size_t RtPrim::numChildren() const
{
    return hnd()->asA<PvtPrim>()->getAllChildren().size();
}

RtPrim RtPrim::getChild(size_t index) const
{
    PvtPrim* child = hnd()->asA<PvtPrim>()->getChild(index);
    return child ? child->hnd() : RtPrim();
}

RtPrim RtPrim::getChild(const RtIdentifier& name) const
{
    PvtPrim* child = hnd()->asA<PvtPrim>()->getChild(name);
    return child ? child->hnd() : RtPrim();
}

RtPrimIterator RtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(*this, predicate);
}



RtAttributeSpec::RtAttributeSpec() :
    _ptr(new PvtAttributeSpec())
{
}

RtAttributeSpec::~RtAttributeSpec()
{
    delete static_cast<PvtAttributeSpec*>(_ptr);
}

const RtIdentifier& RtAttributeSpec::getName() const
{
    return static_cast<PvtAttributeSpec*>(_ptr)->name;
}

const RtIdentifier& RtAttributeSpec::getType() const
{
    return static_cast<PvtAttributeSpec*>(_ptr)->type;
}

const string& RtAttributeSpec::getValue() const
{
    return static_cast<PvtAttributeSpec*>(_ptr)->value;
}

bool RtAttributeSpec::isCustom() const
{
    return static_cast<PvtAttributeSpec*>(_ptr)->custom;
}

bool RtAttributeSpec::isExportable() const
{
    return static_cast<PvtAttributeSpec*>(_ptr)->exportable;
}

}
