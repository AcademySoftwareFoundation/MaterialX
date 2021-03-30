//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXRuntime/RtPath.h>

namespace MaterialX
{

const string PvtPath::SEPARATOR("/");
const RtIdentifier PvtPath::ROOT_NAME("/");

void PvtPath::setObject(const PvtObject* obj)
{
    _elements.clear();

    PvtPrim* parent = obj->getParent();
    if (parent)
    {
        // Get the path from this child down to the root.
        _elements.push_back(obj->getName());
        while (parent)
        {
            _elements.push_back(parent->getName());
            parent = parent->getParent();
        }
        // Reverse it to go from root and up.
        std::reverse(_elements.begin(), _elements.end());
    }
    else
    {
        _elements.push_back(ROOT_NAME);
    }
}

}
