//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtElement.h>

#include <MaterialXRuntime/Private/PvtElement.h>

namespace MaterialX
{

RtElement::RtElement(const RtObject& obj)
{
    setObject(obj);
}

RtApiType RtElement::getApiType() const
{
    return RtApiType::ELEMENT;
}

const RtToken& RtElement::getName() const
{
    return data()->asA<PvtElement>()->getName();
}

void RtElement::setName(const RtToken& name)
{
    data()->asA<PvtElement>()->setName(name);
}

RtObject RtElement::getParent() const
{
    PvtElement* parent = data()->asA<PvtElement>()->getParent();
    return parent ? PvtObject::object(parent->shared_from_this()) : RtObject();
}

RtObject RtElement::getRoot() const
{
    PvtElement* root = data()->asA<PvtElement>()->getRoot();
    return root ? PvtObject::object(root->shared_from_this()) : RtObject();
}

RtAttribute* RtElement::addAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtElement* elem = data()->asA<PvtElement>();
    return elem->addAttribute(name, type, flags);
}

const RtAttribute* RtElement::getAttribute(const RtToken& name) const
{
    PvtElement* elem = data()->asA<PvtElement>();
    return elem->getAttribute(name);
}

RtAttribute* RtElement::getAttribute(const RtToken& name)
{
    PvtElement* elem = data()->asA<PvtElement>();
    return elem->getAttribute(name);
}

const RtAttribute* RtElement::getAttribute(size_t index) const
{
    PvtElement* elem = data()->asA<PvtElement>();
    return elem->getAttribute(index);
}

RtAttribute* RtElement::getAttribute(size_t index)
{
    PvtElement* elem = data()->asA<PvtElement>();
    return elem->getAttribute(index);
}

size_t RtElement::numAttributes() const
{
    return data()->asA<PvtElement>()->numAttributes();
}

RtTreeIterator RtElement::traverseTree(RtTraversalFilter filter)
{
    return RtTreeIterator(getObject(), filter);
}

}
