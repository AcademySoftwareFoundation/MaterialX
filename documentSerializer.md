## Proposal for Document Serialization Interfaces

### Criteria
- Code interfacecs are available in C++ and can habing bindings provided in other languages. This currently includes Javascript and Python.

```c++
using OptionsMap = std::unordered_map<string, ValuePtr>;

class DocumentReader
{
  public:

    virtual DocumentPtr read(const FilePath& uri) = 0;

    virtual DocumentPtr read(const std::string& data)
    {
        return nullptr;
    }

    virtual DocumentPtr read(std::istream& stream)
    {
        return nullptr;
    }

    virtual StringVec supportedExtensions() const = 0;
};

class DocumentWriter
{
  public:
    virtual bool write(DocumentPtr, const FilePath& uri) = 0;
    virtual bool write(DocumentPtr, const std::string& data)
    {
        return false;
    }
    virtual void write(DocumentPtr, std::ostream& stream)
    {
        return false;
    }
}
```

```c++
// XML Reader (added code to guarantee standard libraries are set)
// if specified.
class XMLDocumentReader : public DocumentReader
{
    XMLDocumentReader() = default;

    DocumentPtr read(const FilePath& uri) override
    {
        DocumentPtr doc = createDocument()
        if (_standardLibrary)
        {
            doc->setDataLibrary(_standardLibrary);
        }
        readFromXmlFile(uri, 
                        _searchPath,
                        _readOptions);

        // Implementation for reading XML from a file path
    }

    DocumentPtr read(const std::string& data) override
    {
        DocumentPtr doc = createDocument()
        if (_standardLibrary)
        {
            doc->setDataLibrary(_standardLibrary);
        }
        readFromXmlString(doc, data, _searchPath, _readOptions );
    }

    DocumentPtr read(std::istream& stream) override
    {
        DocumentPtr doc = createDocument()
        if (_standardLibrary)
        {
            doc->setDataLibrary(_standardLibrary);
        }
        readFromXmlStream(doc, stream, _searchPath, _readOptions);
    }

    StringVec supportedExtensions() const override
    {
        return _supportedExtensions;
    }

    void setReadOptions(const XmlReadOptions& options)
    {
        _readOptions = options;
    }

    XmlReadOptions& getReadOptions() const
    {
        return _readOptions;
    }

    void setSearchPath(const FileSearchPath& searchPath)
    {
        _searchPath = searchPath;
    }

    FileSearchPath& getSearchPath() const
    {
        return _searchPath;
    }

    void setStandardLibrary(DocumentPtr &llib)
    {
        _standardLibrary = lib;
    }

  private:
    FileSearchPath _searchPath;
    XmlReadOptions _readOptions;
    StringVec _supportedExtensions = { ".mtlx" };
    DocumentPtr _standardLibrary = nullptr;


}
```

```c++
// prependXInclude is unused and belongs with Document class
class XMLDocumentWriter : public DocmentDeserializer
{
  public:
    XMLDocumentWriter() = default;

    bool write(const DocumentPtr doc, const FilePath& uri)
    {
        writeToXmlFile(doc, uri, _searchPath, _writeOptions);
        return true;
    }

    bool write(const DocumentPtr doc, const std::string& data)
    {
        writeToXmlString(doc, data, _searchPath, _writeOptions);
        return true;
    }

    bool write(const DocumentPtr doc, std::ostream& stream)
    {
        writeToXmlStream(doc, stream, _searchPath, _writeOptions);
        return true;
    }

    StringVec supportedExtensions() const override
    {
        return _supportedExtensions;
    }

    void setWriteOptions(const XmlWriteOptions& options)
    {
        _writeOptions = options;
    }

    XmlWriteOptions& getWriteOptions() const
    {
        return _writeOptions;
    }

    void setSearchPath(const FileSearchPath& searchPath)
    {
        _searchPath = searchPath;
    }

    FileSearchPath& getSearchPath() const
    {
        return _searchPath;
    }

  private:
    FileSearchPath _searchPath;
    XmlWriteOptions _writeOptions;
    StringVec _supportedExtensions = { ".mtlx" };
}