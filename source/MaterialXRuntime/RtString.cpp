//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtString.h>
#include <MaterialXRuntime/External/FarmHash/farmhash.h>

#include <cstring>
#include <mutex>

namespace MaterialX
{

struct RtStringRegistry
{
    typedef RtString::Entry Entry;

    const Entry* findEntry(size_t hash)
    {
        auto it = _table.find(hash);
        return it != _table.end() ? it->second : nullptr;
    }

    template<typename T>
    const Entry* getEntry(T s, size_t hash)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const Entry* entry = findEntry(hash);
        if (entry)
        {
            return entry;
        }
        entry = new Entry(s, hash);
        _table[hash] = entry;
        return entry;
    }

    const Entry* getEntryRaw(const char* s)
    {
        const size_t hash = farmhash::Hash(s, std::char_traits<char>::length(s));
        return getEntry(s, hash);
    }

    const Entry* getEntryStr(const string& s)
    {
        const size_t hash = farmhash::Hash(s.c_str(), s.size());
        return getEntry(s, hash);
    }

    static RtStringRegistry& get()
    {
        static RtStringRegistry _registry;
        return _registry;
    }

    // TODO: Improve to use multiple tables and mutexes
    std::unordered_map<size_t, const Entry*> _table;
    std::mutex _mutex;
};

RtString::RtString(const char* s) :
    _entry(RtStringRegistry::get().getEntryRaw(s))
{
}

RtString::RtString(const string& s) :
    _entry(RtStringRegistry::get().getEntryStr(s))
{
}

// Special case, defining the empty string.
const RtString RtString::EMPTY("");

const RtString RtString::DOC("doc");
const RtString RtString::NAME("name");
const RtString RtString::TYPE("type");
const RtString RtString::VALUE("value");
const RtString RtString::UNIFORM("uniform");
const RtString RtString::DEFAULTGEOMPROP("defaultgeomprop");
const RtString RtString::INTERNALGEOMPROPS("internalgeomprops");
const RtString RtString::ENUM("enum");
const RtString RtString::ENUMVALUES("enumvalues");
const RtString RtString::COLORSPACE("colorspace");
const RtString RtString::FILEPREFIX("fileprefix");
const RtString RtString::UINAME("uiname");
const RtString RtString::UICOLOR("uicolor");
const RtString RtString::UIFOLDER("uifolder");
const RtString RtString::UIMIN("uimin");
const RtString RtString::UIMAX("uimax");
const RtString RtString::UISOFTMIN("uisoftmin");
const RtString RtString::UISOFTMAX("uisoftmax");
const RtString RtString::UISTEP("uistep");
const RtString RtString::UIADVANCED("uiadvanced");
const RtString RtString::UIVISIBLE("uivisible");
const RtString RtString::XPOS("xpos");
const RtString RtString::YPOS("ypos");
const RtString RtString::UNIT("unit");
const RtString RtString::UNITTYPE("unittype");
const RtString RtString::MEMBER("member");
const RtString RtString::CHANNELS("channels");
const RtString RtString::INHERIT("inherit");
const RtString RtString::MATERIAL("material");
const RtString RtString::COLLECTION("collection");
const RtString RtString::GEOM("geom");
const RtString RtString::EXCLUSIVE("exclusive");
const RtString RtString::MATERIALASSIGN("materialassign");
const RtString RtString::ENABLEDLOOKS("enabled");
const RtString RtString::FORMAT("format");
const RtString RtString::FILE("file");
const RtString RtString::FRAGMENT("fragment");
const RtString RtString::FUNCTION("function");
const RtString RtString::LOOK("look");
const RtString RtString::LOOKS("looks");
const RtString RtString::INCLUDEGEOM("includegeom");
const RtString RtString::EXCLUDEGEOM("excludegeom");
const RtString RtString::INCLUDECOLLECTION("includecollection");
const RtString RtString::CONTAINS("contains");
const RtString RtString::MINIMIZED("minimized");
const RtString RtString::WIDTH("width");
const RtString RtString::HEIGHT("height");
const RtString RtString::BITDEPTH("bitdepth");
const RtString RtString::IMPLNAME("implname");
const RtString RtString::KIND("kind");
const RtString RtString::NAMESPACE("namespace");
const RtString RtString::NODE("node");
const RtString RtString::NODEDEF("nodedef");
const RtString RtString::NODEGROUP("nodegroup");
const RtString RtString::NODEIMPL("nodeimpl");
const RtString RtString::NOTE("note");
const RtString RtString::SHADER("shader");
const RtString RtString::SOURCECODE("sourcecode");
const RtString RtString::TARGET("target");
const RtString RtString::VERSION("version");
const RtString RtString::ISDEFAULTVERSION("isdefaultversion");
const RtString RtString::DEFAULT("default");
const RtString RtString::DEFAULTINPUT("defaultinput");
const RtString RtString::UNKNOWN("unknown");

}
