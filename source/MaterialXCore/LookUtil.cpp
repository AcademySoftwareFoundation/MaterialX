//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/LookUtil.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

namespace
{

static void appendLooks(StringVec& destLookList, const StringSet& destLookSet, const StringVec& sourceLookList, const string& appendAfterLook)
{
    StringVec postList;
    if (!appendAfterLook.empty() && destLookSet.count(appendAfterLook))
    {
        StringVec prependList = destLookList;
        destLookList.clear();

        // Add destination looks first.
        bool prepend = true;
        for (const string& lookName : prependList)
        {
            if (prepend)
            {
                destLookList.push_back(lookName);
            }
            else
            {
                postList.push_back(lookName);
            }
            if (lookName == appendAfterLook)
            {
                prepend = false;
            }
        }
    }
    for (const string& lookName : sourceLookList)
    {
        if (!destLookSet.count(lookName))
        {
            destLookList.push_back(lookName);
        }
    }
    if (postList.size())
    {
        for (const string& lookName : postList)
        {
            destLookList.push_back(lookName);
        }
    }
}

}

LookVec getActiveLooks(const LookGroupPtr& lookGroup)
{
    string looks = lookGroup->getActiveLook();
    if (looks.empty())
    {
        looks = lookGroup->getLooks();
    }
    const StringVec& lookList = splitString(looks, ARRAY_VALID_SEPARATORS);
    LookVec activeLooks;
    if (!lookList.empty())
    {
        const LookVec& lookElements = lookGroup->getDocument()->getLooks();
        if (!lookElements.empty())
        {
            for (const string& lookName : lookList)
            {
                for (const auto& lookElement : lookElements)
                {
                    if (lookElement->getName() == lookName)
                    {
                        activeLooks.push_back(lookElement);
                    }
                }
            }
        }
    }
    return activeLooks;
}

void appendLookGroup(LookGroupPtr& lookGroup, const LookGroupPtr& lookGroupToAppend, const string& appendAfterLook)
{
    string sourceLooks = lookGroupToAppend->getLooks();
    if (sourceLooks.empty())
    {
        return;
    }

    // If look already exists in the look list then append if
    // not skipping duplicates
    const StringVec& sourceLookList = splitString(sourceLooks, ARRAY_VALID_SEPARATORS);
    StringVec destLookList = splitString(lookGroup->getLooks(), ARRAY_VALID_SEPARATORS);
    const StringSet destLookSet(destLookList.begin(), destLookList.end());

    appendLooks(destLookList, destLookSet, sourceLookList, appendAfterLook);

    // Append looks to "active" look list. Order does no matter.
    string activeSourceLooks = lookGroupToAppend->getActiveLook();
    const StringVec& sourceActiveLookList = splitString(activeSourceLooks, ARRAY_VALID_SEPARATORS);
    StringVec destActiveLookList = splitString(lookGroup->getActiveLook(), ARRAY_VALID_SEPARATORS);
    const StringSet destActiveLookSet(destActiveLookList.begin(), destActiveLookList.end());

    for (const string& activeLookName : sourceActiveLookList)
    {
        if (!destActiveLookSet.count(activeLookName))
        {
            destActiveLookList.push_back(activeLookName);
        }
    }

    // Update look and active look lists
    lookGroup->setLooks(mergeStringVec(destLookList, ARRAY_VALID_SEPARATORS));
    lookGroup->setActiveLook(mergeStringVec(destActiveLookList, ARRAY_VALID_SEPARATORS));
}

void appendLook(LookGroupPtr& lookGroup, const string& lookName, const string& appendAfterLook)
{
    if (lookName.empty())
    {
        return;
    }

    // If look already exists in the look list then append if
    // not skipping duplicates
    const StringVec& sourceLookList { lookName };
    StringVec destLookList = splitString(lookGroup->getLooks(), ARRAY_VALID_SEPARATORS);
    const StringSet destLookSet(destLookList.begin(), destLookList.end());

    appendLooks(destLookList, destLookSet, sourceLookList, appendAfterLook);

    StringVec destActiveLookList = splitString(lookGroup->getActiveLook(), ARRAY_VALID_SEPARATORS);
    const StringSet destActiveLookSet(destActiveLookList.begin(), destActiveLookList.end());

    if (destActiveLookSet.count(lookName) == 0)
    {
        destActiveLookList.push_back(lookName);
    }

    // Update look list
    lookGroup->setLooks(mergeStringVec(destLookList, ARRAY_VALID_SEPARATORS));
    lookGroup->setActiveLook(mergeStringVec(destActiveLookList, ARRAY_VALID_SEPARATORS));
}

LookPtr combineLooks(const LookGroupPtr& lookGroup)
{
    DocumentPtr document = lookGroup->getDocument();
    LookVec looks = getActiveLooks(lookGroup);
    if (looks.empty())
    {
        return nullptr;
    }

    // Create a new look as a copy of the first look
    LookPtr mergedLook = document->addLook();
    mergedLook->copyContentFrom(looks[0]);

    // Merge in subsequent looks if any
    for (size_t i=1; i<looks.size(); i++)
    {
        mergedLook->append(looks[i]);
    }
    const string& mergedLookName = mergedLook->getName();
    lookGroup->setActiveLook(mergedLookName);
    lookGroup->setLooks(mergedLookName);

    return mergedLook;
}

} // namespace MaterialX

