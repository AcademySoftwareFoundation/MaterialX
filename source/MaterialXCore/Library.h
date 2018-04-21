//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LIBRARY_H
#define MATERIALX_LIBRARY_H

/// @file
/// Library-wide includes and types.  This file should be the first include for
/// any public header in the MaterialX library.

#include <algorithm>
#include <exception>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace MaterialX
{

using std::string;
using std::vector;
using std::shared_ptr;
using std::weak_ptr;

/// A vector of strings.
using StringVec = vector<string>;
/// An unordered map with strings as both keys and values.
using StringMap = std::unordered_map<string, string>;
/// A set of strings.
using StringSet = std::set<string>;

/// @class Exception
/// The base class for exceptions that are propagated from the MaterialX library
/// to the client application.
class Exception : public std::exception
{
  public:
    explicit Exception(const string& msg) :
        std::exception(),
        _msg(msg)
    {
    }

    Exception(const Exception& e) :
        std::exception(),
        _msg(e._msg)
    {
    }

    Exception& operator=(const Exception& e)
    {
        _msg = e._msg;
        return *this;
    }

    virtual ~Exception() throw()
    {
    }

    const char* what() const throw() override
    {
        return _msg.c_str();
    }

  private:
    string _msg;
};

} // namespace MaterialX

#endif
