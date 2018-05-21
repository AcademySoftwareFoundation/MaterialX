//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_XMLIO_H
#define MATERIALX_XMLIO_H

/// @file
/// Support for the MTLX file format

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

/// @class XmlReadOptions
/// A set of options for controlling the behavior of XML read functions.
class XmlReadOptions : public CopyOptions
{
  public:
    XmlReadOptions() :
        readXIncludes(true)
    {
    }
    ~XmlReadOptions() { }
    
    /// If true, XInclude references will be read from disk and included in the
    /// document.  Defaults to true.
    bool readXIncludes;
};

/// @class ExceptionParseError
/// An exception that is thrown when a requested document cannot be parsed.
class ExceptionParseError : public Exception
{
  public:
    using Exception::Exception;
};

/// @class ExceptionFileMissing
/// An exception that is thrown when a requested file cannot be opened.
class ExceptionFileMissing : public Exception
{
  public:
    using Exception::Exception;
};

/// @name Read Functions
/// @{

/// Read a Document as XML from the given character buffer.
/// @param doc The Document into which data is read.
/// @param buffer The character buffer from which data is read.
/// @param readOptions An optional pointer to an XmlReadOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
void readFromXmlBuffer(DocumentPtr doc, const char* buffer, const XmlReadOptions* readOptions = nullptr);

/// Read a Document as XML from the given input stream.
/// @param doc The Document into which data is read.
/// @param stream The input stream from which data is read.
/// @param readOptions An optional pointer to an XmlReadOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
void readFromXmlStream(DocumentPtr doc, std::istream& stream, const XmlReadOptions* readOptions = nullptr);

/// Read a Document as XML from the given filename.
/// @param doc The Document into which data is read.
/// @param filename The filename from which data is read.
/// @param searchPath A semicolon-separated sequence of file paths, which will
///    be applied in order when searching for the given file and its includes.
///    Defaults to the empty string.
/// @param readOptions An optional pointer to an XmlReadOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
/// @throws ExceptionFileMissing if the file cannot be opened.
void readFromXmlFile(DocumentPtr doc,
                     const string& filename,
                     const string& searchPath = EMPTY_STRING,
                     const XmlReadOptions* readOptions = nullptr);

/// Read a Document as XML from the given string.
/// @param doc The Document into which data is read.
/// @param str The string from which data is read.
/// @param readOptions An optional pointer to an XmlReadOptions object.
///    If provided, then the given options will affect the behavior of the
///    read function.  Defaults to a null pointer.
/// @throws ExceptionParseError if the document cannot be parsed.
void readFromXmlString(DocumentPtr doc, const string& str, const XmlReadOptions* readOptions = nullptr);

/// @}
/// @name Write Functions
/// @{

/// Write a Document as XML to the given output stream.
/// @param doc The Document to be written.
/// @param stream The output stream to which data is written
/// @param writeXIncludes If true, elements with source file markings will be written
///    as XIncludes rather than explicit data.  Defaults to true.
/// @param predicate If provided, this function will be used to exclude specific elements
///    (those returning false) from the write operation.
void writeToXmlStream(DocumentPtr doc, std::ostream& stream, bool writeXIncludes = true,
                      const ElementPredicate& predicate = ElementPredicate());

/// Write a Document as XML to the given filename.
/// @param doc The Document to be written.
/// @param filename The filename to which data is written
/// @param writeXIncludes If true, elements with source file markings will be written
///    as XIncludes rather than explicit data.  Defaults to true.
/// @param predicate If provided, this function will be used to exclude specific elements
///    (those returning false) from the write operation.
void writeToXmlFile(DocumentPtr doc, const string& filename, bool writeXIncludes = true,
                    const ElementPredicate& predicate = ElementPredicate());

/// Write a Document as XML to a new string, returned by value.
/// @param doc The Document to be written.
/// @param writeXIncludes If true, elements with source file markings will be written
///    as XIncludes rather than explicit data.  Defaults to true.
/// @param predicate If provided, this function will be used to exclude specific elements
///    (those returning false) from the write operation.
/// @return The output string, returned by value
string writeToXmlString(DocumentPtr doc, bool writeXIncludes = true,
                        const ElementPredicate& predicate = ElementPredicate());

/// @}
/// @name Edit Functions
/// @{

/// Add an XInclude reference to the top of a Document, creating a generic
/// element to hold the reference filename.
/// @param doc The Document to be modified.
/// @param filename The filename of the XInclude reference to be added.
void prependXInclude(DocumentPtr doc, const string& filename);

/// @}

} // namespace MaterialX

#endif
