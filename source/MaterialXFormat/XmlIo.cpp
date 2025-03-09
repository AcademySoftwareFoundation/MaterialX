//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXFormat/External/PugiXML/pugixml.hpp>

#include <MaterialXCore/Types.h>

#include <cstring>
#include <fstream>
#include <sstream>

using namespace pugi;

MATERIALX_NAMESPACE_BEGIN

const string MTLX_EXTENSION = "mtlx";

const int MAX_XINCLUDE_DEPTH = 256;
const int MAX_XML_TREE_DEPTH = 256;

namespace
{

const string XINCLUDE_TAG = "xi:include";
const string XINCLUDE_NAMESPACE = "xmlns:xi";
const string XINCLUDE_URL = "http://www.w3.org/2001/XInclude";

using ElementStack = vector<std::pair<ElementPtr, xml_node>>;

void documentToXml(DocumentPtr doc, xml_node& xmlRoot, const XmlWriteOptions* writeOptions)
{
    ElementStack elemStack;
    elemStack.emplace_back(doc, xmlRoot);
    while (!elemStack.empty())
    {
        ElementPtr elem = elemStack.back().first;
        xml_node xmlNode = elemStack.back().second;
        elemStack.pop_back();

        bool writeXIncludeEnable = writeOptions ? writeOptions->writeXIncludeEnable : true;
        ElementPredicate elementPredicate = writeOptions ? writeOptions->elementPredicate : nullptr;

        // Store attributes in XML.
        if (!elem->getName().empty())
        {
            xmlNode.append_attribute(Element::NAME_ATTRIBUTE.c_str()) = elem->getName().c_str();
        }
        for (const string& attrName : elem->getAttributeNames())
        {
            xml_attribute xmlAttr = xmlNode.append_attribute(attrName.c_str());
            xmlAttr.set_value(elem->getAttribute(attrName).c_str());
        }

        // Create child elements and recurse.
        StringSet writtenSourceFiles;
        for (auto child : elem->getChildren())
        {
            if (elementPredicate && !elementPredicate(child))
            {
                continue;
            }

            // Write XInclude references if requested.
            if (writeXIncludeEnable && child->hasSourceUri())
            {
                string sourceUri = child->getSourceUri();
                if (sourceUri != elem->getDocument()->getSourceUri())
                {
                    if (!writtenSourceFiles.count(sourceUri))
                    {
                        if (!xmlNode.attribute(XINCLUDE_NAMESPACE.c_str()))
                        {
                            xmlNode.append_attribute(XINCLUDE_NAMESPACE.c_str()) = XINCLUDE_URL.c_str();
                        }
                        xml_node includeNode = xmlNode.append_child(XINCLUDE_TAG.c_str());
                        xml_attribute includeAttr = includeNode.append_attribute("href");
                        FilePath includePath(sourceUri);

                        // Write relative include paths in Posix format, and absolute
                        // include paths in native format.
                        FilePath::Format includeFormat = includePath.isAbsolute() ? FilePath::FormatNative : FilePath::FormatPosix;
                        includeAttr.set_value(includePath.asString(includeFormat).c_str());

                        writtenSourceFiles.insert(sourceUri);
                    }
                    continue;
                }
            }

            // Handle the interpretation of comments and newlines.
            if (child->getCategory() == CommentElement::CATEGORY)
            {
                xml_node xmlChild = xmlNode.append_child(node_comment);
                xmlChild.set_value(child->getAttribute(Element::DOC_ATTRIBUTE).c_str());
                continue;
            }
            if (child->getCategory() == NewlineElement::CATEGORY)
            {
                xml_node xmlChild = xmlNode.append_child(node_newline);
                xmlChild.set_value("\n");
                continue;
            }

            // Push child data onto the stack.
            xml_node xmlChild = xmlNode.append_child(child->getCategory().c_str());
            elemStack.emplace_back(child, xmlChild);
        }
    }
}

void elementFromXml(const xml_node& xmlNode, ElementPtr elem, const XmlReadOptions* readOptions, int depth = 1)
{
    // Store attributes in element.
    for (const xml_attribute& xmlAttr : xmlNode.attributes())
    {
        if (xmlAttr.name() != Element::NAME_ATTRIBUTE)
        {
            elem->setAttribute(xmlAttr.name(), xmlAttr.value());
        }
    }

    // Create child elements and recurse.
    for (const xml_node& xmlChild : xmlNode.children())
    {
        // Get child category.
        string category = xmlChild.name();
        if (category == XINCLUDE_TAG)
        {
            continue;
        }

        // Get child name and skip duplicates.
        string name = xmlChild.attribute(Element::NAME_ATTRIBUTE.c_str()).value();
        ConstElementPtr previous = elem->getChild(name);
        if (previous)
        {
            continue;
        }

        // Enforce maximum tree depth.
        if (depth >= MAX_XML_TREE_DEPTH)
        {
            throw ExceptionParseError("Maximum tree depth exceeded.");
        }

        // Create the child element.
        ElementPtr child = elem->addChildOfCategory(category, name);
        elementFromXml(xmlChild, child, readOptions, depth + 1);

        // Handle the interpretation of XML comments and newlines.
        if (readOptions && category.empty())
        {
            if (readOptions->readComments && xmlChild.type() == node_comment)
            {
                child = elem->changeChildCategory(child, CommentElement::CATEGORY);
                child->setDocString(xmlChild.value());
            }
            else if (readOptions->readNewlines && xmlChild.type() == node_newline)
            {
                child = elem->changeChildCategory(child, NewlineElement::CATEGORY);
            }
        }
    }
}

void documentFromXml(DocumentPtr doc, const xml_document& xmlDoc, const FileSearchPath& searchPath, const XmlReadOptions* readOptions)
{
    xml_node xmlRoot = xmlDoc.child(Document::CATEGORY.c_str());
    if (!xmlRoot)
    {
        throw ExceptionParseError("No root MaterialX element found.");
    }

    // Process XInclude directives.
    XmlReadFunction readXIncludeFunction = readOptions ? readOptions->readXIncludeFunction : readFromXmlFile;
    if (readXIncludeFunction)
    {
        for (const xml_node& xmlChild : xmlRoot.children())
        {
            if (xmlChild.name() == XINCLUDE_TAG)
            {
                string filename = xmlChild.attribute("href").value();
                const StringVec& parents = readOptions ? readOptions->parentXIncludes : StringVec();

                // Validate XInclude state.
                if (std::find(parents.begin(), parents.end(), filename) != parents.end())
                {
                    throw ExceptionParseError("XInclude cycle detected.");
                }
                if (parents.size() >= MAX_XINCLUDE_DEPTH)
                {
                    throw ExceptionParseError("Maximum XInclude depth exceeded.");
                }

                // Read the included file into a library document.
                DocumentPtr library = createDocument();
                XmlReadOptions xiReadOptions = readOptions ? *readOptions : XmlReadOptions();
                xiReadOptions.parentXIncludes.push_back(filename);
                readXIncludeFunction(library, filename, searchPath, &xiReadOptions);

                // Import the library document.
                doc->importLibrary(library);
            }
        }
    }

    // Build the element tree.
    elementFromXml(xmlRoot, doc, readOptions);

    // Upgrade version if requested.
    if (!readOptions || readOptions->upgradeVersion)
    {
        doc->upgradeVersion();
    }
}

void validateParseResult(const xml_parse_result& result, const FilePath& filename = FilePath())
{
    if (result)
    {
        return;
    }

    if (result.status == xml_parse_status::status_file_not_found ||
        result.status == xml_parse_status::status_io_error ||
        result.status == xml_parse_status::status_out_of_memory)
    {
        throw ExceptionFileMissing("Failed to open file for reading: " + filename.asString());
    }

    string desc = result.description();
    string offset = std::to_string(result.offset);
    string message = "XML parse error";
    if (!filename.isEmpty())
    {
        message += " in " + filename.asString();
    }
    message += " (" + desc + " at character " + offset + ")";

    throw ExceptionParseError(message);
}

unsigned int getParseOptions(const XmlReadOptions* readOptions)
{
    unsigned int parseOptions = parse_default;
    if (readOptions)
    {
        if (readOptions->readComments)
        {
            parseOptions |= parse_comments;
        }
        if (readOptions->readNewlines)
        {
            parseOptions |= parse_newlines;
        }
    }
    return parseOptions;
}

} // anonymous namespace

