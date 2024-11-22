/******************************************************************************
 * Copyright (c) 2021-2024, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

 // examples/mdl_sdk/shared/utils/strings.h
 //
 // Code shared by all examples

#ifndef EXAMPLE_SHARED_UTILS_STRINGS_H
#define EXAMPLE_SHARED_UTILS_STRINGS_H

#include <cstring>
#include <ctime>
#include <type_traits>
#include <stdint.h>
#include <locale>
#include <codecvt>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>

#include <mi/base/enums.h>
#include <mi/neuraylib/imdl_execution_context.h>

namespace mi { namespace examples { namespace strings
{
    /// convert std::string to std::wstring.
    inline std::wstring str_to_wstr(const std::string& s)
    {
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        return converter.from_bytes(s);
    }

    // --------------------------------------------------------------------------------------------

    /// convert std::wstring to std::string.
    inline std::string wstr_to_str(const std::wstring& s)
    {
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        return converter.to_bytes(s);
    }

    // --------------------------------------------------------------------------------------------

    /// Converts a wchar_t * string into an utf8 encoded string.
    inline std::string wchar_to_utf8(const wchar_t* src)
    {
        std::string res;

        for (wchar_t const *p = src; *p != L'\0'; ++p)
        {
            unsigned code = *p;

            if (code <= 0x7F)
            {
                // 0xxxxxxx
                res += char(code);
            }
            else if (code <= 0x7FF)
            {
                // 110xxxxx 10xxxxxx
                unsigned high = code >> 6;
                unsigned low  = code & 0x3F;
                res += char(0xC0 + high);
                res += char(0x80 + low);
            }
            else if (0xD800 <= code && code <= 0xDBFF && 0xDC00 <= p[1] && p[1] <= 0xDFFF)
            {
                // surrogate pair, 0x10000 to 0x10FFFF
                unsigned high = code & 0x3FF;
                unsigned low  = p[1] & 0x3FF;
                code = 0x10000 + ((high << 10) | low);

                if (code <= 0x10FFFF)
                {
                    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                    unsigned high = (code >> 18) & 0x07;
                    unsigned mh   = (code >> 12) & 0x3F;
                    unsigned ml   = (code >> 6) & 0x3F;
                    unsigned low  = code & 0x3F;
                    res += char(0xF0 + high);
                    res += char(0x80 + mh);
                    res += char(0x80 + ml);
                    res += char(0x80 + low);
                }
                else
                {
                    // error, replace by (U+FFFD) (or EF BF BD in UTF-8)
                    res += unsigned char(0xEF);
                    res += unsigned char(0xBF);
                    res += unsigned char(0xBD);
                }
                ++p;
            }
            else if (code <= 0xFFFF)
            {
                if (code < 0xD800 || code > 0xDFFF)
                {
                    // 1110xxxx 10xxxxxx 10xxxxxx
                    unsigned high   = code >> 12;
                    unsigned middle = (code >> 6) & 0x3F;
                    unsigned low    = code & 0x3F;
                    res += char(0xE0 + high);
                    res += char(0x80 + middle);
                    res += char(0x80 + low);
                }
                else
                {
                    // forbidden surrogate part, replace by (U+FFFD) (or EF BF BD in UTF-8)
                    res += unsigned char(0xEF);
                    res += unsigned char(0xBF);
                    res += unsigned char(0xBD);
                }
            }
            else if (code <= 0x10FFFF)
            {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                unsigned high = (code >> 18) & 0x07;
                unsigned mh   = (code >> 12) & 0x3F;
                unsigned ml   = (code >> 6) & 0x3F;
                unsigned low  = code & 0x3F;
                res += char(0xF0 + high);
                res += char(0x80 + mh);
                res += char(0x80 + ml);
                res += char(0x80 + low);
            }
            else
            {
                // error, replace by (U+FFFD) (or EF BF BD in UTF-8)
                res += unsigned char(0xEF);
                res += unsigned char(0xBF);
                res += unsigned char(0xBD);
            }
        }
        return res;
    }

    // --------------------------------------------------------------------------------------------

    namespace {

    // Converts one utf8 character to a utf32 encoded unicode character.
    inline char const *utf8_to_unicode_char(char const *up, unsigned &res)
    {
        bool error = false;
        unsigned char ch = up[0];

        // find start code: either 0xxxxxxx or 11xxxxxx
        while ((ch >= 0x80) && ((ch & 0xC0) != 0xC0)) {
            ++up;
            ch = up[0];
        }

        if (ch <= 0x7F) {
            // 0xxxxxxx
            res = ch;
            up += 1;
        } else if ((ch & 0xF8) == 0xF0) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            unsigned c1 = ch & 0x07; ch = up[1]; error |= (ch & 0xC0) != 0x80;
            unsigned c2 = ch & 0x3F; ch = up[2]; error |= (ch & 0xC0) != 0x80;
            unsigned c3 = ch & 0x3F; ch = up[3]; error |= (ch & 0xC0) != 0x80;
            unsigned c4 = ch & 0x3F;
            res = (c1 << 18) | (c2 << 12) | (c3 << 6) | c4;

            // must be U+10000 .. U+10FFFF
            error |= (res < 0x1000) || (res > 0x10FFFF);

            // Because surrogate code points are not Unicode scalar values, any UTF-8 byte
            // sequence that would otherwise map to code points U+D800..U+DFFF is illformed
            error |= (0xD800 <= res) && (res <= 0xDFFF);

            if (!error) {
                up += 4;
            } else {
                res = 0xFFFD;  // replacement character
                up += 1;
            }
        } else if ((ch & 0xF0) == 0xE0) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            unsigned c1 = ch & 0x0F; ch = up[1]; error |= (ch & 0xC0) != 0x80;
            unsigned c2 = ch & 0x3F; ch = up[2]; error |= (ch & 0xC0) != 0x80;
            unsigned c3 = ch & 0x3F;
            res = (c1 << 12) | (c2 << 6) | c3;

            // must be U+0800 .. U+FFFF
            error |= res < 0x0800;

            // Because surrogate code points are not Unicode scalar values, any UTF-8 byte
            // sequence that would otherwise map to code points U+D800..U+DFFF is illformed
            error |= (0xD800 <= res) && (res <= 0xDFFF);

            if (!error) {
                up += 3;
            } else {
                res = 0xFFFD;  // replacement character
                up += 1;
            }
        } else if ((ch & 0xE0) == 0xC0) {
            // 110xxxxx 10xxxxxx
            unsigned c1 = ch & 0x1F; ch = up[1]; error |= (ch & 0xC0) != 0x80;
            unsigned c2 = ch & 0x3F;
            res = (c1 << 6) | c2;

            // must be U+0080 .. U+07FF
            error |= res < 0x80;

            if (!error) {
                up += 2;
            } else {
                res = 0xFFFD;  // replacement character
                up += 1;
            }
        } else {
            // error
            res = 0xFFFD;  // replacement character
            up += 1;
        }
        return up;
    }

    } // namespace

    void utf16_append(std::wstring &s, unsigned c);

    // Convert the given char input of UTF-8 format into a wchar.
    inline std::wstring utf8_to_wchar(char const *src)
    {
        std::wstring res;

        // skip BOM
        if (    (static_cast<unsigned char>(src[0]) == 0xEFu)
            &&  (static_cast<unsigned char>(src[1]) == 0xBBu)
            &&  (static_cast<unsigned char>(src[2]) == 0xBFu))
            src += 3;

        while (*src != '\0') {
            unsigned unicode_char;

            src = utf8_to_unicode_char(src, unicode_char);
            utf16_append(res, unicode_char);
        }
        return res;
    }

    // Add an unicode utf32 character to an utf16 string.
    inline void utf16_append(std::wstring &s, unsigned c)
    {
        // assume only valid utf32 characters added
        if (c < 0x10000) {
            s += static_cast<wchar_t>(c);
        } else {
                // encode as surrogate pair
            c -= 0x10000;
            s += static_cast<wchar_t>((c >> 10) + 0xD800);
            s += static_cast<wchar_t>((c & 0x3FF) + 0xDC00);
        }
    }

    // --------------------------------------------------------------------------------------------

    /// split a string into chunks at a separation character.
    inline std::vector<std::string> split(
        const std::string& input,
        char sep)
    {
        std::vector<std::string> chunks;

        size_t offset(0);
        size_t pos(0);
        std::string chunk;
        while (pos != std::string::npos)
        {
            pos = input.find(sep, offset);

            if (pos == std::string::npos)
                chunk = input.substr(offset);
            else
                chunk = input.substr(offset, pos - offset);

            if (!chunk.empty())
                chunks.push_back(chunk);
            offset = pos + 1;
        }
        return chunks;
    }

    // --------------------------------------------------------------------------------------------

    /// split a string into chunks at a separation substring.
    inline std::vector<std::string> split(
        const std::string& input,
        const std::string& sep)
    {
        std::vector<std::string> chunks;

        size_t offset(0);
        while (true)
        {
            size_t pos = input.find(sep, offset);

            if (pos == std::string::npos)
            {
                chunks.push_back(input.substr(offset));
                break;
            }

            chunks.push_back(input.substr(offset, pos - offset));
            offset = pos + sep.length();
        }
        return chunks;
    }

    // --------------------------------------------------------------------------------------------

    /// checks if a string contains a given substring.
    inline bool contains(const std::string& s, const std::string& potential_substring)
    {
        const char* pos = strstr(s.c_str(), potential_substring.c_str());
        return pos != nullptr;
    }

    // --------------------------------------------------------------------------------------------

    /// checks if a string starts with a given prefix.
    inline bool starts_with(const std::string& s, const std::string& potential_start)
    {
        size_t n = potential_start.size();

        if (s.size() < n)
            return false;

        for (size_t i = 0; i < n; ++i)
            if (s[i] != potential_start[i])
                return false;

        return true;
    }

    // --------------------------------------------------------------------------------------------

    /// checks if a string ends with a given suffix.
    inline bool ends_with(const std::string& s, const std::string& potential_end)
    {
        size_t n = potential_end.size();
        size_t sn = s.size();

        if (sn < n)
            return false;

        for (size_t i = 0; i < n; ++i)
            if (s[sn - i - 1] != potential_end[n - i - 1])
                return false;

        return true;
    }

    // --------------------------------------------------------------------------------------------

    /// replaces substrings within a given string.
    inline std::string replace(
        const std::string& input,
        const std::string& old,
        const std::string& with)
    {
        if (input.empty()) return input;

        std::string result(input);
        size_t offset(0);
        while (true)
        {
            size_t pos = result.find(old, offset);
            if (pos == std::string::npos)
                break;

            result.replace(pos, old.length(), with);
            offset = pos + with.length();
        }
        return result;
    }

    // --------------------------------------------------------------------------------------------

    /// replaces characters within a given string.
    inline std::string replace(
        const std::string& input,
        char old,
        char with)
    {
        // added this function for consistency with replace substring
        std::string output(input);
        std::replace(output.begin(), output.end(), old, with);
        return output;
    }

    // --------------------------------------------------------------------------------------------

    /// create a formatted string.
    /// \param  format  printf-like format string
    /// \param  args    arguments to insert into the format string
    /// \return the formatted string
    template <typename... Args>
    inline std::string format(const char *format_string, Args ... args)
    {
        // get string size + 1 for null terminator to allocate a string of correct size
        int size = 1 + snprintf(nullptr, 0, format_string, std::forward<Args>(args)...);

        std::string s;
        s.resize(size);
        snprintf(&s[0], size, format_string, std::forward<Args>(args)...);
        return s.substr(0, size - 1);
    }

    /// create a formatted string (variadic base function).
    /// \return the unchanged \format_string
    inline std::string format(const char *format_string)
    {
        return format_string;
    }

    /// create a formatted string (variadic base function, needed for __VA_ARGS__ mappings).
    /// \return the empty string
    inline std::string format()
    {
        return "";
    }

    // --------------------------------------------------------------------------------------------

    /// removes leading and trailing quotes if there are some.
    /// returns true when it was a non-quoted string or valid quoted string before.
    /// returns false for single quotes and when a quote was only found at one end.
    inline bool remove_quotes(std::string& s)
    {
        size_t l = s.length();
        if (l == 0)
            return true;

        bool leading = s[0] == '\"';
        if (l == 1)
            return !leading; // one single quote

        bool trailing = s[l - 1] == '\"';
        if (leading != trailing) // quote one one side only
            return false;

        if (leading)
            s = s.substr(1, l - 2); // remove quotes on both sides
        return true;
    }

    // --------------------------------------------------------------------------------------------

    // Returns a string-representation of the given message severity
    inline std::string to_string(mi::base::Message_severity severity)
    {
        switch (severity)
        {
            case mi::base::MESSAGE_SEVERITY_FATAL:
                return "fatal";
            case mi::base::MESSAGE_SEVERITY_ERROR:
                return "error";
            case mi::base::MESSAGE_SEVERITY_WARNING:
                return "warning";
            case mi::base::MESSAGE_SEVERITY_INFO:
                return "info";
            case mi::base::MESSAGE_SEVERITY_VERBOSE:
                return "verbose";
            case mi::base::MESSAGE_SEVERITY_DEBUG:
                return "debug";
            default:
                break;
        }
        return "";
    }

    // --------------------------------------------------------------------------------------------

    // Returns a string-representation of the given message category
    inline std::string to_string(mi::neuraylib::IMessage::Kind message_kind)
    {
        switch (message_kind)
        {
            case mi::neuraylib::IMessage::MSG_INTEGRATION:
                return "MDL SDK";
            case mi::neuraylib::IMessage::MSG_IMP_EXP:
                return "Importer/Exporter";
            case mi::neuraylib::IMessage::MSG_COMILER_BACKEND:
                return "Compiler Backend";
            case mi::neuraylib::IMessage::MSG_COMILER_CORE:
                return "Compiler Core";
            case mi::neuraylib::IMessage::MSG_COMPILER_ARCHIVE_TOOL:
                return "Compiler Archive Tool";
            case mi::neuraylib::IMessage::MSG_COMPILER_DAG:
                return "Compiler DAG generator";
            default:
                break;
        }
        return "";
    }

    // --------------------------------------------------------------------------------------------

    // Get current date/time
    inline const std::string current_date_time_local()
    {
        std::time_t t = std::time(nullptr);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%F %T", std::localtime(&t));
        return buffer;
    }

    // Get current date/time
    inline const std::string current_date_time_UTC()
    {
        std::time_t t = std::time(nullptr);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%F %T", std::gmtime(&t));
        return buffer;
    }

    // --------------------------------------------------------------------------------------------

    /// Convert the given string into the given value.
    /// \return value resembling the given input string
    template <typename T>
    T lexicographic_cast(
        const std::string& str)
    {
        std::stringstream s;
        s << str;
        T result = T();
        s >> result;
        return result;
    }

}}}
#endif
