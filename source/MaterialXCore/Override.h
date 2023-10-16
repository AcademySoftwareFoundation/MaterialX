//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OVERRIDE_H
#define MATERIALX_OVERRIDE_H

/// @file
/// Geometric element subclasses

#include <MaterialXCore/Export.h>
#include <MaterialXCore/Value.h>

MATERIALX_NAMESPACE_BEGIN

class Document;
class Element;
class Input;
class Value;

using OverridePtr = shared_ptr<class Override>;

/// @class Override
/// .
class MX_CORE_API Override
{
  public:
    Override(
        std::shared_ptr<Document> doc,
        const vector<string>& properties,
        const vector<ValuePtr>& values = {});
    ~Override() { }

    static OverridePtr create(
        std::shared_ptr<Document> doc,
        const vector<string>& properties,
        const vector<ValuePtr>& values = {})
    {
        return std::make_shared<Override>(doc, properties, values);
    }

    template<typename T>
    void setValue(const string& name, const T& val) {
        int idx = getIndex(name);
        _values[idx] = Value::createValue(val);
    }
    int getIndex(const string& propName) {
        return _indexLookup[propName];
    }
    std::shared_ptr<Document> getDocument() { return _doc; }

    std::shared_ptr<Input> getPropertyInput(int n) { return _propertyInputs[n]; }
    ValuePtr getValue(int n) { return _values[n]; }
    string getPropertyName(int n) { return _properties[n]; }

    int getPropertyCount() { return int(_properties.size()); }
  private:
    std::shared_ptr<Document> _doc;
    vector<string> _properties;
    vector<std::shared_ptr<Input>> _propertyInputs;
    vector<ValuePtr> _values;
    std::unordered_map<string, int> _indexLookup;
};

MATERIALX_NAMESPACE_END

#endif
