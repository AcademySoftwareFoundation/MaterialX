//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPath.h>

namespace MaterialX
{

const string PvtPath::SEPARATOR = "/";

PvtDataHandle PvtPath::getObject() const
{
    if (!_root)
    {
        return nullptr;
    }
    if (_path.empty())
    {
        return _root;
    }

    PvtElement* parent = _root->asA<PvtElement>();
    PvtDataHandle elem = nullptr;
    size_t i = 0;
    while (parent)
    {
        elem = parent->findChildByName(_path[i++]);
        parent = elem && (i < _path.size()) ? elem->asA<PvtElement>() : nullptr;
    }

    return elem;
}

void PvtPath::setObject(PvtDataHandle obj)
{
    if (!(obj && obj->hasApi(RtApiType::ELEMENT)))
    {
        throw ExceptionRuntimeError("Cannot construct path, given object is not a valid element");
    }

    PvtElement* elem = obj->asA<PvtElement>();
    PvtElement* root = elem->getRoot();
    _root = root ? root->shared_from_this() : nullptr;

    PvtElement* parent = elem->getParent();
    if (parent)
    {
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

}
