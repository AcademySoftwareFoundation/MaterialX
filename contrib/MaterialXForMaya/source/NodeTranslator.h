#ifndef MATERIALXFORMAYA_NODETRANSLATOR_H
#define MATERIALXFORMAYA_NODETRANSLATOR_H

// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#include <ExporterTypes.h>

#include <maya/MStatus.h>
#include <maya/MObjectHandle.h>
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Document.h>
#include <set>

namespace mx = MaterialX;

namespace MaterialXForMaya
{

using NodeTranslatorPtr = shared_ptr<class NodeTranslator>;

/// @class NodeTranslator
/// The base class for node translators.
class NodeTranslator
{
public:
    virtual ~NodeTranslator() {}

    /// Initialize with translator data
    virtual void initialize(const MObject& mayaNode, mx::ConstDocumentPtr data);

    /// Return the type name for this transaltor.
    virtual const string& getTypeName() const = 0;

    /// Export a node def for the given Maya node.
    virtual mx::NodeDefPtr exportNodeDef(const MObject& mayaNode, const std::string& outputType, TranslatorContext& context);

    /// Export a node instance for the given Maya node.
    virtual mx::NodePtr exportNode(const MObject& mayaNode, const std::string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context);

    /// Return false if attribute of given name should be ignored.
    virtual bool shouldExport(const string& mayaAttrName) const;

    /// Return false if the given plug should be ignored, considering 
    /// the MaterialX default value. Can be used to ignore plugs where 
    /// the value hasn't changed.
    virtual bool shouldExport(const MPlug& mayaPlug, mx::ValuePtr defaultValue) const;

    /// Return true if connections to this node type should be exported by value
    virtual bool exportByValue() const;

    /// Return the Maya name for the given MaterialX name
    /// in case there is a rename registered for it.
    string getMayaName(const string& mxName) const
    {
        auto it = _translatorData->mxToMaya.find(mxName);
        if (it != _translatorData->mxToMaya.end())
        {
            return it->second;
        }
        return mxName;
    }

protected:
    // Protected constructor
    NodeTranslator();

    string getNodeName(const MObject& mayaNode, const std::string& outputType);

    enum PortType
    {
        INPUT_PORT,
        PARAMETER_PORT,
        OUTPUT_PORT
    };

    struct Attribute
    {
        int portType;
        string name;
        string type;
        string value;
        Attribute() {}
        Attribute(int pt, const string& n, const string& t, const string& v = "") 
            : portType(pt), name(n), type(t), value(v)
        {}
    };

    struct TranslatorData
    {
        bool exportByValue;
        string mayaNodeType;
        string mxNodeType;
        string mxDataType;
        string mxNodeDef;
        vector<Attribute> attributes;
        unordered_map<string, string> mxToMaya;
        TranslatorData(const MObject& mayaNode, mx::ConstDocumentPtr data);
    };

    using TranslatorDataPtr = shared_ptr<TranslatorData>;

    TranslatorDataPtr _translatorData;

    friend class Plugin;
    static set<string> _attributeIgnoreList;
};

// Macro declaring required methods and members for a custom node translator
#define DECLARE_NODE_TRANSLATOR(T)                                       \
    public:                                                              \
        const string& getTypeName() const override { return _typeName; } \
        static const string& typeName() { return _typeName; }            \
        static NodeTranslatorPtr creator() { return make_shared<T>(); }  \
    private:                                                             \
        static const string _typeName;                                   \

// Macro defining required methods and members for a custom node translator
#define DEFINE_NODE_TRANSLATOR(T, NAME)                                  \
    const string T::_typeName = NAME;                                    \

/// @class DefaultNodeTranslator
/// Declare a node translator to be used by default
/// for all nodes that has no custom translator registered.
class DefaultNodeTranslator : public NodeTranslator
{
    DECLARE_NODE_TRANSLATOR(DefaultNodeTranslator)
};

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_NODETRANSLATOR_H
