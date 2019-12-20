//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtElement.h>
#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

PvtElement::PvtElement(RtObjType objType, const RtToken& name) :
    PvtObject(objType),
    _name(name),
    _parent(nullptr)
{
}

void PvtElement::setName(const RtToken& name)
{
    if (name != _name)
    {
        RtToken uniqueName = name;
        if (_parent)
        {
            uniqueName = _parent->makeUniqueChildName(this, name);
            _parent->_childrenByName.erase(_name);
            _parent->_childrenByName[uniqueName] = shared_from_this();
        }
        _name = uniqueName;
    }
}

PvtElement* PvtElement::getRoot() const
{
    PvtElement* root = const_cast<PvtElement*>(this);
    while (root->_parent)
    {
        root = root->_parent;
    }
    return root;
}

void PvtElement::addChild(const PvtDataHandle& elemH)
{
    if (!elemH->hasApi(RtApiType::ELEMENT))
    {
        throw ExceptionRuntimeError("Given object is not a valid element");
    }

    PvtElement* elem = elemH->asA<PvtElement>();
    PvtElement* oldParent = elem->getParent();
    if (oldParent)
    {
        if (oldParent == this)
        {
            // We are already a parent to this child.
            return;
        }
        // We must remove the element from the old parent
        // as elements can't have multiple parents.
        oldParent->removeChild(elem->getName());
    }

    // Make sure the element name is unique within the parent scope.
    const RtToken uniqueName = makeUniqueChildName(elem, elem->getName());
    elem->setName(uniqueName);

    elem->setParent(this);
    _children.push_back(elemH);
    _childrenByName[elem->getName()] = elemH;
}

void PvtElement::removeChild(const RtToken& name)
{
    for (auto it = _children.begin(); it != _children.end(); ++it)
    {
        PvtElement* child = (*it)->asA<PvtElement>();
        if (child->getName() == name)
        {
            _children.erase(it);
            child->setParent(nullptr);
            break;
        }
    }
    _childrenByName.erase(name);
}

PvtDataHandle PvtElement::findChildByName(const RtToken& name) const
{
    auto it = _childrenByName.find(name);
    return it != _childrenByName.end() ? it->second : nullptr;
}

PvtDataHandle PvtElement::findChildByPath(const string& path) const
{
    const StringVec elementNames = splitString(path, PvtPath::SEPARATOR);
    if (elementNames.empty())
    {
        return nullptr;
    }

    size_t i = 0;
    RtToken name = RtToken(elementNames[i++]);
    PvtDataHandle elem = findChildByName(name);

    while (elem != nullptr && i < elementNames.size())
    {
        name = RtToken(elementNames[i++]);
        if (elem->hasApi(RtApiType::NODE))
        {
            PvtNode* node = elem->asA<PvtNode>();
            PvtNodeDef* nodedef = node->getNodeDef()->asA<PvtNodeDef>();
            elem = nodedef->findChildByName(name);
        }
        else
        {
            elem = elem->asA<PvtElement>()->findChildByName(name);
        }
    }

    return elem;
}

RtAttribute* PvtElement::addAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    auto it = _attributesByName.find(name);
    if (it != _attributesByName.end())
    {
        throw ExceptionRuntimeError("An attribute named '" + name.str() + "' already exists for '" + getName().str() + "'");
    }

    PvtAttributePtr attr = createAttribute(name, type, flags);
    _attributes.push_back(attr);
    _attributesByName[name] = attr;

    return attr.get();
}

void PvtElement::removeAttribute(const RtToken& name)
{
    for (auto it = _attributes.begin(); it != _attributes.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            _attributes.erase(it);
            break;
        }
    }
    _attributesByName.erase(name);
}

PvtAllocator& PvtElement::getAllocator()
{
    if (!_parent)
    {
        throw ExceptionRuntimeError("Trying to get allocator for an element with no allocator and no parent: '" + getName().str() + "'");
    }
    return _parent->getAllocator();
}

RtToken PvtElement::makeUniqueChildName(const PvtElement* child, const RtToken& name) const
{
    RtToken newName = name;

    // Check if there is another child with this name.
    PvtDataHandle otherChildH = findChildByName(name);
    if (otherChildH && otherChildH.get() != child)
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of("0123456789") + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other child with the resulting name.
        do {
            newName = baseName + std::to_string(i++);
            otherChildH = findChildByName(newName);
        } while (otherChildH && otherChildH.get() != child);
    }
    return newName;
}

PvtAttributePtr PvtElement::createAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return PvtAttributePtr(new RtAttribute(name, type, getObject(), flags));
}

PvtUnknownElement::PvtUnknownElement(const RtToken& name, const RtToken& category) :
    PvtElement(RtObjType::UNKNOWN, name),
    _category(category)
{
}

PvtDataHandle PvtUnknownElement::createNew(PvtElement* parent, const RtToken& name, const RtToken& category)
{
    PvtDataHandle elem(new PvtUnknownElement(name, category));
    if (parent)
    {
        parent->addChild(elem);
    }
    return elem;
}

}
