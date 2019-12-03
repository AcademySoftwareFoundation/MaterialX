//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvElement.h>
#include <MaterialXRuntime/Private/PrvNode.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

const string PrvElement::PATH_SEPARATOR = "/";

PrvElement::PrvElement(RtObjType objType, const RtToken& name) :
    PrvObject(objType),
    _name(name),
    _parent(nullptr)
{
}

RtToken PrvElement::makeUniqueChildName(const RtToken& name) const
{
    RtToken newName = name;
    if (findChildByName(name))
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
        do {
            newName = baseName + std::to_string(i++);
        } while (findChildByName(newName));
    }
    return newName;
}

void PrvElement::setName(const RtToken& name)
{
    if (name != _name)
    {
        RtToken uniqueName = name;
        if (_parent)
        {
            uniqueName = _parent->makeUniqueChildName(name);
            _parent->_childrenByName.erase(_name);
            _parent->_childrenByName[uniqueName] = shared_from_this();
        }
        _name = uniqueName;
    }
}

PrvElement* PrvElement::getRoot() const
{
    PrvElement* root = const_cast<PrvElement*>(this);
    while (root->_parent)
    {
        root = root->_parent;
    }
    return root;
}

void PrvElement::addChild(PrvObjectHandle elemH)
{
    if (!elemH->hasApi(RtApiType::ELEMENT))
    {
        throw ExceptionRuntimeError("Given object is not a valid element");
    }

    PrvElement* elem = elemH->asA<PrvElement>();
    PrvElement* parent = elem->getParent();
    if (parent)
    {
        if (parent == this)
        {
            // We are already a parent to this child.
            return;
        }
        // We must remove the element from the old parent
        // as elements can't have multiple parents.
        parent->removeChild(elem->getName());
    }

    // Make sure the element name is unique within the parent scope.
    const RtToken uniqueName = makeUniqueChildName(elem->getName());
    elem->setName(uniqueName);

    elem->setParent(this);
    _children.push_back(elemH);
    _childrenByName[elem->getName()] = elemH;
}

void PrvElement::removeChild(const RtToken& name)
{
    for (auto it = _children.begin(); it != _children.end(); ++it)
    {
        PrvElement* child = (*it)->asA<PrvElement>();
        if (child->getName() == name)
        {
            _children.erase(it);
            child->setParent(nullptr);
            break;
        }
    }
    _childrenByName.erase(name);
}

PrvObjectHandle PrvElement::findChildByName(const RtToken& name) const
{
    auto it = _childrenByName.find(name);
    return it != _childrenByName.end() ? it->second : nullptr;
}

PrvObjectHandle PrvElement::findChildByPath(const string& path) const
{
    const StringVec elementNames = splitString(path, PATH_SEPARATOR);
    if (elementNames.empty())
    {
        return nullptr;
    }

    size_t i = 0;
    RtToken name = RtToken(elementNames[i++]);
    PrvObjectHandle elem = findChildByName(name);

    while (elem != nullptr && i < elementNames.size())
    {
        name = RtToken(elementNames[i++]);
        if (elem->hasApi(RtApiType::NODE))
        {
            PrvNode* node = elem->asA<PrvNode>();
            PrvNodeDef* nodedef = node->getNodeDef()->asA<PrvNodeDef>();
            elem = nodedef->findChildByName(name);
        }
        else
        {
            elem = elem->asA<PrvElement>()->findChildByName(name);
        }
    }

    return elem;
}

RtAttribute* PrvElement::addAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    auto it = _attributesByName.find(name);
    if (it != _attributesByName.end())
    {
        throw ExceptionRuntimeError("An attribute named '" + name.str() + "' already exists for '" + getName().str() + "'");
    }

    AttrPtr attr(new RtAttribute(name, type, getObject(), flags));
    _attributes.push_back(attr);
    _attributesByName[name] = attr;

    return attr.get();
}

void PrvElement::removeAttribute(const RtToken& name)
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

PrvAllocator& PrvElement::getAllocator()
{
    if (!_parent)
    {
        throw ExceptionRuntimeError("Trying to get allocator for an element with no allocator and no parent: '" + getName().str() + "'");
    }
    return _parent->getAllocator();
}

PrvUnknownElement::PrvUnknownElement(const RtToken& name, const RtToken& category) :
    PrvElement(RtObjType::UNKNOWN, name),
    _category(category)
{
}

PrvObjectHandle PrvUnknownElement::createNew(PrvElement* parent, const RtToken& name, const RtToken& category)
{
    PrvObjectHandle elem(new PrvUnknownElement(name, category));
    if (parent)
    {
        parent->addChild(elem);
    }
    return elem;
}

}
