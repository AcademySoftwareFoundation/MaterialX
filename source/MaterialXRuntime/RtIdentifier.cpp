//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtIdentifier.h>
#include <MaterialXRuntime/External/FarmHash/farmhash.h>

#include <cstring>
#include <mutex>

namespace MaterialX
{

const RtIdentifier EMPTY_IDENTIFIER("");

struct RtIdentifierRegistry
{
    typedef RtIdentifier::Entry Entry;

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

    static RtIdentifierRegistry& get()
    {
        static RtIdentifierRegistry _registry;
        return _registry;
    }

    // TODO: Improve to use multiple tables and mutexes
    std::unordered_map<size_t, const Entry*> _table;
    std::mutex _mutex;
};

const RtIdentifier::Entry RtIdentifier::NULL_ENTRY("", 0);

RtIdentifier::RtIdentifier(const char* s) :
    _entry(RtIdentifierRegistry::get().getEntryRaw(s))
{
}

RtIdentifier::RtIdentifier(const string& s) :
    _entry(RtIdentifierRegistry::get().getEntryStr(s))
{
}

}
