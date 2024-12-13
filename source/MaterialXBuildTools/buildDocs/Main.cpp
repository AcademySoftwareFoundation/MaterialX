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

    void removeAcceptedValuesIfEmpty()
    {
        const std::string acceptedValuesHeader = "Accepted Values";
        auto it = std::find(_headings.begin(), _headings.end(), acceptedValuesHeader);
        if (it == _headings.end())
            return;
        int index = std::distance(_headings.begin(), it);
        for (const auto& row : _rows)
        {
            if (!row[index].empty())
                return;
        }
        // all accepted values are empty - so we can remove the column.
        _headings.erase(_headings.begin()+index);
        for (auto& row : _rows)
        {
            row.erase(row.begin()+index);
        }
    }

    friend std::ostream& operator<<(
    std::ostream& os, const MDTable& table);

    static void outputRow(const Row& row, const std::vector<size_t> colSizes, std::ostream& os, char padChar = ' ');
private:
    Row _headings;
    std::vector<Row> _rows;

};

std::ostream& operator<<(
  std::ostream& os, const MDTable& table
) {

    // os << std::endl;
    auto colWidths = table.columnWidths();

    MDTable::outputRow(table._headings, colWidths, os);
    MDTable::outputRow(std::vector<std::string>(colWidths.size(), ""), colWidths,os,  '-');
    for (const auto& row : table._rows)
    {
        MDTable::outputRow(row, colWidths, os);
    }

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

mx::StringVec convertOptionToConsolidatedTypes(const mx::StringVec& options)
{

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


    // create optionsTypeStr from options
    // consolidate color3,color4 -> colorN
    // consolidate matrix33,matrix44 -> matrixNN
    // consolidate vector2,vector3,vector4 -> vectorN

    mx::StringVec parts = options;

    for (auto& part : parts)
    {
        part = mx::trimSpaces(part);

        // also check to see if we have any variable names - if we do then we won't consolidate
        // instead just return the original string.
        if (part.find("@") != std::string::npos)
            return options;
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

    return res;
}



const MDTable::Row headings = {"Port", "Description", "Type", "Default", "Accepted Values"};
const std::string constantValuePrefix = "Constant:";


void processNodeDef(std::unordered_map<std::string, std::string>& nodeTables, mx::NodeDefPtr nodedef, const std::vector<mx::TemplateArgs>& templateArgs = {})
{
    MDTable table(headings);

    // skip anything not in stdlib
    auto sourceUri = nodedef->getActiveSourceUri();
    if (sourceUri.find("/stdlib/") == std::string::npos)
        return;

    std::string firstPortTypeMatching = "";
    std::vector<std::string> firstPortTypeListMatching = {};
    auto ports = nodedef->getChildrenOfType<mx::PortElement>();

    // create portTypeStrVec where each variable to be replaced should only exist once - and all
    // others reference the matching variable - prior to expansion - this means
    // we can track multiple different "Same as" types correctly.
    std::vector<std::string> portTypeStrVec;
    for (unsigned int portIndex = 0; portIndex < ports.size(); portIndex++)
    {
        const auto& port = ports[portIndex];

        std::string portType = port->getType();

        // compare to any previous port types
        bool foundMatch = false;
        for (unsigned int i = 0; i < portIndex; ++i)
        {
            if (portType.find("@") == std::string::npos)
                continue;

            if (portType == portTypeStrVec[i])
            {
                portTypeStrVec.emplace_back("Same as `"+ports[i]->getName()+"`");
                foundMatch = true;
                break;
            }
        }

        if (!foundMatch)
            portTypeStrVec.emplace_back(portType);
    }
    std::vector<std::string> portTypeStrVecSameAsRef = portTypeStrVec;;

    // now we need to string replace those variables that are still left in portTypeStrVec
    for (unsigned int portIndex = 0; portIndex < ports.size(); portIndex++)
    {
        auto& portType = portTypeStrVec[portIndex];
        if (portType.find("@") == std::string::npos)
            continue;

        // strip off surrounding @
        std::string matchString = portType.substr(1, portType.size()-2);

        for (const auto& templateArg : templateArgs)
        {
            auto it = std::find(templateArg.varNameVec.begin(), templateArg.varNameVec.end(), matchString);
            if (it == templateArg.varNameVec.end())
                continue;

            size_t index = it - templateArg.varNameVec.begin();

            auto options = templateArg.optionsVec[index];
            options = convertOptionToConsolidatedTypes(options);

            for (auto& option : options)
            {
                if (option.find("@") != std::string::npos)
                {
                    for (unsigned int i = 0; i < portTypeStrVecSameAsRef.size(); i++)
                    {
                        if (option == portTypeStrVecSameAsRef[i])
                        {
                            option = "Same as `"+ports[i]->getName()+"`";
                            break;
                        }
                    }
                }
            }

            // special case to add "or"
            mx::StringVec newOptions;
            for (unsigned int i = 0; i < options.size(); i++)
            {
                if (i != options.size()-1)
                {
                    if (options[i].find("Same as ") != std::string::npos)
                    {
                        newOptions.emplace_back(options[i] + " or " + options[i+1]);
                        i++;
                        continue;
                    }
                }
                newOptions.emplace_back(options[i]);
            }
            options = newOptions;


            // we may still have variables that need expanding...
            for (auto& option : options)
            {
                if (option.find("@") == std::string::npos)
                    continue;

                // strip off surrounding @
                std::string matchString = option.substr(1, option.size()-2);

                for (const auto& templateArg : templateArgs)
                {
                    auto it = std::find(templateArg.varNameVec.begin(), templateArg.varNameVec.end(), matchString);
                    if (it == templateArg.varNameVec.end())
                        continue;

                    size_t index = it - templateArg.varNameVec.begin();

                    newOptions = templateArg.optionsVec[index];

                    if (newOptions.size() > 1)
                    {
                        // unfold list of types again.
                        mx::StringVec newerOptions;
                        for (const auto& newOption : newOptions)
                        {
                            auto parts = mx::splitString(newOption, ",");
                            for (auto&part : parts)
                            {
                                part = mx::trimSpaces(part);
                                if (std::find(newerOptions.begin(), newerOptions.end(), part) == newerOptions.end())
                                {
                                    newerOptions.emplace_back(part);
                                }
                            }
                        }
                        newOptions = newerOptions;
                    }


                    newOptions = convertOptionToConsolidatedTypes(newOptions);
                }
            }
            options = newOptions;

            portType = mx::joinStrings(options, ", ");

            break;
        }

    }

    // for (const auto& input : inputs)
    for (unsigned int portIndex = 0; portIndex < ports.size(); portIndex++)
    {
        const auto& port = ports[portIndex];
        const auto& input = port->asA<mx::Input>();
        if (!input)
            continue;

        std::string portName = input->getName();
        std::string portDesc = input->getAttribute("spec_desc");
        std::string portDefault = input->getValueString();
        std::string portType = portTypeStrVec[portIndex];
        std::string portAcceptedValues = input->getAttribute("spec_acceptedvalues");


        if (portDefault.find(constantValuePrefix) != std::string::npos)
        {
            portDefault = mx::replaceSubstrings(portDefault, {{constantValuePrefix,""}});
            portDefault = "__"+portDefault+"__";
        }
        else if (portDefault.empty())
        {
            if (input->hasDefaultGeomPropString())
            {
                portDefault = "_"+input->getDefaultGeomPropString()+"_";
            }
            else
            {
                portDefault = "__empty__";
            }
        }

        if (portAcceptedValues.empty())
        {
            std::string enumNamesStr;
            if (input->hasAttribute(mx::ValueElement::ENUM_VALUES_ATTRIBUTE))
            {
                enumNamesStr = input->getAttribute(mx::ValueElement::ENUM_VALUES_ATTRIBUTE);
            }
            else
            {
                enumNamesStr = input->getAttribute(mx::ValueElement::ENUM_ATTRIBUTE);
            }

            auto enumNames = mx::splitString(enumNamesStr, ",");
            std::string sep = "";
            for (const auto& enumName : enumNames)
            {
                portAcceptedValues += sep + enumName;
                sep = ", ";
            }
        }

        portName = "`"+portName+"`";

        auto row = MDTable::Row();

        row.emplace_back(portName);
        row.emplace_back(portDesc);
        row.emplace_back(portType);
        row.emplace_back(portDefault);
        row.emplace_back(portAcceptedValues);

        table.addRow(row);
    }

    for (unsigned int portIndex = 0; portIndex < ports.size(); portIndex++)
    {
        const auto& port = ports[portIndex];
        const auto& output = port->asA<mx::Output>();
        if (!output)
            continue;

        std::string portName = output->getName();
        std::string portDesc = output->getAttribute("spec_desc");
        std::string portType = portTypeStrVec[portIndex];
        std::string portDefault = output->getAttribute("default");
        std::string portAcceptedValues = output->getAttribute("spec_acceptedvalues");

        if (output->hasAttribute("defaultinput"))
        {
            portDefault = "`"+output->getAttribute("defaultinput")+"`";
        }
        else if (portDefault.find(constantValuePrefix) != std::string::npos)
        {
            portDefault = mx::replaceSubstrings(portDefault, {{constantValuePrefix,""}});
            portDefault = "__"+portDefault+"__";
        }
        else if (portDefault.empty())
        {
            if (output->hasAttribute("default"))
            {
                portDefault = "__empty__";
            }
            else
            {
                portDefault = "";
            }
        }

        if (portDesc.empty())
            portDesc = "Output";
        else
            portDesc = "Output: " + portDesc;

        portName = "`"+portName+"`";

        auto row = MDTable::Row();

        row.emplace_back(portName);
        row.emplace_back(portDesc);
        row.emplace_back(portType);
        row.emplace_back(portDefault);
        row.emplace_back(portAcceptedValues);

        table.addRow(row);
    }

    table.removeAcceptedValuesIfEmpty();

    std::stringstream sss;
    sss << table;
    std::string tableStr = sss.str();
    tableStr.pop_back();

    const std::string key = "@MX_TABLE_"+nodedef->getNodeString()+"@";

    if (!nodeTables[key].empty())
    {
        tableStr = "\n\n" + tableStr;
    }

    nodeTables[key] += tableStr;
};


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

    std::unordered_map<std::string, std::string> nodeTables;


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
                const auto nodedef = child->asA<mx::NodeDef>();
                processNodeDef(nodeTables, nodedef);
            }
            else if (childCategory == "template")
            {
                if (!child->hasAttribute("varnames") || !child->hasAttribute("options"))
                    throw std::runtime_error("<template> must have both varnames and options attributes");

                const auto varNames = child->getAttribute("varnames");
                const auto options = child->getAttribute("options");

                const auto templateArgs = mx::parseTemplateArgs(varNames, options);

                for (const auto& elem : child->getChildrenOfType<mx::Element>())
                {
                    if (elem->getCategory() == "nodedef")
                    {
                        auto nodedef = elem->asA<mx::NodeDef>();
                        processNodeDef(nodeTables, nodedef, {templateArgs});
                    }
                    else if (elem->getCategory() == "template")
                    {
                        const auto varNames2 = elem->getAttribute("varnames");
                        const auto options2 = elem->getAttribute("options");

                        const auto templateArgs2 = mx::parseTemplateArgs(varNames2, options2);

                        for (const auto& elem2 : elem->getChildrenOfType<mx::Element>())
                        {
                            if (elem2->getCategory() == "nodedef")
                            {
                                auto nodedef = elem2->asA<mx::NodeDef>();
                                processNodeDef(nodeTables, nodedef, {templateArgs,templateArgs2});
                            }
                        }
                    }
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
        auto destBaseName = mx::replaceSubstrings(mdFile.getBaseName(), {{".template.md", ".md"}});

        mx::FilePath sourceFile = sourceMDRoot / mdFile.getParentPath() / mdFile.getBaseName();
        mx::FilePath destFile = destMDRoot / mdFile.getParentPath() / destBaseName;

        auto contents = readTextFile(sourceFile);
        auto newContents = mx::replaceSubstrings(contents, nodeTables);

        writeFile(destFile, newContents);
    }

    return 0;
}
