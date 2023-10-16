//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OVERRIDE_H
#define MATERIALX_OVERRIDE_H

/// @file
/// Material property override classes

#include <MaterialXCore/Export.h>
#include <MaterialXCore/Value.h>

MATERIALX_NAMESPACE_BEGIN

class Document;
class Element;
class Input;
class Value;

using OverridePtr = shared_ptr<class Override>;

/// @class Override
/// Specifies a list of properties (names and values) that override the input values for a document.
class MX_CORE_API Override
{
  public:
    Override(
        std::shared_ptr<Document> doc,
        const vector<string>& properties,
        const vector<ValuePtr>& values = {});
    ~Override() { }

    /// Create an override for this document with a list of properties.
    /// @param doc The document to override.
    /// @param properties Vector of property names, each one must match the path name of an input in the document.
    /// @param values Optional vector of values to override the ones in the document, length must match properties.  If not provided values are initialized with defaults from the document.
    /// @return Pointer to the new override object.
    static OverridePtr create(
        std::shared_ptr<Document> doc,
        const vector<string>& properties,
        const vector<ValuePtr>& values = {})
    {
        return std::make_shared<Override>(doc, properties, values);
    }


    /// Set the value of the named override property.
    /// @param name The name of the property to set.
    /// @param val The value to set, any valid MaterialX type.
    template <typename T>
    void setValue(const string& name, const T& val)
    {
        int idx = getIndex(name);
        _values[idx] = Value::createValue(val);
    }

    /// Get the index of the named property
    /// @param name The name of the property to lookup.
    /// @return The index (or -1 if property name is not found.)
    int getIndex(const string& propName)
    {
        auto iter = _indexLookup.find(propName);
        if (iter == _indexLookup.end())
            return -1;

        return iter->second;
    }

    /// Get the document for this override.
    /// @return The document being overriden.
    std::shared_ptr<Document> getDocument() { return _doc; }

    /// Get the document input for the Nth property being overriden.
    /// @return Pointer to the input within the document.
    std::shared_ptr<Input> getPropertyInput(int n) { return _propertyInputs[n]; }

    /// Get the current value of of the Nth property being overriden.
    /// @return The current value.
    ValuePtr getValue(int n) { return _values[n]; }

    /// Get the name of the Nth property being overriden.
    /// @return The full path name of the input associated with the override property.
    string getPropertyName(int n) { return _properties[n]; }

    /// Get the number of properties in this override.
    /// @return The property count.
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
