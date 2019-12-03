//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvPath.h>

namespace MaterialX
{

PrvPath::PrvPath() :
    _root(nullptr)
{
}

PrvPath::PrvPath(PrvObjectHandle obj) :
    _root(nullptr)
{
    setObject(obj);
}

PrvObjectHandle PrvPath::getObject() const
{
    if (!_root || _path.empty())
    {
        return nullptr;
    }

    PrvElement* parent = _root->asA<PrvElement>();
    PrvObjectHandle elem = nullptr;
    size_t i = 0;
    while (parent)
    {
        elem = parent->findChildByName(_path[i++]);
        parent = elem && (i < _path.size()) ? elem->asA<PrvElement>() : nullptr;
    }

    return elem;
}

void PrvPath::setObject(PrvObjectHandle obj)
{
    if (!(obj && obj->hasApi(RtApiType::ELEMENT)))
    {
        throw ExceptionRuntimeError("Cannot construct path, given object is not a valid element");
    }

    PrvElement* elem = obj->asA<PrvElement>();
    PrvElement* root = elem->getRoot();
    _root = root ? root->shared_from_this() : nullptr;

    PrvElement* parent = elem->getParent();

    // Get the path from child down to parent and then reverse it
    _path.push_back(elem->getName());
    while (parent)
    {
        if (parent == root)
        {
            // Stop when reaching the root.
            break;
        }
        _path.push_back(parent->getName());
        parent = parent->getParent();
    }
    std::reverse(_path.begin(), _path.end());
}

}
