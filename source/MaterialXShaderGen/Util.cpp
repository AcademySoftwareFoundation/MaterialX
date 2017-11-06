#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXCore/Util.h>

#include <stack>
#include <sstream>
#include <fstream>
#if defined(_WIN32)
#include <fcntl.h>
#endif

namespace MaterialX
{

bool readFile(const string& filename, string& contents)
{
#if defined(_WIN32)
    // Protection in case someone sets fmode to binary
    int oldMode;
    _get_fmode(&oldMode);
    _set_fmode(_O_TEXT);
#endif

    bool result = false;
    
    std::ifstream file(filename, std::ios::in );
    if (file)
    {
        string buffer;
        file.seekg(0, std::ios::end);
        buffer.resize(size_t(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(&buffer[0], buffer.size());
        file.close();
        if (buffer.length() > 0)
        {
            size_t pos = buffer.find_last_not_of('\0');
            contents = buffer.substr(0, pos + 1);
        }
        result = true;
    }
#if defined(_WIN32)
    _set_fmode(oldMode ? oldMode : _O_TEXT);
#endif

    return result;
}

string getFileExtension(const string& filename)
{
    size_t i = filename.rfind('.');
    return i != string::npos ? filename.substr(i + 1) : EMPTY_STRING;
}


string printGraphDot(const SgNodeGraph& graph)
{
    std::stringstream dot;

    dot << "digraph {\n";

    // Print the nodes
    for (const SgNode* node : graph.getNodeOrder())
    {
        dot << "    \"" << node->getName() << "\" ";
        const string& category = node->getNodePtr()->getCategory();
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
    std::set<const SgNode*> visited;
    for (auto it : graph.getInternalOutputs())
    {
        std::stack<const SgNode*> stack;
        stack.push(it.second->parent);

        while (!stack.empty())
        {
            const SgNode* node = stack.top();
            stack.pop();

            if (visited.count(node) == 0)
            {
                for (const SgInput* input : node->getInputs())
                {
                    if (input->connection)
                    {
                        dot << "    \"" << input->connection->parent->getName() << "\" -> \"" << input->parent->getName() << "\" [label=\"" << input->name << "\"];\n";
                    }
                }
                visited.insert(node);
            }

            for (const SgInput* input : node->getInputs())
            {
                if (input->connection)
                {
                    stack.push(input->connection->parent);
                }
            }
        }
    }

    dot << "}\n";

    return dot.str();
}

} // namespace MaterialX
