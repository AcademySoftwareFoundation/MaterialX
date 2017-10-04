//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Util.h>

#include <MaterialXCore/Element.h>
#include <MaterialXCore/Node.h>

#include <sstream>

namespace MaterialX
{

const int MAJOR_VERSION = MATERIALX_MAJOR_VERSION;
const int MINOR_VERSION = MATERIALX_MINOR_VERSION;
const int BUILD_VERSION = MATERIALX_BUILD_VERSION;

const string LIBRARY_VERSION_STRING = std::to_string(MAJOR_VERSION) + "." +
                                      std::to_string(MINOR_VERSION) + "." +
                                      std::to_string(BUILD_VERSION);
const string EMPTY_STRING;

//
// Utility methods
//

string getVersionString()
{
    return LIBRARY_VERSION_STRING;
}

std::tuple<int, int, int> getVersionIntegers()
{
    return std::make_tuple(MATERIALX_MAJOR_VERSION,
                           MATERIALX_MINOR_VERSION,
                           MATERIALX_BUILD_VERSION);
}

string createValidName(string name, char replaceChar)
{
    auto replacePred = [](char c) { return !isalnum(c) && c != '_'; };
    std::replace_if(name.begin(), name.end(), replacePred, replaceChar);
    return name;
}

bool isValidName(const string& name)
{
    return name == createValidName(name);
}

string incrementName(const string& name)
{
    size_t split = name.length();
    while (split > 0)
    {
        if (!isdigit(name[split - 1]))
            break;
        split--;
    }

    if (split < name.length())
    {
        string prefix = name.substr(0, split);
        string suffix = name.substr(split, name.length());
        return prefix + std::to_string(std::stoi(suffix) + 1);
    }
    return name + "2";
}

vector<string> splitString(const string& str, const string& sep)
{
    vector<string> split;

    string::size_type lastPos = str.find_first_not_of(sep, 0);
    string::size_type pos = str.find_first_of(sep, lastPos);

    while (pos != string::npos || lastPos != string::npos)
    {
        split.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(sep, pos);
        pos = str.find_first_of(sep, lastPos);
    }

    return split;
}

string replaceSubstrings(string str, const StringMap& stringMap)
{
    for (auto& pair : stringMap)
    {
        if (pair.first.empty())
            continue;

        size_t pos = 0;
        while ((pos = str.find(pair.first, pos)) != std::string::npos)
        {
             str.replace(pos, pair.first.length(), pair.second);
             pos += pair.second.length();
        }
    }
    return str;
}

string prettyPrint(ElementPtr elem)
{
    string text;
    for (TreeIterator it = elem->traverseTree().begin(); it != TreeIterator::end(); ++it)
    {
        string indent(it.getElementDepth() * 2, ' ');
        text += indent + it.getElement()->asString() + "\n";
    }
    return text;
}

string printGraphDot(NodeGraphPtr graph)
{
    std::stringstream dot;

    dot << "digraph {\n";

    // Print the nodes
    for (auto node : graph->getChildrenOfType<Node>())
    {
        dot << "    \"" << node->getName() << "\" ";
        const string& category = node->getCategory();
        if (category == "compare" || category == "switch")
        {
            dot << "[shape=diamond];\n";
        }
        else
        {
            dot << "[shape=box];\n";
        }
    }
 
    // Print the connections
    std::set<Edge> processedEdges;
    for (auto output : graph->getChildrenOfType<Output>())
    {
        for (Edge edge : output->traverseGraph())
        {
            if (processedEdges.count(edge) == 0)
            {
                processedEdges.insert(edge);
                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();
                dot << "    \"" << upstreamElem->getName() << "\" -> \"" << downstreamElem->getName() << "\" [label=\"" << (connectingElem ? connectingElem->getName() : EMPTY_STRING) << "\"];\n";
            }
        }
    }

    dot << "}\n";

    return dot.str();
}

} // namespace MaterialX
