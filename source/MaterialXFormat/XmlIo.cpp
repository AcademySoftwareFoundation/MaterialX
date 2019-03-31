//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXFormat/File.h>

#include <MaterialXFormat/PugiXML/pugixml.hpp>

#include <MaterialXCore/Types.h>
#include <MaterialXCore/Util.h>

#include <fstream>
#include <sstream>
#include <string.h>

using namespace pugi;

namespace MaterialX
{

const string MTLX_EXTENSION = "mtlx";

namespace {

const string SOURCE_URI_ATTRIBUTE = "__sourceUri";
const string XINCLUDE_TAG = "xi:include";

void elementFromXml(const xml_node& xmlNode, ElementPtr elem, const XmlReadOptions* readOptions)
{
    bool skipDuplicateElements = readOptions && readOptions->skipDuplicateElements;

    // Store attributes in element.
    for (const xml_attribute& xmlAttr : xmlNode.attributes())
    {
        if (xmlAttr.name() == SOURCE_URI_ATTRIBUTE)
        {
            elem->setSourceUri(xmlAttr.value());
        }
        else if (xmlAttr.name() != Element::NAME_ATTRIBUTE)
        {
            elem->setAttribute(xmlAttr.name(), xmlAttr.value());
        }
    }

    // Create child elements and recurse.
    for (const xml_node& xmlChild : xmlNode.children())
    {
        string category = xmlChild.name();
        string name;
        for (const xml_attribute& xmlAttr : xmlChild.attributes())
        {
            if (xmlAttr.name() == Element::NAME_ATTRIBUTE)
            {
                name = xmlAttr.value();
                break;
            }
        }

        // If requested, skip elements with duplicate names.
        if (skipDuplicateElements && elem->getChild(name))
        {
            continue;
        }

        ElementPtr child = elem->addChildOfCategory(category, name);
        elementFromXml(xmlChild, child, readOptions);
    }
}

void elementToXml(ConstElementPtr elem, xml_node& xmlNode, const XmlWriteOptions* writeOptions)
{
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

    // Create child nodes and recurse.
    StringSet writtenSourceFiles;
    for (ElementPtr child : elem->getChildren())
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
                    xml_node includeNode = xmlNode.append_child(XINCLUDE_TAG.c_str());
                    xml_attribute includeAttr = includeNode.append_attribute("href");
                    includeAttr.set_value(sourceUri.c_str());
                    writtenSourceFiles.insert(sourceUri);
                }
                continue;
            }
        }

        xml_node xmlChild = xmlNode.append_child(child->getCategory().c_str());
        elementToXml(child, xmlChild, writeOptions);
    }
}

void xmlDocumentFromFile(xml_document& xmlDoc, string filename, const string& searchPath)
{
    FileSearchPath fileSearchPath = FileSearchPath(searchPath);
    fileSearchPath.append(getEnvironmentPath());

    filename = fileSearchPath.find(filename);

    xml_parse_result result = xmlDoc.load_file(filename.c_str());
    if (!result)
    {
        if (result.status == xml_parse_status::status_file_not_found ||
            result.status == xml_parse_status::status_io_error ||
            result.status == xml_parse_status::status_out_of_memory)
        {
            throw ExceptionFileMissing("Failed to open file for reading: " + filename);
        }
        else
        {
            string desc = result.description();
            string offset = std::to_string(result.offset);
            throw ExceptionParseError("XML parse error in file: " + filename +
                                      " (" + desc + " at character " + offset + ")");
        }
    }
}

