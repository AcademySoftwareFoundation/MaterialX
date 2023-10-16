//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Document.h>

#include <MaterialXCore/Override.h>
#include <map>


MATERIALX_NAMESPACE_BEGIN

Override::Override(
    std::shared_ptr<Document> doc,
    const vector<string>& properties,
    const vector<ValuePtr>& values) :
    _doc(doc),
    _properties(properties),
    _values(values)
{
    // Should we get default input values from document?
    bool getValues = _values.size()==0;

    // Find the document inputs associated with the override properties.
    // The material inputs are at the start of the traversed tree, so a linear search is most efficent.
    _propertyInputs.resize(_properties.size());
    // Count how many properties have been found.
    int foundProps = 0;
    for (MaterialX::ElementPtr elem : _doc->traverseTree())
    {
        if (elem->isA<MaterialX::Input>())
        {
            MaterialX::InputPtr pInput = elem->asA<MaterialX::Input>();
            for (int i = 0; i < properties.size(); i++)
            {
                if (pInput->getNamePath().compare(properties[i])==0)
                {
                    _propertyInputs[i] = pInput;
                    foundProps++;
                }
            }

            // Stop the search once all properties have been found.
            if (foundProps == _propertyInputs.size())
                break;
        }
    }

    // Build the index lookup map and set defaults.
    for (int i = 0; i < properties.size(); i++)
    {
        _indexLookup[properties[i]] = i;
        if (getValues && _propertyInputs[i])
        {
            auto val = _propertyInputs[i]->getValue();
            if (val)
                _values.push_back(val->copy());
            else
                _values.push_back(Value::createValue(nullptr));

        }
    }

}

MATERIALX_NAMESPACE_END