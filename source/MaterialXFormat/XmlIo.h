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

/// @name Reading
/// @{

/// @struct XmlReadOptions
/// Structure to hold options to use during reading. 
///
struct XmlReadOptions
{
    XmlReadOptions() :
        readXincludes(true),
        skipDuplicates(false) {}

    virtual ~XmlReadOptions() {}
    
    /// Set option for whether XInclude references will be read from disk and included in the document.
    /// @param value Option value
    void setReadXincludes(bool value)
    {
        readXincludes = value;
    }

    /// Get option for whether XInclude references will be read from disk and included in the document.
    bool getReadXincludes() const
    {
        return readXincludes;
    }

    /// Set option for whether to skip reading in Elements with duplicate names.
    /// @param value Option value
    void setSkipDuplicates(bool value)
    {
        skipDuplicates = value;
    }

    /// Get option for whether to skip reading in Elements with duplicate names.
    bool getSkipDuplicates() const
    {
        return skipDuplicates;
    }

    /// If true, XInclude references will be read from disk and included in the document. Defaults to true.
    bool readXincludes;
    /// If true, Will skip reading in Elements with duplicate names. Defaults to false.
    bool skipDuplicates;
};

/// Read a document as XML from the given character buffer.
/// @param doc The document into which data is read.
/// @param buffer The character buffer from which data is read.
/// @param readOptions Options to use during reading. Default is null which indicates to use the 
///    default values as specified in the XmlReadOptions structure.
/// @throws ExceptionParseError if the document cannot be parsed.
void readFromXmlBuffer(DocumentPtr doc, const char* buffer, const XmlReadOptions* readOptions = nullptr);

/// Read a document as XML from the given input stream.
/// @param doc The document into which data is read.
/// @param stream The input stream from which data is read.
/// @param readOptions Options to use during reading. Default is null which indicates to use the 
///    default values as specified in the XmlReadOptions structure.
/// @throws ExceptionParseError if the document cannot be parsed.
void readFromXmlStream(DocumentPtr doc, std::istream& stream, const XmlReadOptions* readOptions = nullptr);

/// Read a document as XML from the given filename.
/// @param doc The document into which data is read.
/// @param filename The filename from which data is read.
/// @param searchPath A semicolon-separated sequence of file paths, which will
///    be applied in order when searching for the given file and its includes.
///    Defaults to the empty string.
/// @param readOptions Options to use during reading. Default is null which indicates to use the 
///    default values as specified in the XmlReadOptions structure.
/// @throws ExceptionParseError if the document cannot be parsed.
/// @throws ExceptionFileMissing if the file cannot be opened.
void readFromXmlFile(DocumentPtr doc,
                     const string& filename,
                     const string& searchPath = EMPTY_STRING,
                     const XmlReadOptions* readOptions = nullptr);

/// Read a document as XML from the given string.
/// @param doc The document into which data is read.
/// @param str The string from which data is read.
/// @param readOptions Options to use during reading. Default is null which indicates to use the 
///    default values as specified in the XmlReadOptions structure.
/// @throws ExceptionParseError if the document cannot be parsed.
void readFromXmlString(DocumentPtr doc, const string& str, const XmlReadOptions* readOptions = nullptr);

/// @}
/// @name Writing
/// @{

/// Write a document as XML to the given output stream.
/// @param doc The document to be written.
/// @param stream The output stream to which data is written
/// @param writeXIncludes If true, elements with source file markings will be written
///    as XIncludes rather than explicit data.  Defaults to true.
/// @param predicate If provided, this function will be used to exclude specific elements
///    (those returning false) from the write operation.
void writeToXmlStream(DocumentPtr doc, std::ostream& stream, bool writeXIncludes = true,
                      const ElementPredicate& predicate = ElementPredicate());

/// Write a document as XML to the given filename.
/// @param doc The document to be written.
/// @param filename The filename to which data is written
/// @param writeXIncludes If true, elements with source file markings will be written
///    as XIncludes rather than explicit data.  Defaults to true.
/// @param predicate If provided, this function will be used to exclude specific elements
///    (those returning false) from the write operation.
void writeToXmlFile(DocumentPtr doc, const string& filename, bool writeXIncludes = true,
                    const ElementPredicate& predicate = ElementPredicate());

/// Write a document as XML to a new string, returned by value.
/// @param doc The document to be written.
/// @param writeXIncludes If true, elements with source file markings will be written
///    as XIncludes rather than explicit data.  Defaults to true.
/// @param predicate If provided, this function will be used to exclude specific elements
///    (those returning false) from the write operation.
/// @return The output string, returned by value
string writeToXmlString(DocumentPtr doc, bool writeXIncludes = true,
                        const ElementPredicate& predicate = ElementPredicate());

/// @}
/// @name Editing
/// @{

/// Add an XInclude reference to the top of the current document, creating
/// a generic element to hold the reference filename.
/// @param doc The document to be modified.
/// @param filename The filename of the XInclude reference to be added.
void prependXInclude(DocumentPtr doc, const string& filename);

/// @}

/// @class @ExceptionParseError
/// An exception that is thrown when a requested document cannot be parsed.
class ExceptionParseError : public Exception
{
  public:
    ExceptionParseError(const string& msg) :
        Exception(msg)
    {
    }

    ExceptionParseError(const ExceptionParseError& e) :
        Exception(e)
    {
    }

    virtual ~ExceptionParseError() throw()
    {
    }
};

/// @class @ExceptionFileMissing
/// An exception that is thrown when a requested file cannot be opened.
class ExceptionFileMissing : public Exception
{
  public:
    ExceptionFileMissing(const string& msg) :
        Exception(msg)
    {
    }

    ExceptionFileMissing(const ExceptionFileMissing& e) :
        Exception(e)
    {
    }

    virtual ~ExceptionFileMissing() throw()
    {
    }
};

} // namespace MaterialX

#endif
