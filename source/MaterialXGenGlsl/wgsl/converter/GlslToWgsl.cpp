//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/wgsl/converter/GlslToWgsl.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <map>
#include <optional>
#include <sstream>
#include <string_view>

MATERIALX_NAMESPACE_BEGIN

namespace GlslToWgsl
{

namespace
{

using std::string_view;

bool isIdentChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

// True for a non-empty run of identifier characters only (no spaces, operators or punctuation).
bool isIdentifier(string_view s)
{
    if (s.empty())
        return false;
    for (char c : s)
    {
        if (!isIdentChar(c))
            return false;
    }
    return true;
}

bool isHorizontalSpace(char c) { return c == ' ' || c == '\t'; } 

bool startsWith(string_view s, string_view prefix)
{
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

// Strip carriage returns/newlines as well as spaces/tabs (see trim() below).
string_view trimView(string_view s)
{
    const size_t b = s.find_first_not_of(" \t\r\n");
    if (b == string_view::npos)
        return {};
    const size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

string trim(string_view s)
{
    const string_view t = trimView(s);
    return string(t.data(), t.size());
}

// Read the identifier starting at `start` (skipping leading whitespace).
// Returns the identifier and advances `start` to one past its last character.
string readIdentifier(const string& s, size_t& start)
{
    while (start < s.size() && isHorizontalSpace(s[start]))
        start++;
    size_t end = start;
    while (end < s.size() && isIdentChar(s[end]))
        end++;
    string ident = s.substr(start, end - start);
    start = end;
    return ident;
}

int braceDelta(string_view s)
{
    int d = 0;
    for (char c : s)
    {
        if (c == '{')
            d++;
        else if (c == '}')
            d--;
    }
    return d;
}

std::vector<string> readLines(const string& text)
{
    std::vector<string> lines;
    std::istringstream stream(text);
    string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(std::move(line));
    }
    return lines;
}

string joinLines(const std::vector<string>& lines)
{
    string result;
    result.reserve(lines.size() * 32);
    for (size_t i = 0; i < lines.size(); ++i)
    {
        result += lines[i];
        if (i + 1 < lines.size())
            result += '\n';
    }
    return result;
}

struct FnDef
{
    string name;
    string params;
};

// Parse a `fn NAME(PARAMS)` definition line. Returns nullopt if not a definition.
std::optional<FnDef> parseFnDef(string_view lineIn)
{
    const string line(lineIn);
    const size_t indentEnd = line.find_first_not_of(" \t");
    if (indentEnd == string::npos || !startsWith(string_view(line).substr(indentEnd), "fn "))
        return std::nullopt;
    const size_t nameStart = indentEnd + 3;
    const size_t paren = line.find('(', nameStart);
    if (paren == string::npos)
        return std::nullopt;
    int depth = 0;
    size_t close = string::npos;
    for (size_t i = paren; i < line.size(); ++i)
    {
        if (line[i] == '(')
            depth++;
        else if (line[i] == ')')
        {
            depth--;
            if (depth == 0)
            {
                close = i;
                break;
            }
        }
    }
    if (close == string::npos)
        return std::nullopt;
    FnDef def;
    def.name = trim(string_view(line).substr(nameStart, paren - nameStart));
    def.params = line.substr(paren + 1, close - paren - 1);
    if (def.name.empty() || def.name.find_first_of(" \t") != string::npos)
        return std::nullopt;
    return def;
}

// Split a parameter list (without the enclosing parentheses) on top-level
// commas, ignoring commas nested inside angle brackets or parens.
std::vector<string> splitTopLevel(const string& params)
{
    std::vector<string> out;
    int angle = 0, paren = 0;
    size_t start = 0;
    for (size_t i = 0; i < params.size(); ++i)
    {
        char c = params[i];
        if (c == '<')
            angle++;
        else if (c == '>')
            angle--;
        else if (c == '(')
            paren++;
        else if (c == ')')
            paren--;
        else if (c == ',' && angle == 0 && paren == 0)
        {
            out.push_back(trim(params.substr(start, i - start)));
            start = i + 1;
        }
    }
    string last = trim(params.substr(start));
    if (!last.empty())
        out.push_back(last);
    return out;
}

bool isKnownReturnType(string_view t)
{
    static constexpr string_view SCALARS[] = {
        "void", "float", "int", "uint", "bool",
    };
    if (std::find(std::begin(SCALARS), std::end(SCALARS), t) != std::end(SCALARS))
        return true;
    if (startsWith(t, "vec") || startsWith(t, "ivec") || startsWith(t, "uvec") || startsWith(t, "mat"))
        return true;
    static constexpr string_view STRUCTS[] = {
        "ClosureData", "FresnelData", "BSDF", "EDF", "VDF", "surfaceshader", "volumeshader",
        "displacementshader", "lightshader", "material",
    };
    return std::find(std::begin(STRUCTS), std::end(STRUCTS), t) != std::end(STRUCTS);
}

void countBraces(const string& line, int& depth, bool& seenOpen)
{
    for (char c : line)
    {
        if (c == '{')
        {
            depth++;
            seenOpen = true;
        }
        else if (c == '}')
        {
            depth--;
        }
    }
}

int netParens(const string& line)
{
    int net = 0;
    for (char c : line)
    {
        if (c == '(')
            net++;
        else if (c == ')')
            net--;
    }
    return net;
}

constexpr std::array<std::pair<string_view, string_view>, 19> TYPE_MAP = { {
    { "float", "f32" },
    { "int", "i32" },
    { "uint", "u32" },
    { "bool", "bool" },
    { "vec2", "vec2f" },
    { "vec3", "vec3f" },
    { "vec4", "vec4f" },
    { "ivec2", "vec2<i32>" },
    { "ivec3", "vec3<i32>" },
    { "ivec4", "vec4<i32>" },
    { "uvec2", "vec2<u32>" },
    { "uvec3", "vec3<u32>" },
    { "uvec4", "vec4<u32>" },
    { "bvec2", "vec2<bool>" },
    { "bvec3", "vec3<bool>" },
    { "bvec4", "vec4<bool>" },
    { "mat2", "mat2x2f" },
    { "mat3", "mat3x3f" },
    { "mat4", "mat4x4f" },
} };

string_view lookupWgslType(string_view glslType)
{
    for (const auto& entry : TYPE_MAP)
    {
        if (entry.first == glslType)
            return entry.second;
    }
    return glslType;
}

// A WGSL type that cannot be copied into a local `var` (handle/pointer types).
bool isHandleType(const string& wgslType)
{
    return wgslType == "sampler" ||
           wgslType.compare(0, 7, "texture") == 0 ||
           wgslType.compare(0, 3, "ptr") == 0;
}

// Replace GLSL-style scalar casts with WGSL spellings (float() -> f32(), int() -> i32(), …).
string rewriteScalarCast(string line, string_view glslType, string_view wgslType)
{
    size_t pos = 0;
    const string token = string(glslType) + "(";
    const string repl = string(wgslType) + "(";
    while ((pos = line.find(token, pos)) != string::npos)
    {
        if (pos > 0 && isIdentChar(line[pos - 1]))
        {
            pos += token.size();
            continue;
        }
        line.replace(pos, token.size(), repl);
        pos += repl.size();
    }
    return line;
}

string rewriteScalarCasts(string line)
{
    static constexpr std::array<std::pair<string_view, string_view>, 3> SCALAR_CASTS = { {
        { "float", "f32" },
        { "int", "i32" },
        { "uint", "u32" },
    } };
    for (const auto& cast : SCALAR_CASTS)
        line = rewriteScalarCast(std::move(line), cast.first, cast.second);
    return line;
}

// Rewrite "const type name = ..." to "const name: type = ...".
string rewriteConst(const string& line)
{
    string result = line;
    size_t pos = 0;
    while ((pos = result.find("const ", pos)) != string::npos)
    {
        size_t constStart = pos;
        size_t constEnd = pos + 6;

        size_t nameStart = constEnd;
        while (nameStart < result.size() && isHorizontalSpace(result[nameStart]))
            nameStart++;
        if (nameStart >= result.size())
        {
            pos = constEnd;
            continue;
        }

        // Only handle the GLSL form "const type name =" (no colon before '=').
        size_t eqPos = result.find('=', nameStart);
        size_t colonPos = result.find(':', nameStart);
        if (eqPos == string::npos)
        {
            pos = constEnd;
            continue;
        }
        if (colonPos != string::npos && colonPos < eqPos)
        {
            // Already in WGSL "name: type =" form (or unrelated colon) -- leave it.
            pos = constEnd;
            continue;
        }

        size_t typeStart = nameStart;
        size_t typeEnd = typeStart;
        if (result[typeEnd] == 'v' || result[typeEnd] == 'm')
        {
            size_t angle = result.find('>', typeEnd);
            if (angle != string::npos && angle < eqPos)
                typeEnd = angle + 1;
            else
                typeEnd = result.find_first_of(" \t", typeStart);
        }
        else
            typeEnd = result.find_first_of(" \t", typeStart);
        if (typeEnd == string::npos || typeEnd > eqPos)
        {
            pos = constEnd;
            continue;
        }

        string typeStr = mapType(result.substr(typeStart, typeEnd - typeStart));
        while (typeEnd < result.size() && isHorizontalSpace(result[typeEnd]))
            typeEnd++;
        size_t nameStart2 = typeEnd;
        size_t nameEnd2 = result.find_first_of(" \t=;", nameStart2);
        if (nameEnd2 == string::npos || nameEnd2 > eqPos)
        {
            pos = constEnd;
            continue;
        }
        string nameStr = result.substr(nameStart2, nameEnd2 - nameStart2);
        string wgsl = "const " + nameStr + ": " + typeStr + " =";
        result.replace(constStart, eqPos - constStart + 1, wgsl);
        pos = constStart + wgsl.size();
    }
    return result;
}

bool replaceOneTernary(string& line)
{
    size_t q = line.find('?');
    if (q == string::npos)
        return false;

    size_t qBefore = q;
    while (qBefore > 0 && isHorizontalSpace(line[qBefore - 1]))
        qBefore--;

    size_t qAfter = q + 1;
    while (qAfter < line.size() && isHorizontalSpace(line[qAfter]))
        qAfter++;

    // Walk back to the start of the condition expression. Stop only at expression
    // boundaries — an enclosing `(`, a top-level `,`/`;`/`{`/`}`, or an assignment
    // `=` — NOT at spaces or comparison/logical operators, so multi-token conditions
    // like `cosTheta < cosB` are captured whole.
    size_t condStart = qBefore;
    int paren = 0;
    while (condStart > 0)
    {
        char ch = line[condStart - 1];
        if (ch == ')')
            paren++;
        else if (ch == '(')
        {
            paren--;
            if (paren < 0)
                break;
        }
        else if (paren == 0)
        {
            if (ch == ',' || ch == ';' || ch == '{' || ch == '}')
                break;
            if (ch == '=')
            {
                const char l = (condStart >= 2) ? line[condStart - 2] : '\0';
                const char r = (condStart < line.size()) ? line[condStart] : '\0';
                const bool opPart = (l == '<' || l == '>' || l == '!' || l == '=') || (r == '=');
                if (!opPart)
                    break; // a true assignment, not ==/<=/>=/!=
            }
        }
        condStart--;
    }
    while (condStart < qBefore && isHorizontalSpace(line[condStart]))
        condStart++;
    // A `return <cond> ? ...` leaves the keyword outside the condition.
    if (line.compare(condStart, 7, "return ") == 0)
        condStart += 7;
    while (condStart < qBefore && isHorizontalSpace(line[condStart]))
        condStart++;

    string cond = line.substr(condStart, qBefore - condStart);

    size_t colonPos = string::npos;
    paren = 0;
    for (size_t c = qAfter; c < line.size(); ++c)
    {
        char ch = line[c];
        if (ch == '(')
            paren++;
        else if (ch == ')')
            paren--;
        else if (ch == ':' && paren == 0)
        {
            colonPos = c;
            break;
        }
    }
    if (colonPos == string::npos)
        return false;

    size_t tStart = qAfter;
    while (tStart < colonPos && isHorizontalSpace(line[tStart]))
        tStart++;
    size_t tEnd = colonPos;
    while (tEnd > tStart && isHorizontalSpace(line[tEnd - 1]))
        tEnd--;
    string tVal = trim(line.substr(tStart, tEnd - tStart));

    size_t fStart = colonPos + 1;
    while (fStart < line.size() && isHorizontalSpace(line[fStart]))
        fStart++;

    size_t fEnd = fStart;
    paren = 0;
    for (size_t i = fStart; i < line.size(); ++i)
    {
        char ch = line[i];
        if (ch == '(')
            paren++;
        else if (ch == ')')
        {
            paren--;
            if (paren < 0)
            {
                fEnd = i;
                break;
            }
        }
        else if (paren == 0 && (ch == ';' || ch == ','))
        {
            fEnd = i;
            break;
        }
        fEnd = i + 1;
    }
    string fVal = trim(line.substr(fStart, fEnd - fStart));

    if (cond.empty() || tVal.empty() || fVal.empty())
        return false;

    string repl = "select(" + fVal + ", " + tVal + ", " + cond + ")";
    line.replace(condStart, fEnd - condStart, repl);
    return true;
}

string rewriteTernaries(string line)
{
    const size_t MAX_ITERATIONS = 64;
    for (size_t iter = 0; iter < MAX_ITERATIONS; ++iter)
    {
        string prev = line;
        if (!replaceOneTernary(line))
            break;
        if (line == prev)
            break;
    }
    return line;
}

// Rewrite "type name = value" to "var name: wgslType = value" for known GLSL types.
string rewriteVariableDecl(string line)
{
    size_t lineStart = 0;
    while (lineStart < line.size() && isHorizontalSpace(line[lineStart]))
        lineStart++;
    for (const auto& entry : TYPE_MAP)
    {
        const string_view glslType = entry.first;
        const string_view wgslType = entry.second;
        if (line.size() > lineStart + glslType.size() + 1 &&
            line.compare(lineStart, glslType.size(), glslType.data(), glslType.size()) == 0 &&
            isHorizontalSpace(line[lineStart + glslType.size()]))
        {
            size_t nameStart = lineStart + glslType.size();
            while (nameStart < line.size() && isHorizontalSpace(line[nameStart]))
                nameStart++;
            if (nameStart >= line.size())
                break;
            size_t nameEnd = nameStart;
            while (nameEnd < line.size() && isIdentChar(line[nameEnd]))
                nameEnd++;
            if (nameEnd <= nameStart)
                break;
            size_t afterName = nameEnd;
            while (afterName < line.size() && isHorizontalSpace(line[afterName]))
                afterName++;
            const string varName = line.substr(nameStart, nameEnd - nameStart);
            const string leading = line.substr(0, lineStart);
            const string wgsl = string(wgslType.data(), wgslType.size());
            if (afterName < line.size() && line[afterName] == '=')
            {
                size_t rs = afterName + 1;
                while (rs < line.size() && isHorizontalSpace(line[rs]))
                    rs++;
                return leading + "var " + varName + ": " + wgsl + " = " + line.substr(rs);
            }
            if (afterName < line.size() && line[afterName] == ';')
            {
                return leading + "var " + varName + ": " + wgsl + ";";
            }
            break;
        }
    }

    // Also handle declarations already using WGSL type names but GLSL order
    // (e.g. emitted source like `vec3f V = ...` -> `var V: vec3f = ...`).
    static constexpr string_view WGSL_TYPES[] = {
        "vec2f", "vec3f", "vec4f", "mat2x2f", "mat3x3f", "mat4x4f", "f32", "i32", "u32"
    };
    for (const string_view wtype : WGSL_TYPES)
    {
        if (line.size() > lineStart + wtype.size() + 1 &&
            line.compare(lineStart, wtype.size(), wtype.data(), wtype.size()) == 0 &&
            isHorizontalSpace(line[lineStart + wtype.size()]))
        {
            size_t nameStart = lineStart + wtype.size();
            while (nameStart < line.size() && isHorizontalSpace(line[nameStart]))
                nameStart++;
            size_t nameEnd = nameStart;
            while (nameEnd < line.size() && isIdentChar(line[nameEnd]))
                nameEnd++;
            if (nameEnd <= nameStart)
                break;
            size_t afterName = nameEnd;
            while (afterName < line.size() && isHorizontalSpace(line[afterName]))
                afterName++;
            const string varName = line.substr(nameStart, nameEnd - nameStart);
            const string wgsl = string(wtype.data(), wtype.size());
            if (afterName < line.size() && line[afterName] == '=')
            {
                size_t rs = afterName + 1;
                while (rs < line.size() && isHorizontalSpace(line[rs]))
                    rs++;
                return line.substr(0, lineStart) + "var " + varName + ": " + wgsl + " = " + line.substr(rs);
            }
            if (afterName < line.size() && line[afterName] == ';')
            {
                return line.substr(0, lineStart) + "var " + varName + ": " + wgsl + ";";
            }
            break;
        }
    }
    return line;
}

const char* KNOWN_STRUCT_TYPES[] = {
    "ClosureData", "FresnelData", "lightshader", "material", "BSDF", "EDF", "VDF",
    "surfaceshader", "volumeshader", "displacementshader"
};

// Rewrite "StructType name [= value];" to "var name: StructType [= value];" for
// known MaterialX struct types.
string rewriteStructVariableDecl(string line)
{
    size_t lineStart = 0;
    while (lineStart < line.size() && isHorizontalSpace(line[lineStart]))
        lineStart++;
    if (lineStart >= line.size())
        return line;

    size_t typeEnd = lineStart;
    while (typeEnd < line.size() && isIdentChar(line[typeEnd]))
        typeEnd++;
    if (typeEnd <= lineStart || typeEnd >= line.size())
        return line;
    if (!isHorizontalSpace(line[typeEnd]))
        return line;

    string typeName = line.substr(lineStart, typeEnd - lineStart);

    bool isKnownStruct = false;
    for (const char* t : KNOWN_STRUCT_TYPES)
    {
        if (typeName == t)
        {
            isKnownStruct = true;
            break;
        }
    }
    if (!isKnownStruct)
        return line;

    size_t nameStart = typeEnd;
    while (nameStart < line.size() && isHorizontalSpace(line[nameStart]))
        nameStart++;
    if (nameStart >= line.size())
        return line;

    size_t nameEnd = nameStart;
    while (nameEnd < line.size() && isIdentChar(line[nameEnd]))
        nameEnd++;
    if (nameEnd <= nameStart)
        return line;

    string varName = line.substr(nameStart, nameEnd - nameStart);
    size_t afterName = nameEnd;
    while (afterName < line.size() && isHorizontalSpace(line[afterName]))
        afterName++;

    string leading = line.substr(0, lineStart);

    if (afterName < line.size() && line[afterName] == '=')
    {
        size_t rs = afterName + 1;
        while (rs < line.size() && isHorizontalSpace(line[rs]))
            rs++;
        string rest = line.substr(rs);
        return leading + "var " + varName + ": " + typeName + " = " + rest;
    }
    if (afterName >= line.size() || line[afterName] == ';' ||
        line[afterName] == '\n' || line[afterName] == '\r')
    {
        return leading + "var " + varName + ": " + typeName + ";";
    }
    return line;
}

// Rewrite GLSL component-wise comparison built-ins to WGSL operators:
//   greaterThan(a, b) -> (a > b), lessThanEqual(a, b) -> (a <= b), etc.
string rewriteVectorCompare(string line)
{
    struct Cmp
    {
        const char* fn;
        const char* op;
    };
    // Longer names first so e.g. greaterThanEqual is matched before greaterThan.
    static const Cmp CMPS[] = {
        { "greaterThanEqual(", " >= " },
        { "lessThanEqual(", " <= " },
        { "greaterThan(", " > " },
        { "lessThan(", " < " },
        { "notEqual(", " != " },
        { "equal(", " == " },
    };
    for (const Cmp& c : CMPS)
    {
        const string tok = c.fn;
        size_t pos = 0;
        while ((pos = line.find(tok, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(line[pos - 1]))
            {
                pos += tok.size();
                continue;
            }
            const size_t argStart = pos + tok.size();
            int depth = 1;
            size_t i = argStart;
            std::vector<size_t> commas;
            for (; i < line.size() && depth > 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                {
                    depth--;
                    if (depth == 0)
                        break;
                }
                else if (line[i] == ',' && depth == 1)
                    commas.push_back(i);
            }
            if (depth != 0 || commas.size() != 1)
            {
                pos += tok.size();
                continue;
            }
            const string a = trim(line.substr(argStart, commas[0] - argStart));
            const string b = trim(line.substr(commas[0] + 1, i - 1 - commas[0] - 1));
            const string repl = "(" + a + c.op + b + ")";
            line.replace(pos, i - pos, repl);
            pos += repl.size();
        }
    }
    return line;
}

// Rewrite GLSL matrix constructors to WGSL: mat3(...) -> mat3x3f(...). (vec3(...)
// is left as-is; WGSL infers its component type.)
string rewriteMatrixCtors(string line)
{
    static constexpr std::array<std::pair<string_view, string_view>, 3> MATRIX_CTORS = { {
        { "mat2(", "mat2x2f(" },
        { "mat3(", "mat3x3f(" },
        { "mat4(", "mat4x4f(" },
    } };
    for (const auto& m : MATRIX_CTORS)
    {
        const string from(m.first);
        const string to(m.second);
        size_t pos = 0;
        while ((pos = line.find(from, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(line[pos - 1]))
            {
                pos += from.size();
                continue;
            }
            line.replace(pos, from.size(), to);
            pos += to.size();
        }
    }
    return line;
}

// Rewrite genglsl math built-in alias calls (`mx_cos(` etc.) to WGSL built-ins.
string rewriteMathBuiltins(string line)
{
    // mx_atan maps to WGSL atan (1 arg) or atan2 (2 args).
    {
        const string tok = "mx_atan(";
        size_t pos = 0;
        while ((pos = line.find(tok, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(line[pos - 1]))
            {
                pos += tok.size();
                continue;
            }
            int depth = 1;
            size_t i = pos + tok.size();
            int commas = 0;
            for (; i < line.size() && depth > 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                    depth--;
                else if (line[i] == ',' && depth == 1)
                    commas++;
            }
            const string repl = (commas == 1) ? "atan2(" : "atan(";
            line.replace(pos, tok.size(), repl);
            pos += repl.size();
        }
    }

    static constexpr std::array<std::pair<string_view, string_view>, 7> ALIASES = { {
        { "mx_inversesqrt(", "inverseSqrt(" },
        { "mx_radians(", "radians(" },
        { "mx_asin(", "asin(" },
        { "mx_acos(", "acos(" },
        { "mx_sin(", "sin(" },
        { "mx_cos(", "cos(" },
        { "mx_tan(", "tan(" },
    } };
    for (const auto& alias : ALIASES)
    {
        const string from(alias.first);
        const string to(alias.second);
        size_t pos = 0;
        while ((pos = line.find(from, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(line[pos - 1]))
            {
                pos += from.size();
                continue;
            }
            line.replace(pos, from.size(), to);
            pos += to.size();
        }
    }
    return line;
}

// Add a `&` to the (out) result argument of the light-bridge call
// `sampleLightSource(light, position, result)` -> `(light, position, &result)`.
string rewriteSampleLightSource(string line)
{
    const string tok = "sampleLightSource(";
    const size_t pos = line.find(tok);
    if (pos == string::npos || (pos > 0 && isIdentChar(line[pos - 1])))
        return line;
    // HwLightSamplerNode emits the GLSL definition `void sampleLightSource(...)`; only
    // rewrite call sites (third argument needs `&`), not the definition signature.
    {
        const string head = trim(line.substr(0, pos));
        if (head == "void" || head.rfind("void ", 0) == 0)
            return line;
    }
    const size_t argStart = pos + tok.size();
    int depth = 1;
    std::vector<size_t> commas;
    size_t i = argStart;
    for (; i < line.size() && depth > 0; ++i)
    {
        const char c = line[i];
        if (c == '(')
            depth++;
        else if (c == ')')
        {
            depth--;
            if (depth == 0)
                break;
        }
        else if (c == ',' && depth == 1)
            commas.push_back(i);
    }
    if (depth != 0 || commas.size() != 2)
        return line;
    size_t a3 = commas[1] + 1;
    while (a3 < line.size() && isHorizontalSpace(line[a3]))
        a3++;
    if (a3 < line.size() && line[a3] != '&')
        line.insert(a3, "&");
    return line;
}

// Rewrite a GLSL preprocessor `#define` to a WGSL `const` or type `alias`.
//   #define M_PI 3.14159        -> const M_PI: f32 = 3.14159;
//   #define CLOSURE_TYPE_X 1    -> const CLOSURE_TYPE_X: i32 = 1;
//   #define EDF vec3            -> alias EDF = vec3f;
//   #define material surfaceshader -> alias material = surfaceshader;
string rewriteDefine(const string& line)
{
    const size_t p = line.find("#define");
    string rest = trim(line.substr(p + 7));
    const size_t sp = rest.find_first_of(" \t");
    if (sp == string::npos)
        return ""; // bare `#define NAME` guard — drop (WGSL has no preprocessor)
    const string name = rest.substr(0, sp);
    const string value = trim(rest.substr(sp));
    if (value.empty())
        return "";

    // Drop genglsl math built-in aliases (`#define mx_cos cos`, `#define mx_float_bits_to_int
    // floatBitsToInt`, ...) — a single-identifier value. Their call sites are rewritten to WGSL
    // built-ins by rewriteMathBuiltins.
    if (startsWith(name, "mx_") && isIdentifier(value))
        return "";

    // A type-valued define becomes a WGSL type `alias`. mapType() converts a GLSL type spelling
    // (`vec3` -> `vec3f`) and returns the value unchanged for the MaterialX closure structs, which
    // are already valid WGSL type names -- so the alias target is just the mapped value.
    const string mapped = mapType(value);
    const bool isClosureType = value == "surfaceshader" || value == "material" ||
                               value == "BSDF" || value == "EDF" || value == "VDF";
    if (isIdentifier(value) && (mapped != value || isClosureType))
        return "alias " + name + " = " + mapped + ";";

    // Otherwise it is a numeric or expression define -> `const` (integer literal => i32, else f32).
    const bool isInt = value.find_first_not_of("0123456789+-") == string::npos;
    return "const " + name + ": " + (isInt ? "i32" : "f32") + " = " + value + ";";
}

// Rewrite the declaration in a `for (<type> i = ...; ...)` init clause to WGSL.
//   for (int i = 0; i < n; i++)  -> for (var i: i32 = 0; i < n; i += 1)
string rewriteForLoopInit(string line)
{
    size_t pos = 0;
    while ((pos = line.find("for", pos)) != string::npos)
    {
        // Whole-word `for` followed by `(`.
        const bool wordBefore = (pos > 0 && isIdentChar(line[pos - 1]));
        size_t paren = pos + 3;
        while (paren < line.size() && isHorizontalSpace(line[paren]))
            paren++;
        if (wordBefore || paren >= line.size() || line[paren] != '(')
        {
            pos += 3;
            continue;
        }
        const size_t initStart = paren + 1;
        const size_t semi = line.find(';', initStart);
        if (semi == string::npos)
        {
            pos = paren + 1;
            continue;
        }
        // The init clause is rewritten with the standard value-declaration pass.
        const string init = line.substr(initStart, semi - initStart);
        const string trimmedInit = trim(init);
        const string rewritten = rewriteVariableDecl(trimmedInit);
        if (rewritten != trimmedInit)
        {
            line.replace(initStart, semi - initStart, rewritten);
            pos = line.find(';', initStart) + 1;
            continue;
        }
        pos = semi + 1;
    }
    return line;
}

// Rename GLSL identifiers that collide with WGSL keywords (e.g. a variable named
// `var` in the thin-film code). Runs before declarations introduce the `var`
// keyword, so every such token in the GLSL source is an identifier.
string rewriteReservedIdents(string line)
{
    static const char* RESERVED[] = { "var", "let", "loop" };
    for (const char* r : RESERVED)
    {
        const string word = r;
        size_t pos = 0;
        while ((pos = line.find(word, pos)) != string::npos)
        {
            const bool before = (pos > 0) && isIdentChar(line[pos - 1]);
            const size_t after = pos + word.size();
            const bool afterIdent = (after < line.size()) && isIdentChar(line[after]);
            // WGSL resource declarations use `var<uniform>` / `var<storage>`; do not rename.
            const bool wgslStorage = (word == "var" && after < line.size() && line[after] == '<');
            if (!before && !afterIdent && !wgslStorage)
            {
                line.insert(after, "_");
                pos = after + 1;
            }
            else
            {
                pos = after;
            }
        }
    }
    return line;
}

// Split a line on top-level `;` (ignoring nested parens/brackets/braces).
std::vector<string> splitTopLevelSemicolons(const string& line)
{
    std::vector<string> out;
    int angle = 0, paren = 0, brace = 0;
    size_t start = 0;
    for (size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (c == '<')
            angle++;
        else if (c == '>')
            angle--;
        else if (c == '(')
            paren++;
        else if (c == ')')
            paren--;
        else if (c == '{')
            brace++;
        else if (c == '}')
            brace--;
        else if (c == ';' && angle == 0 && paren == 0 && brace == 0)
        {
            out.push_back(line.substr(start, i - start));
            start = i + 1;
        }
    }
    out.push_back(line.substr(start));
    return out;
}

bool looksLikeTypedDecl(const string& segIn)
{
    const string seg = trim(segIn);
    if (seg.empty())
        return false;
    size_t at = 0;
    while (at < seg.size() && isHorizontalSpace(seg[at]))
        at++;
    for (const auto& entry : TYPE_MAP)
    {
        const string_view glslType = entry.first;
        if (seg.size() >= at + glslType.size() &&
            seg.compare(at, glslType.size(), glslType.data(), glslType.size()) == 0 &&
            (at + glslType.size() >= seg.size() || isHorizontalSpace(seg[at + glslType.size()])))
        {
            size_t nameStart = at + glslType.size();
            while (nameStart < seg.size() && isHorizontalSpace(seg[nameStart]))
                nameStart++;
            size_t nameEnd = nameStart;
            while (nameEnd < seg.size() && isIdentChar(seg[nameEnd]))
                nameEnd++;
            if (nameEnd <= nameStart)
                return false;
            size_t after = nameEnd;
            while (after < seg.size() && isHorizontalSpace(seg[after]))
                after++;
            return after >= seg.size() || seg[after] == '=';
        }
    }
    return false;
}

// Expand chained GLSL declarations on one line, e.g.
// `float h = hsv.x; float s = hsv.y; float v = hsv.z;` -> separate WGSL `var` decls.
string rewriteChainedDecls(string line)
{
    std::vector<string> parts;
    for (const string& p : splitTopLevelSemicolons(line))
    {
        if (!trim(p).empty())
            parts.push_back(p);
    }
    if (parts.size() < 2)
        return line;
    for (const string& p : parts)
    {
        if (!looksLikeTypedDecl(p))
            return line;
    }
    const size_t ls = line.find_first_not_of(" \t");
    const string indent = (ls == string::npos) ? "" : line.substr(0, ls);
    string out;
    for (const string& p : parts)
    {
        const string seg = trim(p);
        if (seg.empty())
            continue;
        const string decl = rewriteVariableDecl(seg + ";");
        out += (out.empty() ? "" : " ") + trim(decl);
    }
    return indent + out;
}

size_t findMatchingCloseParen(const string& s, size_t open)
{
    int depth = 0;
    for (size_t i = open; i < s.size(); ++i)
    {
        if (s[i] == '(')
            depth++;
        else if (s[i] == ')')
        {
            depth--;
            if (depth == 0)
                return i;
        }
    }
    return string::npos;
}

// Wrap a single-statement control body on the same line in `{ }`.
// GLSL allows `else s = 0.0f;` and `else if (a) return x;`; WGSL requires blocks.
string rewriteBracelessControlSameLine(string line)
{
    const size_t ls = line.find_first_not_of(" \t");
    const string indent = (ls == string::npos) ? "" : line.substr(0, ls);
    const string t = trim(line);
    if (t.empty() || t.find('{') != string::npos)
        return line;

    size_t headerEnd = string::npos;

    if ((t.compare(0, 8, "else if ") == 0) || (t.size() > 7 && t.compare(0, 7, "else if") == 0 && t[7] == '('))
    {
        const size_t open = t.find('(');
        if (open != string::npos)
        {
            const size_t close = findMatchingCloseParen(t, open);
            if (close != string::npos)
                headerEnd = close + 1;
        }
    }
    else if (t.compare(0, 4, "else") == 0 && (t.size() == 4 || isHorizontalSpace(t[4])))
    {
        size_t after = 4;
        while (after < t.size() && isHorizontalSpace(t[after]))
            after++;
        if (after + 2 <= t.size() && t.compare(after, 2, "if") == 0 &&
            (after + 2 >= t.size() || !isIdentChar(t[after + 2])))
            return line;
        // `else // comment` with a braced body on the following line — not a same-line stmt.
        if (after >= t.size() || t.compare(after, 2, "//") == 0 || t.compare(after, 2, "/*") == 0)
            return line;
        headerEnd = after;
    }
    else
    {
        static constexpr string_view CONTROL_PREFIXES[] = { "if", "while", "for" };
        for (const string_view pfx : CONTROL_PREFIXES)
        {
            if (startsWith(t, pfx) && (t.size() == pfx.size() || isHorizontalSpace(t[pfx.size()]) || t[pfx.size()] == '('))
            {
                const size_t open = t.find('(');
                if (open != string::npos)
                {
                    const size_t close = findMatchingCloseParen(t, open);
                    if (close != string::npos)
                        headerEnd = close + 1;
                }
                break;
            }
        }
    }

    if (headerEnd == string::npos || headerEnd >= t.size())
        return line;
    size_t bodyStart = headerEnd;
    while (bodyStart < t.size() && isHorizontalSpace(t[bodyStart]))
        bodyStart++;
    if (bodyStart >= t.size() || t[bodyStart] == '{')
        return line;
    if (t.compare(bodyStart, 2, "//") == 0 || t.compare(bodyStart, 2, "/*") == 0)
        return line;

    const string header = trim(t.substr(0, headerEnd));
    const string body = trim(t.substr(bodyStart));
    return indent + header + " { " + body + " }";
}

// Strip the GLSL `f` suffix from float literals (`0.0f` -> `0.0`).
string rewriteFloatLiteralSuffix(string line)
{
    for (size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (c != 'f' && c != 'F')
            continue;
        if (i == 0)
            continue;
        const char prev = line[i - 1];
        if (!std::isdigit(static_cast<unsigned char>(prev)) && prev != '.')
            continue;
        if (i + 1 < line.size() && isIdentChar(line[i + 1]))
            continue;
        // WGSL type names such as vec3f and mat4x4f end in `f` after a digit; do not treat those
        // as GLSL float-literal suffixes (otherwise native WGSL struct/uniform declarations break).
        size_t j = i - 1;
        while (j > 0 && (std::isdigit(static_cast<unsigned char>(line[j])) || line[j] == '.'))
            --j;
        if (j < i - 1 && isIdentChar(line[j]))
            continue;
        line.erase(i, 1);
        if (i > 0)
            --i;
    }
    return line;
}

// Expand a GLSL multiple declaration `vec3 a, b;` into separate WGSL declarations
// `var a: vec3f; var b: vec3f;`. Only the no-initializer form is handled.
string rewriteMultiDecl(string line)
{
    size_t ls = 0;
    while (ls < line.size() && isHorizontalSpace(line[ls]))
        ls++;
    size_t typeEnd = ls;
    while (typeEnd < line.size() && (isalnum(static_cast<unsigned char>(line[typeEnd])) || line[typeEnd] == '_'))
        typeEnd++;
    if (typeEnd == ls || typeEnd >= line.size() || !isHorizontalSpace(line[typeEnd]))
        return line;
    const string glslType = line.substr(ls, typeEnd - ls);
    const string wtype = mapType(glslType);
    if (wtype == glslType && glslType.compare(0, 3, "vec") != 0 && glslType.compare(0, 3, "mat") != 0 &&
        glslType != "float" && glslType != "int" && glslType != "bool")
        return line; // not a known type

    const size_t semi = line.find(';', typeEnd);
    if (semi == string::npos)
        return line;
    const string body = line.substr(typeEnd, semi - typeEnd);
    if (body.find(',') == string::npos || body.find('=') != string::npos || body.find('(') != string::npos)
        return line; // not a comma-separated declaration list

    std::vector<string> names;
    for (const string& n : splitTopLevel(body))
    {
        const string nm = trim(n);
        if (!isIdentifier(nm))
            return line; // not a plain identifier list
        names.push_back(nm);
    }
    const string indent = line.substr(0, ls);
    string out;
    for (const string& nm : names)
        out += (out.empty() ? "" : " ") + ("var " + nm + ": " + wtype + ";");
    return indent + out;
}

// Replace ++/-- (prefix and postfix) with += 1 / -= 1.
string rewriteIncDec(string line)
{
    // Prefix: ++var -> var += 1, --var -> var -= 1
    const char* OPS[] = { "++", "--" };
    const char* DELTAS[] = { " += 1", " -= 1" };
    for (size_t k = 0; k < 2; ++k)
    {
        for (size_t pos = 0; (pos = line.find(OPS[k], pos)) != string::npos;)
        {
            size_t nameStart = pos + 2;
            while (nameStart < line.size() && isHorizontalSpace(line[nameStart]))
                nameStart++;
            size_t nameEnd = nameStart;
            while (nameEnd < line.size() && isIdentChar(line[nameEnd]))
                nameEnd++;
            if (nameEnd > nameStart)
            {
                string varName = line.substr(nameStart, nameEnd - nameStart);
                line.replace(pos, nameEnd - pos, varName + DELTAS[k]);
                pos = nameStart + varName.size();
                continue;
            }
            pos += 2;
        }
    }
    // Postfix: var++ -> var += 1, var-- -> var -= 1
    for (size_t k = 0; k < 2; ++k)
    {
        size_t pos = 0;
        while ((pos = line.find(OPS[k], pos)) != string::npos)
        {
            size_t nameEnd = pos;
            while (nameEnd > 0 && isHorizontalSpace(line[nameEnd - 1]))
                nameEnd--;
            size_t nameStart = nameEnd;
            while (nameStart > 0 && isIdentChar(line[nameStart - 1]))
                nameStart--;
            if (nameStart < nameEnd)
            {
                string varName = line.substr(nameStart, nameEnd - nameStart);
                line.replace(nameStart, (pos + 2) - nameStart, varName + DELTAS[k]);
                pos = nameStart + varName.size();
                continue;
            }
            pos += 2;
        }
    }
    return line;
}

// A value parameter renamed to make it mutable in WGSL: the signature takes
// `<name><ARG_SUFFIX>` and the body gets an injected `var <name> = <name><ARG_SUFFIX>;`.
const char* const ARG_SUFFIX = "_arg";

// Rewrite a single function-definition parameter to WGSL. Records out-parameter
// names (for body dereferencing) and copyable value parameters (for the mutable
// `var` copy injected at body start). Combined samplers are expanded into the
// split texture/sampler pair.
string rewriteParam(string p, std::vector<string>& outNames,
                    std::vector<std::pair<string, string>>& valueParams)
{
    p = trim(p);
    if (p.empty())
        return p;

    bool isOut = false;
    if (p.compare(0, 4, "out ") == 0)
    {
        isOut = true;
        p = trim(p.substr(4));
    }
    else if (p.compare(0, 6, "inout ") == 0)
    {
        isOut = true;
        p = trim(p.substr(6));
    }
    else if (p.compare(0, 3, "in ") == 0)
    {
        p = trim(p.substr(3));
    }

    // Combined sampler: `sampler2D name` -> split texture + sampler (handle types).
    if (p.compare(0, 10, "sampler2D ") == 0)
    {
        size_t s = 10;
        const string name = readIdentifier(p, s);
        return name + "_texture: texture_2d<f32>, " + name + "_sampler: sampler";
    }

    string name, type;
    size_t colon = p.find(':');
    if (colon != string::npos)
    {
        // Already `name: type` (e.g. from token substitution); normalize the type.
        name = trim(p.substr(0, colon));
        type = mapType(trim(p.substr(colon + 1)));
    }
    else
    {
        // GLSL `type name`.
        size_t sp = p.find_first_of(" \t");
        if (sp == string::npos)
            return p; // malformed; leave as-is
        type = mapType(trim(p.substr(0, sp)));
        name = trim(p.substr(sp + 1));
    }

    if (isOut)
    {
        outNames.push_back(name);
        return name + ": ptr<function, " + type + ">";
    }

    // Value parameters are renamed and copied into a mutable local so bodies that
    // assign to them compile under WGSL (parameters are immutable). Handle types
    // (textures/samplers/pointers) cannot be copied and are passed through.
    if (!isHandleType(type))
    {
        valueParams.emplace_back(name, type);
        return name + ARG_SUFFIX + ": " + type;
    }
    return name + ": " + type;
}

struct RewrittenSignature
{
    string sig;
    std::vector<string> outNames;
    std::vector<std::pair<string, string>> valueParams;
};

// Detect and rewrite a GLSL function-definition signature line.
std::optional<RewrittenSignature> rewriteSignature(const string& line)
{
    const size_t indentEnd = line.find_first_not_of(" \t");
    if (indentEnd == string::npos)
        return std::nullopt;
    const string indent = line.substr(0, indentEnd);

    const size_t paren = line.find('(');
    if (paren == string::npos)
        return std::nullopt;

    // Head must be exactly "<returnType> <funcName>" with no assignment.
    const string head = trim(string_view(line).substr(indentEnd, paren - indentEnd));
    if (head.find('=') != string::npos)
        return std::nullopt;
    const size_t headSp = head.find_first_of(" \t");
    if (headSp == string::npos)
        return std::nullopt;
    const string retType = trim(head.substr(0, headSp));
    const string funcName = trim(head.substr(headSp + 1));
    if (funcName.empty() || funcName.find_first_of(" \t") != string::npos)
        return std::nullopt;
    if (!isKnownReturnType(retType))
        return std::nullopt;

    // Find the matching close paren for the parameter list.
    int depth = 0;
    size_t close = string::npos;
    for (size_t i = paren; i < line.size(); ++i)
    {
        if (line[i] == '(')
            depth++;
        else if (line[i] == ')')
        {
            depth--;
            if (depth == 0)
            {
                close = i;
                break;
            }
        }
    }
    if (close == string::npos)
        return std::nullopt;

    // After the parameter list only whitespace and an optional `{` may follow.
    const string tail = trim(string_view(line).substr(close + 1));
    if (!tail.empty() && tail[0] != '{')
        return std::nullopt;

    const string paramList = line.substr(paren + 1, close - paren - 1);
    const std::vector<string> params = splitTopLevel(paramList);

    RewrittenSignature rewritten;
    string wgslParams;
    for (const string& p : params)
    {
        const string wp = rewriteParam(p, rewritten.outNames, rewritten.valueParams);
        if (wp.empty())
            continue;
        if (!wgslParams.empty())
            wgslParams += ", ";
        wgslParams += wp;
    }

    string sig = indent + "fn " + funcName + "(" + wgslParams + ")";
    // Keep the WGSL return type for any non-void function (a function may have both
    // a value return and `out` parameters); only `void` procedures have no return.
    if (retType != "void")
        sig += " -> " + mapType(retType);
    sig += tail.empty() ? "" : " " + tail;

    rewritten.sig = std::move(sig);
    return rewritten;
}

// True when a line begins a function definition: `<knownReturnType> <name> (`
// with no assignment. Used to start multi-line signature accumulation.
bool looksLikeSignatureStart(const string& line)
{
    const size_t indentEnd = line.find_first_not_of(" \t");
    if (indentEnd == string::npos)
        return false;
    const size_t paren = line.find('(');
    if (paren == string::npos)
        return false;
    const string head = trim(string_view(line).substr(indentEnd, paren - indentEnd));
    if (head.empty() || head.find('=') != string::npos)
        return false;
    const size_t sp = head.find_first_of(" \t");
    if (sp == string::npos)
        return false;
    const string retType = trim(head.substr(0, sp));
    const string name = trim(head.substr(sp + 1));
    if (name.empty() || name.find_first_of(" \t") != string::npos)
        return false;
    return isKnownReturnType(retType);
}

// `(*name)` is only wrong as a lone function-call argument; undo that case so the
// callee still receives `ptr<function,T>`.
string undoDerefInCallArgs(string line, const std::vector<string>& names)
{
    for (const string& name : names)
    {
        if (name.empty())
            continue;
        const string deref = "(*" + name + ")";
        size_t pos = 0;
        while ((pos = line.find(deref, pos)) != string::npos)
        {
            size_t before = pos;
            while (before > 0 && isHorizontalSpace(line[before - 1]))
                before--;
            const bool okBefore = (before == 0) || line[before - 1] == '(' || line[before - 1] == ',';

            size_t after = pos + deref.size();
            while (after < line.size() && isHorizontalSpace(line[after]))
                after++;
            const bool okAfter = (after >= line.size()) || line[after] == ')' || line[after] == ',';

            if (okBefore && okAfter)
            {
                line.replace(pos, deref.size(), name);
                pos += name.size();
            }
            else
            {
                pos += deref.size();
            }
        }
    }
    return line;
}

// Dereference out-parameter names in expressions and assignments. Function-call
// arguments must stay as pointers (undoDerefInCallArgs restores those).
string derefOutParams(string line, const std::vector<string>& names)
{
    for (const string& name : names)
    {
        if (name.empty())
            continue;
        size_t pos = 0;
        const string repl = "(*" + name + ")";
        while ((pos = line.find(name, pos)) != string::npos)
        {
            const bool boundaryBefore = (pos == 0) || !isIdentChar(line[pos - 1]);
            const size_t after = pos + name.size();
            const bool boundaryAfter = (after >= line.size()) || !isIdentChar(line[after]);
            const bool alreadyDeref = (pos >= 2 && line[pos - 1] == '*' && line[pos - 2] == '(');
            if (boundaryBefore && boundaryAfter && !alreadyDeref)
            {
                line.replace(pos, name.size(), repl);
                pos += repl.size();
            }
            else
            {
                pos = after;
            }
        }
    }
    return undoDerefInCallArgs(line, names);
}

// A braceless control header `if (...)`, `else if (...)`, `for (...)`, `while (...)`
// or `else` with the single-statement body on the FOLLOWING line (GLSL allows this;
// WGSL requires a `{ }` block). The body-on-same-line case is left to the caller.
bool looksLikeControlHeader(const string& lineIn)
{
    const string t = trim(lineIn);
    if (t.find('{') != string::npos)
        return false; // already braced on this line
    if (t.compare(0, 4, "else") == 0 && (t.size() == 4 || isHorizontalSpace(t[4])))
    {
        size_t after = 4;
        while (after < t.size() && isHorizontalSpace(t[after]))
            after++;
        if (after >= t.size())
            return true; // bare `else`, body on next line
        if (t.compare(after, 2, "//") == 0 || t.compare(after, 2, "/*") == 0)
            return true; // `else // comment`, braced body on next line
    }
    size_t paren = string::npos;
    if (t.compare(0, 3, "if ") == 0 || t.compare(0, 3, "if(") == 0 ||
        t.compare(0, 8, "else if ") == 0 || t.compare(0, 7, "else if") == 0 ||
        t.compare(0, 4, "for ") == 0 || t.compare(0, 4, "for(") == 0 ||
        t.compare(0, 6, "while ") == 0 || t.compare(0, 6, "while(") == 0)
        paren = t.find('(');
    if (paren == string::npos)
        return false;
    int depth = 0;
    size_t close = string::npos;
    for (size_t i = paren; i < t.size(); ++i)
    {
        if (t[i] == '(')
            depth++;
        else if (t[i] == ')')
        {
            depth--;
            if (depth == 0)
            {
                close = i;
                break;
            }
        }
    }
    if (close == string::npos)
        return false;
    return trim(t.substr(close + 1)).empty(); // nothing after ')' -> body is next line
}

// WGSL requires `if`/`while` conditions to be `bool`. MaterialX stores boolean HW uniforms
// (e.g. `u_refractionTwoSided`) as `i32`, because WGSL uniform buffers cannot hold `bool`.
// A GLSL boolean condition such as `if ($refractionTwoSided)` is substituted to
// `if (u_refractionTwoSided)`, which WGSL rejects ("if condition must be bool, got i32").
//
// Rewrite a condition that is exactly a single `u_`-prefixed uniform reference to an explicit
// `!= 0` comparison. Only `u_` identifiers are touched, since those are precisely the
// bool-stored-as-i32 HW uniforms; genuine `bool` locals/params (never `u_`-prefixed) are left
// untouched so no valid `bool` condition is broken.
string rewriteBoolUniformCondition(string line)
{
    static const char* const KEYWORDS[] = { "if", "while" };
    for (const char* kw : KEYWORDS)
    {
        const string k = kw;
        for (size_t pos = 0; (pos = line.find(k, pos)) != string::npos;)
        {
            const bool boundaryBefore = (pos == 0) || !isIdentChar(line[pos - 1]);
            size_t i = pos + k.size();
            while (i < line.size() && isHorizontalSpace(line[i]))
                i++;
            if (!boundaryBefore || i >= line.size() || line[i] != '(')
            {
                pos += k.size();
                continue;
            }
            const size_t open = i;
            size_t identStart = open + 1;
            while (identStart < line.size() && isHorizontalSpace(line[identStart]))
                identStart++;
            if (identStart + 1 < line.size() && line[identStart] == 'u' && line[identStart + 1] == '_')
            {
                size_t identEnd = identStart;
                while (identEnd < line.size() && isIdentChar(line[identEnd]))
                    identEnd++;
                size_t afterIdent = identEnd;
                while (afterIdent < line.size() && isHorizontalSpace(line[afterIdent]))
                    afterIdent++;
                if (afterIdent < line.size() && line[afterIdent] == ')')
                {
                    const string ident = line.substr(identStart, identEnd - identStart);
                    const string replacement = "(" + ident + " != 0)";
                    line.replace(open, afterIdent - open + 1, replacement);
                    pos = open + replacement.size();
                    continue;
                }
            }
            pos += k.size();
        }
    }
    return line;
}

// Split a GLSL `sampler2D tex` parameter into the WGSL texture + sampler pair.
string rewriteSamplerParams(string line)
{
    const string token = "sampler2D ";
    size_t pos = 0;
    while ((pos = line.find(token, pos)) != string::npos)
    {
        size_t nameStart = pos + token.size();
        const string name = readIdentifier(line, nameStart);
        if (name.empty())
        {
            pos += token.size();
            continue;
        }
        const string repl = name + "_texture: texture_2d<f32>, " + name + "_sampler: sampler";
        line.replace(pos, nameStart - pos, repl);
        pos += repl.size();
    }
    return line;
}

// Rewrite a GLSL `texture(tex, uv)` read to the WGSL `mx_texture_sample(...)` helper.
string rewriteTextureSampling(string line)
{
    const string token = "texture(";
    size_t pos = 0;
    while ((pos = line.find(token, pos)) != string::npos)
    {
        if (pos > 0 && isIdentChar(line[pos - 1]))
        {
            pos += token.size();
            continue;
        }
        size_t argStart = pos + token.size();
        const string sampler = readIdentifier(line, argStart);
        if (sampler.empty() || argStart >= line.size() || line[argStart] != ',')
        {
            pos += token.size();
            continue;
        }
        const string repl = "mx_texture_sample(" + sampler + "_texture, " + sampler + "_sampler,";
        line.replace(pos, (argStart + 1) - pos, repl);
        pos += repl.size();
    }
    return line;
}

// Rewrite a whole multi-line GLSL block to WGSL, line by line. `#include` directives are
// left verbatim so the caller can resolve and rewrite them with path context.
string rewriteBlock(const string& block)
{
    std::istringstream stream(block);
    string line;
    string result;
    result.reserve(block.size() + block.size() / 8);
    LineRewriter rewriter;
    while (std::getline(stream, line))
    {
        bool hadCr = !line.empty() && line.back() == '\r';
        if (hadCr)
            line.pop_back();

        size_t firstNonSpace = line.find_first_not_of(" \t");
        bool isInclude = (firstNonSpace != string::npos &&
                          line.compare(firstNonSpace, 8, "#include") == 0);
        if (!isInclude)
            line = rewriter.rewrite(line);

        result += line;
        if (hadCr)
            result += '\r';
        result += '\n';
    }
    return result;
}

// Rewrite GLSL combined-sampler reads to WGSL split form:
//   texture(sampler2D(A, B), C)    -> textureSample(A, B, C)
//   textureLod(sampler2D(A, B), C, D) -> textureSampleLevel(A, B, C, D)
string rewriteCombinedTextureSampling(string line)
{
    struct Form
    {
        const char* glsl;
        const char* wgsl;
    };
    const Form forms[] = {
        { "texture(sampler2D(", "textureSample(" },
        { "textureLod(sampler2D(", "textureSampleLevel(" },
    };
    for (const Form& f : forms)
    {
        const string token = f.glsl;
        size_t pos = 0;
        while ((pos = line.find(token, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(line[pos - 1]))
            {
                pos += token.size();
                continue;
            }
            // Find the close of the inner sampler2D( ... ).
            size_t inner = pos + token.size();
            int depth = 1;
            size_t i = inner;
            for (; i < line.size() && depth > 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                    depth--;
            }
            if (depth != 0)
            {
                pos += token.size();
                continue;
            }
            const size_t innerClose = i - 1; // index of the ')' closing sampler2D(
            // Expect a following comma (the remaining args of the sample call).
            size_t comma = innerClose + 1;
            while (comma < line.size() && (line[comma] == ' ' || line[comma] == '\t'))
                comma++;
            if (comma >= line.size() || line[comma] != ',')
            {
                pos += token.size();
                continue;
            }
            const string innerArgs = line.substr(inner, innerClose - inner); // "A, B"
            const string repl = string(f.wgsl) + innerArgs + ",";
            line.replace(pos, (comma + 1) - pos, repl);
            pos += repl.size();
        }
    }
    return line;
}

// Ordinary (non-signature) line rewrite: shared GLSL→WGSL syntax conversions
// plus the WebGPU-specific sampler/texture deltas.
string rewriteNonSignatureLine(string line)
{
    // Reserved-word identifier rename runs only on raw library GLSL (this path),
    // before declarations introduce the WGSL `var` keyword — never on the
    // already-WGSL lines that reach rewriteAll via the generator's emitLine.
    line = rewriteReservedIdents(std::move(line));
    line = rewriteSamplerParams(std::move(line));
    line = rewriteAll(std::move(line));
    line = rewriteCombinedTextureSampling(std::move(line));
    line = rewriteTextureSampling(std::move(line));
    return line;
}

// Known compile-time macro values used to evaluate preprocessor conditionals.
// DIRECTIONAL_ALBEDO_METHOD 0 selects the self-contained analytic albedo path
// (no albedo-table texture).
int knownMacroValue(const string& name, bool& known)
{
    known = true;
    if (name == "DIRECTIONAL_ALBEDO_METHOD")
        return 0;
    known = false;
    return 0;
}

// Evaluate a simple `#if`/`#elif` expression: `IDENT == INT`, `IDENT != INT`,
// `defined(IDENT)`, or a bare identifier/number. Unknown macros evaluate to 0.
bool evalPPExpr(const string& exprIn)
{
    string expr = trim(exprIn);
    if (expr.compare(0, 8, "defined(") == 0)
        return true; // treat referenced macros as defined
    const size_t eq = expr.find("==");
    const size_t ne = expr.find("!=");
    if (eq != string::npos || ne != string::npos)
    {
        const bool isEq = (eq != string::npos);
        const size_t op = isEq ? eq : ne;
        const string lhs = trim(expr.substr(0, op));
        const string rhs = trim(expr.substr(op + 2));
        bool known;
        const int lval = knownMacroValue(lhs, known);
        const int rval = std::atoi(rhs.c_str());
        return isEq ? (lval == rval) : (lval != rval);
    }
    // Bare identifier/number: nonzero is true.
    bool known;
    const int v = (expr.find_first_not_of("0123456789") == string::npos) ? std::atoi(expr.c_str()) : knownMacroValue(expr, known);
    return v != 0;
}

// Make a WGSL type name safe to embed in an identifier suffix.
string typeCode(const string& type)
{
    string t = type;
    for (char& c : t)
        if (!std::isalnum(static_cast<unsigned char>(c)))
            c = '_';
    return t;
}

// Split a function parameter list into the ordered parameter type strings.
std::vector<string> paramTypes(const string& params)
{
    std::vector<string> types;
    for (const string& p : splitTopLevel(params))
    {
        const size_t colon = p.find(':');
        if (colon != string::npos)
            types.push_back(trim(p.substr(colon + 1)));
    }
    return types;
}

// Infer the WGSL type of an argument expression from the symbol tables.
string inferArgType(const string& argIn,
                    const std::map<string, string>& locals,
                    const std::map<string, string>& globals,
                    const std::map<string, std::map<string, string>>& structs)
{
    string a = trim(argIn);
    if (a.empty())
        return "";
    // Strip a fully-enclosing pair of parentheses: "(expr)" -> "expr".
    while (a.size() >= 2 && a.front() == '(' && a.back() == ')')
    {
        int depth = 0;
        bool wraps = true;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i] == '(')
                depth++;
            else if (a[i] == ')')
                depth--;
            if (depth == 0 && i + 1 < a.size())
            {
                wraps = false;
                break;
            }
        }
        if (!wraps)
            break;
        a = trim(a.substr(1, a.size() - 2));
        if (a.empty())
            return "";
    }
    if (a == "true" || a == "false")
        return "bool";
    // Numeric literal — the WHOLE token must be numeric (else it is an expression).
    if ((std::isdigit(static_cast<unsigned char>(a[0])) || a[0] == '-' || a[0] == '+') &&
        a.find_first_not_of("0123456789.eE+-fF") == string::npos &&
        a.find_first_of("0123456789") != string::npos)
        return (a.find('.') != string::npos || a.find('e') != string::npos || a.find('E') != string::npos || a.back() == 'f') ? "f32" : "i32";
    // Constructor expression.
    static constexpr string_view CTORS[] = {
        "vec2f", "vec3f", "vec4f", "mat2x2f", "mat3x3f", "mat4x4f", "f32", "i32", "u32"
    };
    for (const string_view c : CTORS)
        if (startsWith(a, c) && a.size() > c.size() && a[c.size()] == '(')
            return string(c.data(), c.size());
    if (startsWith(a, "vec3<"))
        return "vec3f";
    if (startsWith(a, "vec2<"))
        return "vec2f";
    if (startsWith(a, "vec4<"))
        return "vec4f";

    // Type-preserving unary built-in call f(x): the result type equals x's type.
    {
        const size_t lp = a.find('(');
        if (lp != string::npos && !a.empty() && a.back() == ')' &&
            a.substr(0, lp).find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
        {
            const string fn = a.substr(0, lp);
            static const char* PRESERVE[] = {
                "sqrt", "abs", "normalize", "floor", "ceil", "fract", "exp", "exp2",
                "log", "log2", "sin", "cos", "tan", "asin", "acos", "inverseSqrt",
                "sign", "radians", "degrees", "saturate", "mx_square", "pow", "reflect"
            };
            for (const char* p : PRESERVE)
                if (fn == p)
                {
                    const std::vector<string> inner = splitTopLevel(a.substr(lp + 1, a.size() - lp - 2));
                    return inner.empty() ? string() : inferArgType(inner[0], locals, globals, structs);
                }
            // User-function call: use its recorded return type (globals "fn:" prefix).
            const auto fr = globals.find("fn:" + fn);
            if (fr != globals.end() && fr->second != "void")
                return fr->second;
        }
    }

    // Top-level binary operator (a OP b): the expression type is the vector operand
    // if either is a float vector, else the first known operand type.
    {
        int paren = 0;
        for (size_t i = a.size(); i-- > 0;)
        {
            const char c = a[i];
            if (c == ')' || c == ']' || c == '>')
                paren++;
            else if (c == '(' || c == '[' || c == '<')
                paren--;
            else if (paren == 0 && (c == '+' || c == '-' || c == '*' || c == '/'))
            {
                // Require a complete operand to the left (else it is unary / not an operator).
                size_t j = i;
                while (j > 0 && isHorizontalSpace(a[j - 1]))
                    j--;
                if (j == 0)
                    continue;
                const char prev = a[j - 1];
                if (!(isIdentChar(prev) || prev == ')' || prev == ']'))
                    continue;
                const string lt = inferArgType(a.substr(0, i), locals, globals, structs);
                const string rt = inferArgType(a.substr(i + 1), locals, globals, structs);
                if (lt == "vec2f" || lt == "vec3f" || lt == "vec4f")
                    return lt;
                if (rt == "vec2f" || rt == "vec3f" || rt == "vec4f")
                    return rt;
                if (!lt.empty())
                    return lt;
                if (!rt.empty())
                    return rt;
                return "";
            }
        }
    }

    // Swizzle access base.{xyzw|rgba}: 1 component -> f32, 2/3/4 -> vecNf.
    {
        const size_t dot = a.rfind('.');
        if (dot != string::npos && dot + 1 < a.size())
        {
            const string sw = a.substr(dot + 1);
            if (sw.size() >= 1 && sw.size() <= 4 && sw.find_first_not_of("xyzwrgba") == string::npos)
                return sw.size() == 1 ? string("f32") : ("vec" + std::to_string(sw.size()) + "f");
        }
    }

    // Member access X.Y (single level).
    const size_t dot = a.find('.');
    if (dot != string::npos && a.find('(') == string::npos)
    {
        const string base = a.substr(0, dot);
        const string member = a.substr(dot + 1);
        auto bt = locals.count(base) ? locals.find(base) : globals.find(base);
        if (bt != (locals.count(base) ? locals.end() : globals.end()))
        {
            auto s = structs.find(bt->second);
            if (s != structs.end())
            {
                auto m = s->second.find(member);
                if (m != s->second.end())
                    return m->second;
            }
        }
        return "";
    }
    // Plain identifier.
    if (a.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
    {
        auto l = locals.find(a);
        if (l != locals.end())
            return l->second;
        auto g = globals.find(a);
        if (g != globals.end())
            return g->second;
    }
    return "";
}

// Collect every `var/let/const NAME: TYPE` declaration on a line into `out`
// (a line may declare several, e.g. `var a: vec3f; var b: vec3f;`).
void collectVarDecl(const string& line, std::map<string, string>& out)
{
    for (const char* kw : { "var ", "let ", "const " })
    {
        const string kwStr = kw;
        size_t k = 0;
        while ((k = line.find(kwStr, k)) != string::npos)
        {
            // Whole-word keyword (preceded by a non-identifier char).
            if (k > 0 && isIdentChar(line[k - 1]))
            {
                k += kwStr.size();
                continue;
            }
            size_t p = k + kwStr.size();
            while (p < line.size() && isHorizontalSpace(line[p]))
                p++;
            size_t nameEnd = p;
            while (nameEnd < line.size() && isIdentChar(line[nameEnd]))
                nameEnd++;
            const string name = line.substr(p, nameEnd - p);
            size_t c = nameEnd;
            while (c < line.size() && isHorizontalSpace(line[c]))
                c++;
            if (c >= line.size() || line[c] != ':')
            {
                k = p;
                continue;
            }
            size_t ts = c + 1;
            while (ts < line.size() && isHorizontalSpace(line[ts]))
                ts++;
            size_t te = ts;
            int angle = 0;
            while (te < line.size())
            {
                char ch = line[te];
                if (ch == '<')
                    angle++;
                else if (ch == '>')
                    angle--;
                else if (angle == 0 && (ch == ';' || ch == '=' || isHorizontalSpace(ch)))
                    break;
                te++;
            }
            if (te > ts && !name.empty())
                out[name] = trim(line.substr(ts, te - ts));
            k = te;
        }
    }
}

bool isScalarType(const string& t)
{
    return t == "f32" || t == "i32" || t == "u32" || t == "f16";
}
bool isVecFloatType(const string& t)
{
    return t == "vec2f" || t == "vec3f" || t == "vec4f";
}

// GLSL broadcasts scalar arguments in component-wise built-ins (max(vec3, 0.0));
// WGSL requires matching types. Promote scalar args to the call's vector type:
// max(v, 0.1) -> max(v, vec3f(0.1)).
string rewriteScalarBroadcast(string line,
                              const std::map<string, string>& locals,
                              const std::map<string, string>& globals,
                              const std::map<string, std::map<string, string>>& structs)
{
    static const char* BUILTINS[] = { "max(", "min(", "pow(", "mod(", "step(", "clamp(", "smoothstep(" };
    for (const char* b : BUILTINS)
    {
        const string tok = b;
        size_t pos = 0;
        while ((pos = line.find(tok, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(line[pos - 1]))
            {
                pos += tok.size();
                continue;
            }
            const size_t argStart = pos + tok.size();
            int depth = 1;
            size_t i = argStart;
            for (; i < line.size() && depth > 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                    depth--;
            }
            if (depth != 0)
                break; // unterminated call on this line; nothing more to match
            const size_t close = i - 1;
            std::vector<string> args = splitTopLevel(line.substr(argStart, close - argStart));

            string vecType;
            std::vector<string> types;
            for (const string& a : args)
            {
                const string t = inferArgType(a, locals, globals, structs);
                types.push_back(t);
                if (vecType.empty() && isVecFloatType(t))
                    vecType = t;
            }
            bool changed = false;
            if (!vecType.empty())
            {
                for (size_t a = 0; a < args.size(); ++a)
                    if (isScalarType(types[a]))
                    {
                        args[a] = vecType + "(" + args[a] + ")";
                        changed = true;
                    }
            }
            if (changed)
            {
                string rebuilt = tok;
                for (size_t a = 0; a < args.size(); ++a)
                    rebuilt += (a ? ", " : "") + args[a];
                rebuilt += ")";
                line.replace(pos, (close + 1) - pos, rebuilt);
                pos += rebuilt.size();
            }
            else
            {
                pos += tok.size();
            }
        }
    }
    return line;
}

// GLSL `mix(x, y, boolMask)` has no WGSL equivalent (mix needs a float t); it maps
// to `select(x, y, boolMask)` (same argument order). Rewrite when the 3rd argument
// infers to a boolean type.
string rewriteMixToSelect(string line,
                          const std::map<string, string>& locals,
                          const std::map<string, string>& globals,
                          const std::map<string, std::map<string, string>>& structs)
{
    const string tok = "mix(";
    size_t pos = 0;
    while ((pos = line.find(tok, pos)) != string::npos)
    {
        if (pos > 0 && isIdentChar(line[pos - 1]))
        {
            pos += tok.size();
            continue;
        }
        const size_t argStart = pos + tok.size();
        int depth = 1;
        size_t i = argStart;
        for (; i < line.size() && depth > 0; ++i)
        {
            if (line[i] == '(')
                depth++;
            else if (line[i] == ')')
                depth--;
        }
        if (depth != 0)
        {
            pos += tok.size();
            break;
        }
        const std::vector<string> args = splitTopLevel(line.substr(argStart, i - 1 - argStart));
        if (args.size() == 3)
        {
            const string t = inferArgType(args[2], locals, globals, structs);
            if (t.find("bool") != string::npos)
            {
                line.replace(pos, 3, "select");
                pos += string("select").size();
                continue;
            }
        }
        pos += tok.size();
    }
    return line;
}

// Insert `&` before out-arguments at call sites of functions with `ptr<function,T>`
// parameters (GLSL passes out-args by bare name; WGSL needs the address). `fnPtr`
// maps function name -> list of per-overload ptr-position flag vectors.
string addAddressOfToOutArgs(string line,
                             const std::map<string, std::vector<std::vector<bool>>>& fnPtr,
                             const std::map<string, string>& locals)
{
    for (const auto& entry : fnPtr)
    {
        const string token = entry.first + "(";
        size_t pos = 0;
        while ((pos = line.find(token, pos)) != string::npos)
        {
            const bool defHere = (pos >= 3 && line.compare(pos - 3, 3, "fn ") == 0);
            if ((pos > 0 && isIdentChar(line[pos - 1])) || defHere)
            {
                pos += token.size();
                continue;
            }
            const size_t argStart = pos + token.size();
            int depth = 1;
            size_t i = argStart;
            for (; i < line.size() && depth > 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                    depth--;
            }
            if (depth != 0)
            {
                pos += token.size();
                break;
            }
            const size_t close = i - 1;
            std::vector<string> args = splitTopLevel(line.substr(argStart, close - argStart));

            const std::vector<bool>* flags = nullptr;
            for (const auto& f : entry.second)
                if (f.size() == args.size())
                {
                    flags = &f;
                    break;
                }
            bool changed = false;
            if (flags)
            {
                for (size_t a = 0; a < args.size(); ++a)
                {
                    if (!(*flags)[a])
                        continue;
                    const string arg = trim(args[a]);
                    if (arg.empty() || arg[0] == '&')
                        continue;
                    if (arg.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos)
                        continue;
                    const auto it = locals.find(arg);
                    if (it != locals.end() && it->second.compare(0, 3, "ptr") == 0)
                        continue; // already a pointer
                    args[a] = "&" + arg;
                    changed = true;
                }
            }
            if (changed)
            {
                string rebuilt = token;
                for (size_t a = 0; a < args.size(); ++a)
                    rebuilt += (a ? ", " : "") + args[a];
                rebuilt += ")";
                line.replace(pos, (close + 1) - pos, rebuilt);
                pos += rebuilt.size();
            }
            else
            {
                pos += token.size();
            }
        }
    }
    return line;
}

// Rewrite overloaded call sites on a line using the symbol tables.
string rewriteOverloadCalls(string line,
                            const std::map<string, std::vector<std::pair<std::vector<string>, string>>>& overloads,
                            const std::map<string, string>& locals,
                            const std::map<string, string>& globals,
                            const std::map<string, std::map<string, string>>& structs)
{
    for (const auto& entry : overloads)
    {
        const string& name = entry.first;
        const string token = name + "(";
        size_t pos = 0;
        while ((pos = line.find(token, pos)) != string::npos)
        {
            // Skip the definition itself and identifier-suffix false matches.
            const bool defHere = (pos >= 3 && line.compare(pos - 3, 3, "fn ") == 0);
            if ((pos > 0 && isIdentChar(line[pos - 1])) || defHere)
            {
                pos += token.size();
                continue;
            }

            size_t argStart = pos + token.size();
            int depth = 1;
            size_t i = argStart;
            for (; i < line.size() && depth > 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                    depth--;
            }
            if (depth != 0)
            {
                pos += token.size();
                break;
            }
            const size_t close = i - 1;
            const std::vector<string> args = splitTopLevel(line.substr(argStart, close - argStart));

            // Candidate overloads with the matching argument count.
            string chosen;
            int bestScore = -1;
            for (const auto& ov : entry.second)
            {
                if (ov.first.size() != args.size())
                    continue;
                int score = 0;
                bool mismatch = false;
                for (size_t a = 0; a < args.size(); ++a)
                {
                    const string t = inferArgType(args[a], locals, globals, structs);
                    if (t.empty())
                        continue; // unknown: neither matches nor conflicts
                    if (t == ov.first[a])
                        score++;
                    else
                    {
                        mismatch = true;
                        break;
                    }
                }
                if (!mismatch && score > bestScore)
                {
                    bestScore = score;
                    chosen = ov.second;
                }
            }
            if (!chosen.empty() && chosen != name)
            {
                line.replace(pos, name.size(), chosen);
                pos += chosen.size() + 1;
            }
            else
            {
                pos += token.size();
            }
        }
    }
    return line;
}
} // namespace

// ─── Public entry points ────────────────────────────────────────────────────

string mapType(const string& glslType)
{
    if (glslType == "texture2D")
        return "texture_2d<f32>";
    const string_view mapped = lookupWgslType(glslType);
    return string(mapped.data(), mapped.size());
}

string rewriteAll(string line)
{
    // Preprocessor defines become WGSL const/alias declarations.
    const size_t firstNonSpace = line.find_first_not_of(" \t");
    if (firstNonSpace != string::npos && line.compare(firstNonSpace, 7, "#define") == 0)
        return rewriteDefine(line);

    line = rewriteScalarCasts(std::move(line));
    line = rewriteConst(line);
    line = rewriteTernaries(std::move(line));
    line = rewriteIncDec(std::move(line));
    line = rewriteBracelessControlSameLine(std::move(line));
    line = rewriteChainedDecls(std::move(line));
    line = rewriteMultiDecl(std::move(line));
    line = rewriteVariableDecl(std::move(line));
    line = rewriteStructVariableDecl(std::move(line));
    line = rewriteForLoopInit(std::move(line));
    line = rewriteVectorCompare(std::move(line));
    line = rewriteMatrixCtors(std::move(line));
    line = rewriteMathBuiltins(std::move(line));
    line = rewriteSampleLightSource(std::move(line));
    line = rewriteBoolUniformCondition(std::move(line));
    line = rewriteFloatLiteralSuffix(std::move(line));
    // Collapse the occasional doubled statement terminator (WGSL has no empty statement).
    for (size_t p; (p = line.find(";;")) != string::npos;)
        line.replace(p, 2, ";");
    return line;
}

// ─── LineRewriter ───────────────────────────────────────────────────────────

string LineRewriter::beginFunction(const string& sig, std::vector<string>&& outNames,
                                   const std::vector<std::pair<string, string>>& valueParams)
{
    _func.active = true;
    _func.seenOpenBrace = false;
    _func.braceDepth = 0;
    _func.injected = false;
    _func.outParams = std::move(outNames);

    // Build the mutable `var` copies for value parameters.
    _func.paramInjections.clear();
    for (const auto& vp : valueParams)
        _func.paramInjections.push_back("    var " + vp.first + ": " + vp.second + " = " + vp.first + ARG_SUFFIX + ";");

    string out = sig;
    countBraces(sig, _func.braceDepth, _func.seenOpenBrace);
    if (_func.seenOpenBrace)
    {
        // The signature line also opened the body brace (single-line function, or
        // brace on the signature line): inject the var copies just after the `{`
        // so they land inside the body, not after a single-line function's `}`.
        const size_t bpos = out.find('{');
        if (bpos != string::npos && !_func.paramInjections.empty())
        {
            string inj;
            for (const string& i : _func.paramInjections)
                inj += " " + trim(i);
            out.insert(bpos + 1, inj);
        }
        _func.injected = true;
    }
    if (_func.seenOpenBrace && _func.braceDepth <= 0)
        leaveFunctionScope();
    return out;
}

void LineRewriter::leaveFunctionScope()
{
    _func.active = false;
    _func.outParams.clear();
    _func.paramInjections.clear();
}

// Rewrite a GLSL struct member `type name;` to a WGSL member `name: type,`.
static string rewriteStructMember(const string& line)
{
    const size_t indentEnd = line.find_first_not_of(" \t");
    if (indentEnd == string::npos)
        return line;
    const string indent = line.substr(0, indentEnd);
    string body = line.substr(indentEnd);
    // Drop a trailing ';'.
    const size_t semi = body.find(';');
    if (semi == string::npos)
        return line; // not a simple member declaration
    body = body.substr(0, semi);
    const size_t sp = body.find_first_of(" \t");
    if (sp == string::npos)
        return line;
    const string type = mapType(body.substr(0, sp));
    const string name = body.substr(body.find_first_not_of(" \t", sp));
    if (name.empty() || name.find_first_of(" \t") != string::npos)
        return line;
    return indent + name + ": " + type + ",";
}

bool LineRewriter::Preprocessor::active() const
{
    for (const Frame& f : stack)
        if (!f.active)
            return false;
    return true;
}

void LineRewriter::Preprocessor::update(const string& t)
{
    auto directive = [&](string_view d) { return startsWith(trimView(t), d); };
    if (directive("#ifdef") || directive("#ifndef") || directive("#if"))
    {
        const bool parentActive = active();
        bool cond;
        if (directive("#ifdef"))
            cond = true; // referenced macros treated as defined
        else if (directive("#ifndef"))
            cond = false;
        else
            cond = evalPPExpr(t.substr(3));
        const bool on = parentActive && cond;
        stack.push_back({ on, on, parentActive });
    }
    else if (directive("#elif"))
    {
        if (!stack.empty())
        {
            Frame& f = stack.back();
            const bool cond = evalPPExpr(t.substr(5));
            f.active = f.parentActive && !f.taken && cond;
            f.taken = f.taken || f.active;
        }
    }
    else if (directive("#else"))
    {
        if (!stack.empty())
        {
            Frame& f = stack.back();
            f.active = f.parentActive && !f.taken;
            f.taken = true;
        }
    }
    else if (directive("#endif"))
    {
        if (!stack.empty())
            stack.pop_back();
    }
}

string LineRewriter::rewrite(const string& line)
{
    string out;
    if (handlePreprocessor(line, out))
        return out;
    if (handleStruct(line, out))
        return out;
    if (_func.active)
        return rewriteFunctionBodyLine(line);
    if (handleSignature(line, out))
        return out;
    return rewriteNonSignatureLine(line);
}

// Minimal C preprocessor for `#if/#elif/#else/#endif`: evaluate conditionals (WGSL has
// none) and drop both the directive lines and any lines in inactive branches.
bool LineRewriter::handlePreprocessor(const string& line, string& out)
{
    const string t = trim(line);
    if (!t.empty() && t[0] == '#' &&
        (startsWith(t, "#if") || startsWith(t, "#elif") || startsWith(t, "#else") || startsWith(t, "#endif")))
    {
        _pp.update(t);
        out.clear();
        return true;
    }
    if (!_pp.active())
    {
        out.clear(); // inside an inactive preprocessor branch
        return true;
    }
    return false;
}

// Struct definition body: rewrite GLSL members to WGSL and drop the trailing semicolon
// on the closing brace. Entered by a `struct Name {` / `struct Name` header.
bool LineRewriter::handleStruct(const string& line, string& out)
{
    const string t = trim(line);
    if (_inStruct)
    {
        if (t == "};" || t == "}")
        {
            _inStruct = false;
            const size_t indentEnd = line.find_first_not_of(" \t");
            out = (indentEnd == string::npos ? "" : line.substr(0, indentEnd)) + "}";
        }
        else if (t.empty() || t[0] == '/')
        {
            out = line; // blank or comment
        }
        else
        {
            out = rewriteStructMember(line);
        }
        return true;
    }
    // Enter struct mode for `struct Name {` or `struct Name` (brace on next line),
    // but not a single-line `struct Name { ... };`.
    if (startsWith(t, "struct ") && t.find('}') == string::npos)
    {
        _inStruct = true;
        out = line; // valid WGSL as-is
        return true;
    }
    return false;
}

// A line inside a function body: resolve buffered braceless headers and multi-line
// ternaries, dereference out parameters, and inject value-parameter copies.
string LineRewriter::rewriteFunctionBodyLine(const string& line)
{
    // Resolve a buffered braceless control header now that we can see its body.
    if (_pendingHeader.active)
    {
        const string bt = trim(line);
        // Skip blank lines and comments between the header and its body.
        if (bt.empty() || startsWith(bt, "//") || startsWith(bt, "/*"))
            return "";
        _pendingHeader.active = false;
        const string hdr = rewriteNonSignatureLine(derefOutParams(_pendingHeader.line, _func.outParams));
        const string body = rewriteNonSignatureLine(derefOutParams(line, _func.outParams));
        countBraces(line, _func.braceDepth, _func.seenOpenBrace);
        if (_func.seenOpenBrace && _func.braceDepth <= 0)
            leaveFunctionScope();
        if (!bt.empty() && bt[0] == '{')
            return hdr + "\n" + body;       // body is a normal `{` block
        return hdr + " {\n" + body + "\n}"; // wrap the single-statement body
    }
    if (looksLikeControlHeader(line))
    {
        _pendingHeader.active = true;
        _pendingHeader.line = line;
        return "";
    }

    // Accumulate a ternary expression that spans multiple lines (the per-line rewrite
    // needs the whole `cond ? a : b` on one line). A continuation begins when a line
    // ends with `?` and runs until the statement terminator `;`.
    if (_ternary.active)
    {
        _ternary.buffer += " " + trim(line);
        if (!trim(line).empty() && trim(line).back() == ';')
        {
            const string joined = _ternary.buffer;
            _ternary.active = false;
            _ternary.buffer.clear();
            const string done = rewriteNonSignatureLine(derefOutParams(joined, _func.outParams));
            countBraces(joined, _func.braceDepth, _func.seenOpenBrace);
            if (_func.seenOpenBrace && _func.braceDepth <= 0)
                leaveFunctionScope();
            return done;
        }
        return "";
    }
    if (!trim(line).empty() && trim(line).back() == '?')
    {
        _ternary.active = true;
        _ternary.buffer = trim(line);
        return "";
    }

    // Ordinary body line: dereference out parameters, then apply the shared syntax
    // rewrites (body lines never match the signature heuristic).
    string out = rewriteNonSignatureLine(derefOutParams(line, _func.outParams));

    const bool wasOpen = _func.seenOpenBrace;
    countBraces(line, _func.braceDepth, _func.seenOpenBrace);
    if (!_func.injected && !wasOpen && _func.seenOpenBrace)
    {
        // The body brace just opened on this line: inject the value-param copies.
        for (const string& inj : _func.paramInjections)
            out += "\n" + inj;
        _func.injected = true;
    }
    if (_func.seenOpenBrace && _func.braceDepth <= 0)
        leaveFunctionScope();
    return out;
}

// A function signature: single-line, or a parameter list spanning several lines.
bool LineRewriter::handleSignature(const string& line, string& out)
{
    // Continuation of a signature whose parameter list spans multiple lines.
    if (_sig.active)
    {
        _sig.buffer += " " + line;
        _sig.parenDepth += netParens(line);
        if (_sig.parenDepth > 0)
        {
            out.clear(); // still open: defer emission until the list closes
            return true;
        }
        const string joined = _sig.buffer;
        _sig.active = false;
        _sig.buffer.clear();
        if (auto rewritten = rewriteSignature(joined))
            out = beginFunction(rewritten->sig, std::move(rewritten->outNames), std::move(rewritten->valueParams));
        else
            out = rewriteNonSignatureLine(joined); // not a signature after all
        return true;
    }

    // Single-line signature.
    if (auto rewritten = rewriteSignature(line))
    {
        out = beginFunction(rewritten->sig, std::move(rewritten->outNames), std::move(rewritten->valueParams));
        return true;
    }

    // Signature whose parameter list opens here but closes on a later line.
    if (looksLikeSignatureStart(line) && netParens(line) > 0)
    {
        _sig.active = true;
        _sig.buffer = line;
        _sig.parenDepth = netParens(line);
        out.clear();
        return true;
    }
    return false;
}

// ─── Overload resolution ─────────────────────────────────────────────────────

string derefPointerParams(const string& shader)
{
    std::istringstream in(shader);
    std::ostringstream out;
    string line;

    std::vector<string> fnLines;   // buffered current function
    std::vector<string> ptrParams; // its ptr<function,...> parameter names
    bool inFn = false, seenDeref = false;
    int depth = 0;

    auto flush = [&]()
    {
        // Compound functions assign to out params by bare name (`out1 = ...`); library
        // functions already use `(*out1)`. Only deref when no `(*` is present.
        if (!ptrParams.empty() && !seenDeref)
            for (size_t i = 1; i < fnLines.size(); ++i)
                fnLines[i] = derefOutParams(fnLines[i], ptrParams);
        for (const string& l : fnLines)
            out << l << "\n";
        fnLines.clear();
        ptrParams.clear();
        inFn = false;
        seenDeref = false;
        depth = 0;
    };

    while (std::getline(in, line))
    {
        if (!inFn)
        {
            if (auto def = parseFnDef(line))
            {
                inFn = true;
                seenDeref = (line.find("(*") != string::npos);
                for (const string& p : splitTopLevel(def->params))
                {
                    const size_t colon = p.find(':');
                    if (colon != string::npos && p.find("ptr<function", colon) != string::npos)
                        ptrParams.push_back(trim(p.substr(0, colon)));
                }
                fnLines.push_back(line);
                depth = braceDelta(line);
                if (depth <= 0 && line.find('{') != string::npos)
                    flush(); // single-line fn
                continue;
            }
            out << line << "\n";
            continue;
        }
        fnLines.push_back(line);
        if (line.find("(*") != string::npos)
            seenDeref = true;
        depth += braceDelta(line);
        if (depth <= 0)
            flush();
    }
    if (inFn)
        flush();
    return out.str();
}

string dedupDefinitions(const string& shader)
{
    std::istringstream in(shader);
    std::ostringstream out;
    string line;
    std::set<string> seenConst, seenAlias, seenStruct, seenFn;
    int depth = 0;
    bool skipping = false;
    int skipBase = 0;

    while (std::getline(in, line))
    {
        if (skipping)
        {
            depth += braceDelta(line);
            if (depth <= skipBase)
                skipping = false;
            continue; // drop the duplicate block
        }

        const string t = trim(line);
        bool drop = false;
        if (depth == 0 && !t.empty())
        {
            if (t.compare(0, 6, "const ") == 0)
            {
                size_t s = 6;
                const string n = readIdentifier(t, s);
                if (!n.empty() && !seenConst.insert(n).second)
                    drop = true;
            }
            else if (t.compare(0, 6, "alias ") == 0)
            {
                size_t s = 6;
                const string n = readIdentifier(t, s);
                if (!n.empty() && !seenAlias.insert(n).second)
                    drop = true;
            }
            else if (t.compare(0, 7, "struct ") == 0)
            {
                size_t s = 7;
                const string n = readIdentifier(t, s);
                if (!n.empty() && !seenStruct.insert(n).second)
                {
                    const int d = braceDelta(line);
                    if (d > 0)
                    {
                        skipping = true;
                        skipBase = depth;
                        depth += d;
                    }
                    continue; // drop opening (and block if multi-line)
                }
            }
            else if (startsWith(t, "fn "))
            {
                if (auto def = parseFnDef(line))
                {
                    string key = def->name + "(";
                    for (const string& ty : paramTypes(def->params))
                        key += ty + ",";
                    if (!seenFn.insert(key).second)
                    {
                        const int d = braceDelta(line);
                        if (d > 0)
                        {
                            skipping = true;
                            skipBase = depth;
                            depth += d;
                        }
                        continue; // drop duplicate function (single- or multi-line)
                    }
                }
            }
        }

        depth += braceDelta(line);
        if (!drop)
            out << line << "\n";
    }
    return out.str();
}

// MaterialX booleans are represented as i32 in WGSL (uniform buffers cannot hold `bool`), but a
// handful of hand-written library functions shared with the GLSL backends declare boolean
// parameters with the native WGSL `bool` type (e.g. `retroreflective`, `energy_compensation`,
// `flip_g`). Passing a MaterialX boolean (an i32 value or integer literal) to such a parameter is
// rejected by WGSL ("cannot convert value of type 'abstract-int' to type 'bool'"). Wrap those
// integer arguments in an explicit `bool(...)` conversion. `fnBool` maps a function name to its
// per-definition flags marking which parameters are `bool`.
string coerceBoolArgs(string line,
                      const std::map<string, std::vector<std::vector<bool>>>& fnBool,
                      const std::map<string, string>& locals,
                      const std::map<string, string>& globals,
                      const std::map<string, std::map<string, string>>& structs)
{
    for (const auto& entry : fnBool)
    {
        const string token = entry.first + "(";
        for (size_t pos = 0; (pos = line.find(token, pos)) != string::npos;)
        {
            const bool defHere = (pos >= 3 && line.compare(pos - 3, 3, "fn ") == 0);
            const bool boundaryBefore = (pos == 0) || !isIdentChar(line[pos - 1]);
            if (!boundaryBefore || defHere)
            {
                pos += token.size();
                continue;
            }
            const size_t argStart = pos + token.size();
            int depth = 1;
            size_t i = argStart;
            for (; i < line.size() && depth != 0; ++i)
            {
                if (line[i] == '(')
                    depth++;
                else if (line[i] == ')')
                    depth--;
            }
            if (depth != 0)
            {
                pos += token.size();
                continue; // unterminated call on this line
            }
            const size_t close = i - 1;
            std::vector<string> args = splitTopLevel(line.substr(argStart, close - argStart));

            const std::vector<bool>* flags = nullptr;
            for (const auto& f : entry.second)
                if (f.size() == args.size())
                {
                    flags = &f;
                    break;
                }
            if (!flags)
            {
                pos += token.size();
                continue;
            }

            bool changed = false;
            for (size_t a = 0; a < args.size(); ++a)
            {
                if (!(*flags)[a])
                    continue;
                const string arg = trim(args[a]);
                if (arg.empty() || arg.compare(0, 5, "bool(") == 0)
                    continue;
                // MaterialX booleans are emitted as i32 (`0`/`1`). WGSL will not implicitly
                // convert an integer literal (abstract-int) or i32 variable to `bool`.
                const bool isIntLiteral = (arg == "0" || arg == "1");
                const string at = inferArgType(arg, locals, globals, structs);
                if (!isIntLiteral && at != "i32" && at != "u32")
                    continue;
                args[a] = "bool(" + arg + ")";
                changed = true;
            }
            if (!changed)
            {
                pos = close + 1;
                continue;
            }
            string rebuilt;
            for (size_t a = 0; a < args.size(); ++a)
                rebuilt += (a ? ", " : "") + args[a];
            line.replace(argStart, close - argStart, rebuilt);
            pos = argStart + rebuilt.size() + 1;
        }
    }
    return line;
}

string coerceBoolCallSites(const string& shader)
{
    std::istringstream pre(shader);
    string line;

    std::map<string, std::map<string, string>> structs;
    std::map<string, string> globals;
    std::map<string, std::vector<std::vector<bool>>> fnBool;
    string curStruct;
    int braceDepth = 0;

    while (std::getline(pre, line))
    {
        const string t = trim(line);
        if (!curStruct.empty())
        {
            if (t == "}" || t == "};")
                curStruct.clear();
            else
            {
                const size_t colon = t.find(':');
                if (colon != string::npos)
                {
                    string ty = trim(t.substr(colon + 1));
                    if (!ty.empty() && ty.back() == ',')
                        ty.pop_back();
                    structs[curStruct][trim(t.substr(0, colon))] = ty;
                }
            }
            continue;
        }
        if (t.compare(0, 7, "struct ") == 0 && t.find('}') == string::npos)
        {
            size_t s = 7;
            curStruct = readIdentifier(t, s);
            continue;
        }

        if (auto def = parseFnDef(line))
        {
            braceDepth = 0;
            const std::vector<string> types = paramTypes(def->params);
            std::vector<bool> boolFlags;
            bool anyBool = false;
            for (const string& ty : types)
            {
                const bool b = (ty == "bool");
                boolFlags.push_back(b);
                anyBool = anyBool || b;
            }
            if (anyBool)
                fnBool[def->name].push_back(boolFlags);
        }
        else if (braceDepth == 0)
        {
            collectVarDecl(line, globals);
        }
        for (char c : line)
        {
            if (c == '{')
                braceDepth++;
            else if (c == '}')
                braceDepth--;
        }
    }

    if (fnBool.empty())
        return shader;

    std::istringstream in(shader);
    std::ostringstream out;
    std::map<string, string> locals;
    braceDepth = 0;
    curStruct.clear();
    while (std::getline(in, line))
    {
        const string t = trim(line);
        if (!curStruct.empty())
        {
            out << line << "\n";
            if (t == "}" || t == "};")
                curStruct.clear();
            continue;
        }
        if (t.compare(0, 7, "struct ") == 0 && t.find('}') == string::npos)
        {
            curStruct = "x";
            out << line << "\n";
            continue;
        }

        if (auto def = parseFnDef(line))
        {
            braceDepth = 0;
            locals.clear();
            for (const string& p : splitTopLevel(def->params))
            {
                const size_t colon = p.find(':');
                if (colon != string::npos)
                    locals[trim(p.substr(0, colon))] = trim(p.substr(colon + 1));
            }
        }
        else
        {
            collectVarDecl(line, locals);
            line = coerceBoolArgs(line, fnBool, locals, globals, structs);
        }

        for (char c : line)
        {
            if (c == '{')
                braceDepth++;
            else if (c == '}')
                braceDepth--;
        }
        out << line << "\n";
    }
    return out.str();
}

string resolveOverloads(const string& shader)
{
    std::istringstream pre(shader);
    string line;

    // Pass 1: struct member types, module-scope globals, and overload groups.
    std::map<string, std::map<string, string>> structs;
    std::map<string, string> globals;
    std::map<string, std::vector<std::pair<std::vector<string>, string>>> defs; // name -> [(types,_)]
    std::map<string, std::vector<string>> defRet;                               // name -> [return type per def]
    std::map<string, std::vector<std::vector<bool>>> fnPtr;                     // name -> [per-overload ptr-flags]
    std::map<string, std::vector<std::vector<bool>>> fnBool;                    // name -> [per-overload bool-flags]
    string curStruct;
    int braceDepth = 0;
    while (std::getline(pre, line))
    {
        const string t = trim(line);
        if (!curStruct.empty())
        {
            if (t == "}" || t == "};")
            {
                curStruct.clear();
                continue;
            }
            const size_t colon = t.find(':');
            if (colon != string::npos)
            {
                const string m = trim(t.substr(0, colon));
                string ty = trim(t.substr(colon + 1));
                if (!ty.empty() && ty.back() == ',')
                    ty.pop_back();
                structs[curStruct][m] = ty;
            }
            continue;
        }
        if (t.compare(0, 7, "struct ") == 0 && t.find('}') == string::npos)
        {
            size_t s = 7;
            const string nm = readIdentifier(t, s);
            curStruct = nm;
            continue;
        }
        // Track brace depth to distinguish module-scope globals from locals. A function
        // definition is always at module scope (WGSL has no nested functions), so use it
        // to re-anchor braceDepth to 0 — this keeps definition collection robust even if
        // an earlier line's brace count drifted (which otherwise silently drops later
        // overloads and produces redeclarations).
        if (auto def = parseFnDef(line))
        {
            braceDepth = 0;
            const std::vector<string> types = paramTypes(def->params);
            defs[def->name].push_back({ types, string() });
            std::vector<bool> ptrFlags;
            std::vector<bool> boolFlags;
            bool anyPtr = false;
            bool anyBool = false;
            for (const string& ty : types)
            {
                const bool p = ty.compare(0, 3, "ptr") == 0;
                ptrFlags.push_back(p);
                anyPtr = anyPtr || p;
                const bool b = (ty == "bool");
                boolFlags.push_back(b);
                anyBool = anyBool || b;
            }
            if (anyPtr)
                fnPtr[def->name].push_back(ptrFlags);
            if (anyBool)
                fnBool[def->name].push_back(boolFlags);
            // Capture the return type (keyed below by the function's final name).
            string ret = "void";
            const size_t arrow = line.find("->");
            if (arrow != string::npos)
            {
                const size_t br = line.find('{', arrow);
                ret = trim(string_view(line).substr(arrow + 2, (br == string::npos ? line.size() : br) - (arrow + 2)));
            }
            defRet[def->name].push_back(ret);
        }
        else if (braceDepth == 0)
        {
            collectVarDecl(line, globals);
            // module-scope uniform: `@group(..) ... var<uniform> NAME: TYPE`
            const size_t v = line.find("var<");
            if (v != string::npos)
            {
                const size_t gt = line.find('>', v);
                if (gt != string::npos)
                {
                    string tail = line.substr(gt + 1);
                    collectVarDecl("var " + trim(tail), globals);
                }
            }
        }
        for (char c : line)
        {
            if (c == '{')
                braceDepth++;
            else if (c == '}')
                braceDepth--;
        }
    }

    // Determine overloaded names and assign unique type-suffixed names.
    std::map<string, std::vector<std::pair<std::vector<string>, string>>> overloads;
    for (auto& d : defs)
    {
        if (d.second.size() < 2)
            continue;
        for (auto& variant : d.second)
        {
            string suffix;
            for (const string& ty : variant.first)
                suffix += "_" + typeCode(ty);
            variant.second = d.first + suffix;
            overloads[d.first].push_back(variant);
        }
    }
    // Record each function's final (possibly overload-suffixed) name -> return type
    // in `globals` under a "fn:" prefix, so inferArgType can type call expressions.
    for (const auto& d : defs)
    {
        for (size_t i = 0; i < d.second.size(); ++i)
        {
            string fn = d.first;
            if (d.second.size() >= 2)
                for (const string& ty : d.second[i].first)
                    fn += "_" + typeCode(ty);
            if (i < defRet[d.first].size())
                globals["fn:" + fn] = defRet[d.first][i];
        }
    }

    if (overloads.empty() && fnPtr.empty() && fnBool.empty())
        return shader;

    // Pass 2: rewrite definitions and call sites, tracking per-function locals.
    std::istringstream in(shader);
    std::ostringstream out;
    std::map<string, string> locals;
    braceDepth = 0;
    curStruct.clear();
    while (std::getline(in, line))
    {
        const string t = trim(line);
        // Skip struct bodies unchanged.
        if (!curStruct.empty())
        {
            out << line << "\n";
            if (t == "}" || t == "};")
                curStruct.clear();
            continue;
        }
        if (t.compare(0, 7, "struct ") == 0 && t.find('}') == string::npos)
        {
            curStruct = "x";
            out << line << "\n";
            continue;
        }

        if (auto def = parseFnDef(line))
        {
            braceDepth = 0; // re-anchor at each top-level definition (see pass 1)
            locals.clear();
            // Seed locals with parameter names/types.
            for (const string& p : splitTopLevel(def->params))
            {
                const size_t colon = p.find(':');
                if (colon != string::npos)
                    locals[trim(p.substr(0, colon))] = trim(p.substr(colon + 1));
            }
            // Rename this definition if its name is overloaded.
            auto ov = overloads.find(def->name);
            if (ov != overloads.end())
            {
                const std::vector<string> types = paramTypes(def->params);
                for (const auto& variant : ov->second)
                    if (variant.first == types)
                    {
                        const size_t p = line.find("fn " + def->name + "(");
                        if (p != string::npos)
                            line.replace(p + 3, def->name.size(), variant.second);
                        break;
                    }
            }
        }
        else
        {
            collectVarDecl(line, locals);
            line = addAddressOfToOutArgs(line, fnPtr, locals);
            line = coerceBoolArgs(line, fnBool, locals, globals, structs);
            line = rewriteOverloadCalls(line, overloads, locals, globals, structs);
            line = rewriteMixToSelect(line, locals, globals, structs);
            line = rewriteScalarBroadcast(line, locals, globals, structs);
        }

        for (char c : line)
        {
            if (c == '{')
                braceDepth++;
            else if (c == '}')
                braceDepth--;
        }
        out << line << "\n";
    }
    return out.str();
}

// Rewrite GLSL function definitions emitted inline by HwNumLightsNode / HwLightSamplerNode.
// Per-line rewriteAll() does not convert their signatures; rewriteBlock() on the full stage
// mangles native WGSL (`var<uniform>` bindings). Convert only the known residual stubs.
string rewriteResidualGlslFunctions(const string& shader)
{
    static const char* GLSL_FUNCS[] = {
        "int numActiveLightSources(",
        "void sampleLightSource(",
    };

    string result = shader;
    for (const char* prefix : GLSL_FUNCS)
    {
        const string sig = prefix;
        size_t searchFrom = 0;
        while (true)
        {
            const size_t pos = result.find(sig, searchFrom);
            if (pos == string::npos)
                break;

            const size_t lineStart = (pos == 0 || result[pos - 1] == '\n') ? pos : result.rfind('\n', pos);
            const size_t blockStart = (lineStart == string::npos) ? 0 : (lineStart + (lineStart == pos ? 0 : 1));
            const size_t braceOpen = result.find('{', pos);
            if (braceOpen == string::npos)
            {
                searchFrom = pos + sig.size();
                continue;
            }

            int depth = 0;
            size_t blockEnd = string::npos;
            for (size_t i = braceOpen; i < result.size(); ++i)
            {
                if (result[i] == '{')
                    depth++;
                else if (result[i] == '}')
                {
                    depth--;
                    if (depth == 0)
                    {
                        blockEnd = i + 1;
                        break;
                    }
                }
            }
            if (blockEnd == string::npos)
                break;

            string block = result.substr(blockStart, blockEnd - blockStart);
            // Undo mistaken `&out` on a definition line from an older rewriteSampleLightSource.
            const size_t sigEnd = block.find('{');
            if (sigEnd != string::npos)
            {
                string sigLine = block.substr(0, sigEnd);
                for (size_t p; (p = sigLine.find("&out ")) != string::npos;)
                    sigLine.replace(p, 5, "out ");
                block = sigLine + block.substr(sigEnd);
            }

            const string converted = rewriteBlock(block);
            result.replace(blockStart, blockEnd - blockStart, converted);
            searchFrom = blockStart + converted.size();
        }
    }
    return result;
}

// Repair `else { // comment }` empty blocks left before a real `{` body. This pattern
// breaks brace balance and makes the next `fn` look like it is still inside a function.
string repairEmptyElseCommentBlocks(const string& shader)
{
    const std::vector<string> lines = readLines(shader);
    std::vector<string> out;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        const string t = trim(lines[i]);
        if (startsWith(t, "else {") && t.find("//") != string::npos)
        {
            const size_t close = t.rfind('}');
            const size_t comment = t.find("//");
            if (close != string::npos && comment != string::npos && comment < close)
            {
                size_t j = i + 1;
                while (j < lines.size() && trim(lines[j]).empty())
                    j++;
                if (j < lines.size() && trim(lines[j]) == "{")
                {
                    const size_t indent = lines[i].find_first_not_of(" \t");
                    const string ind = (indent == string::npos) ? "" : lines[i].substr(0, indent);
                    out.push_back(ind + "else " + trim(t.substr(comment, close - comment)));
                    out.push_back(lines[j]);
                    i = j;
                    continue;
                }
            }
        }
        out.push_back(lines[i]);
    }

    return joinLines(out);
}

StringVec findResidualGlsl(const string& wgsl)
{
    // Tokens that should not appear in fully converted WGSL. The trailing space on the
    // sampler/texture entries avoids matching the legitimate WGSL `texture_2d<...>` and
    // `sampler` forms; we only want the GLSL `sampler2D x` / `texture2D x` declarations.
    static constexpr std::array<std::pair<string_view, string_view>, 6> RESIDUAL_MARKERS = { {
        { "#version", "GLSL #version directive" },
        { "#define", "GLSL preprocessor #define" },
        { "layout(", "GLSL layout() qualifier" },
        { "sampler2D ", "GLSL combined-sampler type" },
        { "texture2D ", "GLSL texture type" },
        { "gl_", "GLSL gl_* builtin" },
    } };
    StringVec findings;
    for (const auto& [token, message] : RESIDUAL_MARKERS)
    {
        if (wgsl.find(token) != string::npos)
            findings.push_back(string("residual GLSL token '") + string(token) + "' (" + string(message) + ")");
    }
    return findings;
}

} // namespace GlslToWgsl

MATERIALX_NAMESPACE_END