//
// XmlReadOptions methods
//

XmlReadOptions::XmlReadOptions() :
    readComments(false),
    readNewlines(false),
    upgradeVersion(true),
    readXIncludeFunction(readFromXmlFile)
{
}

//
// XmlWriteOptions methods
//

XmlWriteOptions::XmlWriteOptions() :
    writeXIncludeEnable(true)
{
}

//
// Reading
//

void readFromXmlBuffer(DocumentPtr doc, const char* buffer, FileSearchPath searchPath, const XmlReadOptions* readOptions)
{
    searchPath.append(getEnvironmentPath());

    xml_document xmlDoc;
    xml_parse_result result = xmlDoc.load_string(buffer, getParseOptions(readOptions));
    validateParseResult(result);

    documentFromXml(doc, xmlDoc, searchPath, readOptions);
}

void readFromXmlStream(DocumentPtr doc, std::istream& stream, FileSearchPath searchPath, const XmlReadOptions* readOptions)
{
    searchPath.append(getEnvironmentPath());

    xml_document xmlDoc;
    xml_parse_result result = xmlDoc.load(stream, getParseOptions(readOptions));
    validateParseResult(result);

    documentFromXml(doc, xmlDoc, searchPath, readOptions);
}

void readFromXmlFile(DocumentPtr doc, FilePath filename, FileSearchPath searchPath, const XmlReadOptions* readOptions)
{
    searchPath.append(getEnvironmentPath());
    filename = searchPath.find(filename);

    xml_document xmlDoc;
    xml_parse_result result = xmlDoc.load_file(filename.asString().c_str(), getParseOptions(readOptions));
    validateParseResult(result, filename);

    // Store the source URI of the document.
    FilePath sourcePath = (readOptions && !readOptions->parentXIncludes.empty()) ?
                           FilePath(readOptions->parentXIncludes[0]) :
                           FilePath(filename);
    doc->setSourceUri(sourcePath);

    // Enable XIncludes that are relative to this document.
    if (!sourcePath.isAbsolute())
    {
        sourcePath = searchPath.find(sourcePath);
    }
    searchPath.prepend(sourcePath.getParentPath());

    documentFromXml(doc, xmlDoc, searchPath, readOptions);
}