void processXIncludes(DocumentPtr doc, xml_node& xmlNode, const string& searchPath, const XmlReadOptions* readOptions)
{
    XmlReadFunction readXIncludeFunction = readOptions ? readOptions->readXIncludeFunction : readFromXmlFile;
    xml_node xmlChild = xmlNode.first_child();
    while (xmlChild)
    {
        if (xmlChild.name() == XINCLUDE_TAG)
        {
            // Read XInclude references if requested.
            if (readXIncludeFunction)
            {
                string filename = xmlChild.attribute("href").value();

                // Check for XInclude cycles.
                if (readOptions && readOptions->parentFilenames.count(filename))
                {
                    throw ExceptionParseError("XInclude cycle detected.");
                }

                // Read the included file into a library document.
                DocumentPtr library = createDocument();
                XmlReadOptions xiReadOptions = readOptions ? *readOptions : XmlReadOptions();
                xiReadOptions.parentFilenames.insert(filename);
                readXIncludeFunction(library, filename, searchPath, &xiReadOptions);

                // Import the library document.
                doc->importLibrary(library, readOptions);
            }

            // Remove include directive.
            xml_node includeNode = xmlChild;
            xmlChild = xmlChild.next_sibling();
            xmlNode.remove_child(includeNode);
        }
        else
        {
            xmlChild = xmlChild.next_sibling();
        }
    }
}

void documentFromXml(DocumentPtr doc,
                     const xml_document& xmlDoc,
                     const string& searchPath = EMPTY_STRING,
                     const XmlReadOptions* readOptions = nullptr)
{
    ScopedUpdate update(doc);
    doc->onRead();

    xml_node xmlRoot = xmlDoc.child(Document::CATEGORY.c_str());
    if (xmlRoot)
    {
        processXIncludes(doc, xmlRoot, searchPath, readOptions);
        elementFromXml(xmlRoot, doc, readOptions);
    }

    doc->upgradeVersion();
}

} // anonymous namespace

//
// XmlReadOptions methods
//

XmlReadOptions::XmlReadOptions() :
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

void readFromXmlBuffer(DocumentPtr doc, const char* buffer, const XmlReadOptions* readOptions)
{
    xml_document xmlDoc;
    xml_parse_result result = xmlDoc.load_string(buffer);
    if (!result)
    {
        throw ExceptionParseError("Parse error in readFromXmlBuffer");
    }

    documentFromXml(doc, xmlDoc, EMPTY_STRING, readOptions);
}

void readFromXmlStream(DocumentPtr doc, std::istream& stream, const XmlReadOptions* readOptions)
{
    xml_document xmlDoc;
    xml_parse_result result = xmlDoc.load(stream);
    if (!result)
    {
        throw ExceptionParseError("Parse error in readFromXmlStream");
    }

    documentFromXml(doc, xmlDoc, EMPTY_STRING, readOptions);
}

void readFromXmlFile(DocumentPtr doc, const string& filename, const string& searchPath, const XmlReadOptions* readOptions)
{
    xml_document xmlDoc;
    xmlDocumentFromFile(xmlDoc, filename, searchPath);

    documentFromXml(doc, xmlDoc, searchPath, readOptions);
    doc->setSourceUri(filename);
}

void readFromXmlString(DocumentPtr doc, const string& str, const XmlReadOptions* readOptions)
{
    std::istringstream stream(str);
    readFromXmlStream(doc, stream, readOptions);
}

//
// Writing
//

void writeToXmlStream(DocumentPtr doc, std::ostream& stream, const XmlWriteOptions* writeOptions)
{
    ScopedUpdate update(doc);
    doc->onWrite();

    xml_document xmlDoc;
    xml_node xmlRoot = xmlDoc.append_child("materialx");
    elementToXml(doc, xmlRoot, writeOptions);
    xmlDoc.save(stream, "  ");
}

void writeToXmlFile(DocumentPtr doc, const string& filename, const XmlWriteOptions* writeOptions)
{
    std::ofstream ofs(filename);
    writeToXmlStream(doc, ofs, writeOptions);
}

string writeToXmlString(DocumentPtr doc, const XmlWriteOptions* writeOptions)
{
    std::ostringstream stream;
    writeToXmlStream(doc, stream, writeOptions);
    return stream.str();
}

void prependXInclude(DocumentPtr doc, const string& filename)
{
    ElementPtr elem = doc->addChildOfCategory("xinclude");
    elem->setSourceUri(filename);
    doc->setChildIndex(elem->getName(), 0);
}

} // namespace MaterialX
