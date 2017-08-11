#ifndef MATERIALXFORMAYA_TYPES_H
#define MATERIALXFORMAYA_TYPES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <exception>
#include <memory>

using std::string;
using std::vector;
using std::unordered_map;
using std::set;
using std::pair;
using std::shared_ptr;
using std::make_shared;

struct TranslatorContext;
class NodeTranslator;

/// The base class for exceptions used for error handling
class Exception : public std::exception
{
public:
    Exception(const string& msg) :
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

#endif // MATERIALXFORMAYA_TYPES_H
