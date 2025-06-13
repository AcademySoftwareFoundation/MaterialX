//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Exception.h>
#include <MaterialXCore/Util.h>

namespace mx = MaterialX;

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

const std::string cmdLineOptions =
    " Options: \n"
    "    --sourceLibraryRoot [FILEPATH]     Directory containing the source data library files.\n"
    "    --destDocRoot [FILEPATH]           Directory to write the modified data library files.\n"
    "    --sourceMDRoot [FILEPATH]           Directory to write the modified data library files.\n"
    "    --manifestFile [FILEPATH]          Directory to write the modified data library files.\n"
    "    --help                             Display the complete list of command-line options\n";

template <class T> void parseToken(std::string token, std::string type, T& res)
{
    if (token.empty())
    {
        return;
    }

    mx::ValuePtr value = mx::Value::createValueFromStrings(token, type);
    if (!value)
    {
        std::cout << "Unable to parse token " << token << " as type " << type << std::endl;
        return;
    }

    res = value->asA<T>();
}

mx::FilePathVec getMaterialXFiles(const mx::FilePath& libraryRoot)
{
    mx::FilePathVec result;

    auto subDirs = libraryRoot.getSubDirectories();
    for (const auto& subDir : subDirs)
    {
        for (const auto& file : subDir.getFilesInDirectory("mtlx"))
        {
            auto absFile = subDir / file;
            std::string absFileStr = absFile.asString();
            std::string relFileStr = absFileStr.substr(libraryRoot.asString().size()+1);
            result.emplace_back(mx::FilePath(relFileStr));
        }
    }
    return result;
}


mx::FilePathVec getMarkdownFiles(const mx::FilePath& libraryRoot)
{
    mx::FilePathVec result;

    auto subDirs = libraryRoot.getSubDirectories();
    for (const auto& subDir : subDirs)
    {
        for (const auto& file : subDir.getFilesInDirectory("md"))
        {
            auto absFile = subDir / file;
            std::string absFileStr = absFile.asString();
            std::string relFileStr = absFileStr.substr(libraryRoot.asString().size()+1);
            result.emplace_back(mx::FilePath(relFileStr));
        }
    }
    return result;
}


void makeDir(const mx::FilePath& dir, bool recursive = true)
{
    if (recursive && !dir.getParentPath().isDirectory())
    {
        makeDir(dir.getParentPath(), recursive);
    }
    dir.createDirectory();
}

bool writeFile(const mx::FilePath& file, const std::string fileContents, bool createDirectories = true)
{
    if (createDirectories && !file.getParentPath().isDirectory())
    {
        makeDir(file.getParentPath(), true);
    }

    std::string filename = file.asString();
    std::ofstream file_out(filename);
    if (!file_out.is_open()) {
        std::cerr << "Error opening file for writing - " << filename << std::endl;
        return false;
    }

    file_out << fileContents;
    file_out.close();
    return true;
}

std::string readTextFile(const mx::FilePath& file)
{
    std::string filename = file.asString();

    std::ifstream file_in(filename);
    if (!file_in.is_open()) {
        std::cerr << "Error opening file for reading - " << filename << std::endl;
        return "";
    }

    std::string contents;
    std::string line;
    while (std::getline(file_in, line)) {
        contents += line +"\n";
    }
    file_in.close();

    return contents;
}

class MDTable
{
public:

    using Row = std::vector<std::string>;

    MDTable(Row headings) : _headings(headings) {}

    void addRow(const Row& row)
    {
        if (row.size() != _headings.size())
            throw std::runtime_error("Incorrect row size)");
        _rows.emplace_back(row);
    }

    std::vector<size_t> columnWidths() const
    {
        std::vector<size_t> widths;
        for (const auto& heading : _headings)
        {
            widths.emplace_back(heading.size());
        }
        for (const auto& row : _rows)
        {
            for (unsigned int i = 0, n = row.size(); i < n; ++i)
            {
                widths[i] = std::max(widths[i], row[i].size());
            }
        }
        return widths;
    }

    friend std::ostream& operator<<(
    std::ostream& os, const MDTable& table);

    static void outputRow(const Row& row, const std::vector<size_t> colSizes, std::ostream& os, char padChar = ' ');
private:
    const Row _headings;
    std::vector<Row> _rows;

};

std::ostream& operator<<(
  std::ostream& os, const MDTable& table
) {

    os << std::endl;
    auto colWidths = table.columnWidths();

    MDTable::outputRow(table._headings, colWidths, os);
    MDTable::outputRow(std::vector<std::string>(colWidths.size(), ""), colWidths,os,  '-');
    for (const auto& row : table._rows)
    {
        MDTable::outputRow(row, colWidths, os);
    }

    os << std::endl;

    return os;
}