void readFromXmlString(DocumentPtr doc, const string& str, const FileSearchPath& searchPath, const XmlReadOptions* readOptions)
{
    std::istringstream stream(str);
    readFromXmlStream(doc, stream, searchPath, readOptions);
}

//
// Writing
//

void writeToXmlStream(DocumentPtr doc, std::ostream& stream, const XmlWriteOptions* writeOptions)
{
    xml_document xmlDoc;
    xml_node xmlRoot = xmlDoc.append_child("materialx");
    documentToXml(doc, xmlRoot, writeOptions);
    xmlDoc.save(stream, "  ");
}

void writeToXmlFile(DocumentPtr doc, const FilePath& filename, const XmlWriteOptions* writeOptions)
{
    std::ofstream ofs(filename.asString());
    writeToXmlStream(doc, ofs, writeOptions);
}

string writeToXmlString(DocumentPtr doc, const XmlWriteOptions* writeOptions)
{
    std::ostringstream stream;
    writeToXmlStream(doc, stream, writeOptions);
    return stream.str();
}

void prependXInclude(DocumentPtr doc, const FilePath& filename)
{
    if (!filename.isEmpty())
    {
        ElementPtr elem = doc->addNode("xinclude");
        elem->setSourceUri(filename.asString());
        doc->setChildIndex(elem->getName(), 0);
    }
}

MATERIALX_NAMESPACE_END
