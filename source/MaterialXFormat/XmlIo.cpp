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

namespace {

const string SOURCE_URI_ATTRIBUTE = "__sourceUri";
const string XINCLUDE_TAG = "xi:include";

void elementFromXml(const xml_node& xmlNode, ElementPtr elem, const XmlReadOptions* readOptions)
{
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
        if (readOptions && readOptions->skipDuplicateElements && elem->getChild(name))
        {
            continue;
        }

        ElementPtr child = elem->addChildOfCategory(category, name);
        elementFromXml(xmlChild, child, readOptions);
    }
}

void elementToXml(ConstElementPtr elem, xml_node& xmlNode, bool writeXIncludes, const ElementPredicate& predicate)
{
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
        if (predicate && !predicate(child))
        {
            continue;
        }
        if (writeXIncludes && child->hasSourceUri())
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
        elementToXml(child, xmlChild, writeXIncludes, predicate);
    }
}

void xmlDocumentFromFile(xml_document& xmlDoc, string filename, const string& searchPath)
{
    if (!searchPath.empty())
    {
        filename = FileSearchPath(searchPath).find(filename);
    }

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
            throw ExceptionParseError("XML parse error in file: " + filename + " (" + result.description() + ")");
        }
    }
}

void processXIncludes(DocumentPtr doc, xml_node& xmlNode, const string& searchPath, const XmlReadOptions* readOptions)
{
    xml_node xmlChild = xmlNode.first_child();
    while (xmlChild)
    {
        if (xmlChild.name() == XINCLUDE_TAG)
        {
            if (!readOptions || readOptions->readXIncludes)
            {
                // Read the included file into a library document.
                xml_attribute fileAttr = xmlChild.attribute("href");
                DocumentPtr library = createDocument();
                readFromXmlFile(library, fileAttr.value(), searchPath, readOptions);

                // Import the library.
                CopyOptions copyOptions = readOptions ? (CopyOptions) *readOptions : CopyOptions();
                copyOptions.copySourceUris = true;
                doc->importLibrary(library, &copyOptions);
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

void writeToXmlStream(DocumentPtr doc, std::ostream& stream, bool writeXIncludes, const ElementPredicate& predicate)
{
    ScopedUpdate update(doc);
    doc->onWrite();

    xml_document xmlDoc;
    xml_node xmlRoot = xmlDoc.append_child("materialx");
    elementToXml(doc, xmlRoot, writeXIncludes, predicate);
    xmlDoc.save(stream, "  ");
}

void writeToXmlFile(DocumentPtr doc, const string& filename, bool writeXIncludes, const ElementPredicate& predicate)
{
    std::ofstream ofs(filename);
    writeToXmlStream(doc, ofs, writeXIncludes, predicate);
}

string writeToXmlString(DocumentPtr doc, bool writeXIncludes, const ElementPredicate& predicate)
{
    std::ostringstream stream;
    writeToXmlStream(doc, stream, writeXIncludes, predicate);
    return stream.str();
}

void prependXInclude(DocumentPtr doc, const string& filename)
{
    ElementPtr elem = doc->addChildOfCategory("xinclude");
    elem->setSourceUri(filename);
    doc->setChildIndex(elem->getName(), 0);
}

} // namespace MaterialX