void MDTable::outputRow(const Row& row, const std::vector<size_t> colSizes, std::ostream& os, char padChar)
{

    os << "|";
    for (unsigned int i = 0, n = row.size(); i < n; ++i)
    {
        const auto& entry = row[i];
        auto numSpaces = colSizes[i] -  entry.size();
        os << entry << std::string(numSpaces, padChar) << "|";
    }
    os << std::endl;
}

int main(int argc, char* const argv[])
{
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    std::string sourceLibraryRootStr = "";
    std::string destDocRootStr = "";
    std::string sourceMDRootStr = "";
    std::string destMDRootStr = "";
    std::string manifestFileStr = "";

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;

        if (token == "--sourceLibraryRoot")
        {
            sourceLibraryRootStr = nextToken;
        }
        else if (token == "--destDocRoot")
        {
            destDocRootStr = nextToken;
        }
        else if (token == "--sourceMDRoot")
        {
            sourceMDRootStr = nextToken;
        }
        else if (token == "--destMDRoot")
        {
            destMDRootStr = nextToken;
        }
        else if (token == "--manifestFile")
        {
            manifestFileStr = nextToken;
        }
        else if (token == "--help")
        {
            std::cout << " MaterialXBuildLibrary version " << mx::getVersionString() << std::endl;
            std::cout << cmdLineOptions << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Unrecognized command-line option: " << token << std::endl;
            std::cout << "Launch the viewer with '--help' for a complete list of supported options." << std::endl;
            continue;
        }

        if (nextToken.empty())
        {
            std::cout << "Expected another token following command-line option: " << token << std::endl;
        }
        else
        {
            i++;
        }
    }

    mx::FilePath sourceLibraryRoot = mx::FilePath(sourceLibraryRootStr);
    mx::FilePath destDocRoot = mx::FilePath(destDocRootStr);
    mx::FilePath sourceMDRoot = mx::FilePath(sourceMDRootStr);
    mx::FilePath destMDRoot = mx::FilePath(destMDRootStr);

    if (!sourceLibraryRoot.isDirectory())
    {
        std::cerr << "Source Library Root is not a directory" << std::endl;
        return 1;
    }

    if (!destDocRoot.isDirectory())
    {
        std::cerr << "Destination Library Root is not a directory" << std::endl;
        return 1;
    }

    if (!sourceMDRoot.isDirectory())
    {
        std::cerr << "Destination MD Root is not a directory" << std::endl;
        return 1;
    }

    if (!destMDRoot.isDirectory())
    {
        std::cerr << "Destination MD Root is not a directory" << std::endl;
        return 1;
    }

    mx::FilePathVec mtlxFiles = getMaterialXFiles(sourceLibraryRoot);
    mx::FilePathVec mdFiles = getMarkdownFiles(sourceMDRoot);

    mx::XmlReadOptions readOptions;
    readOptions.readComments = true;
    readOptions.readNewlines = true;
    readOptions.expandTemplateElems = false;

    mx::XmlWriteOptions writeOptions;
    writeOptions.createDirectories = true;

    const MDTable::Row headings = {"Port", "Description", "Type", "Default", "Accepted Values"};
    std::unordered_map<std::string, std::string> nodeTables;

    auto processNodeDef = [&](const mx::NodeDefPtr nodedef, std::string matchString="#NOT_MATCHING#", std::string optionsTypeStr = "")
    {
        MDTable table(headings);

        std::string firstPortMatchingType = "";
        auto ports = nodedef->getChildrenOfType<mx::PortElement>();

        for (const auto& port : ports)
        {
            std::string portName = port->getName();
            std::string portDesc = port->getAttribute("spec_desc");
            std::string portType = port->getType();
            std::string portDefault = port->getValueString();
            std::string portAcceptedValues = port->getAttribute("spec_acceptedvalues");

            portDefault = mx::replaceSubstrings(portDefault, {{"Value:",""}});

            if (portType == matchString)
            {
                if (firstPortMatchingType.empty())
                {
                    firstPortMatchingType = portName;
                    portType = optionsTypeStr;
                }
                else
                {
                    portType = "Same as `"+firstPortMatchingType+"`";
                }
            }

            if (auto output = port->asA<mx::Output>())
            {
                if (portDesc.empty())
                    portDesc = "Output";
                else
                    portDesc = "Output: " + portDesc;
            }


            auto row = MDTable::Row();

            row.emplace_back(portName);
            row.emplace_back(portDesc);
            row.emplace_back(portType);
            row.emplace_back(portDefault);
            row.emplace_back(portAcceptedValues);

            table.addRow(row);
        }


        std::stringstream sss;
        sss << table;

        const std::string key = "@MX_TABLE_"+nodedef->getNodeString()+"@";

        nodeTables[key] += sss.str();
    };


    for (const auto& mtlxFile : mtlxFiles)
    {
        mx::FilePath sourceFile = sourceLibraryRoot / mtlxFile;
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, sourceFile, mx::FileSearchPath(), &readOptions);

        for (const auto& child : doc->getChildren())
        {
            const auto& childCategory = child->getCategory();

            if (childCategory == "nodedef")
            {
                auto nodedef = child->asA<mx::NodeDef>();
                processNodeDef(nodedef);
            }
            else if (childCategory == "template")
            {
                if (!child->hasAttribute("varName") || !child->hasAttribute("options"))
                    throw std::runtime_error("<template> must have both varName and options attributes");

                const auto varName = child->getAttribute("varName");
                const auto options = child->getAttribute("options");

                std::string optionsTypeStr = options;

                static const std::vector<std::string> preferredOptionOrder = {
                    "float",
                    "colorN", "color3", "color4",
                    "vectorN", "vector2", "vector3", "vector4",
                    "matrixNN", "matrix33", "matrix44",
                    "boolean",
                    "integer",
                    "string", "filename",
                    "surfaceshader", "displacementshader", "volumeshader", "lightshader"
                };

                auto contains = [&](const std::vector<std::string>& array, const std::string& str) -> bool {
                    return std::find(array.begin(), array.end(), str) != array.end();
                };
                auto remove = [&](std::vector<std::string>& array, const std::string& str) -> void {
                    array.erase(std::remove(array.begin(), array.end(), str));
                };

                {
                    // create optionsTypeStr from options
                    // consolidate color3,color4 -> colorN
                    // consolidate matrix33,matrix44 -> matrixNN
                    // consolidate vector2,vector3,vector4 -> vectorN

                    auto parts = mx::splitString(options,",");
                    for (auto& part : parts)
                    {
                        part = mx::trimSpaces(part);
                    }

                    if (contains(parts, "vector2") && contains(parts, "vector3") && contains(parts, "vector4"))
                    {
                        remove(parts, "vector2");
                        remove(parts, "vector3");
                        remove(parts, "vector4");
                        parts.emplace_back("vectorN");
                    }
                    if (contains(parts, "color3") && contains(parts, "color4"))
                    {
                        remove(parts, "color3");
                        remove(parts, "color4");
                        parts.emplace_back("colorN");
                    }
                    if (contains(parts, "matrix33") && contains(parts, "matrix44"))
                    {
                        remove(parts, "matrix33");
                        remove(parts, "matrix44");
                        parts.emplace_back("matrixNN");
                    }

                    std::vector<std::string> res;
                    for (const auto& part : preferredOptionOrder)
                    {
                        if (contains(parts, part))
                        {
                            remove(parts, part);
                            res.emplace_back(part);
                        }
                    }

                    if (parts.size())
                    {
                        std::stringstream ss;
                        for (const auto& part : parts)
                        {
                            ss << "\""<<part<<"\", ";
                        }
                        throw std::runtime_error("unhandled types in preferred list: "+ss.str());
                    }

                    optionsTypeStr = mx::joinStrings(res, ", ");
                }

                const auto matchString = "@"+varName+"@";

                auto childNodeDefs = child->getChildrenOfType<mx::NodeDef>();
                for (const auto& nodedef : childNodeDefs)
                {
                    processNodeDef(nodedef, matchString, optionsTypeStr);
                }
            }
        }
    }

    std::string manifestStr = "";
    for (const auto& it : nodeTables)
    {
        mx::FilePath destFile = destDocRoot / (it.first + std::string("_doc.md"));
        writeFile(destFile, it.second);
        manifestStr += destFile.asString() + "\n";
    }
    writeFile(mx::FilePath(manifestFileStr), manifestStr);

    for (const auto& mdFile : mdFiles)
    {
        mx::FilePath sourceFile = sourceMDRoot / mdFile;
        mx::FilePath destFile = destMDRoot / mdFile;

        auto contents = readTextFile(sourceFile);
        auto newContents = mx::replaceSubstrings(contents, nodeTables);

        writeFile(destFile, newContents);
    }

    return 0;
}
