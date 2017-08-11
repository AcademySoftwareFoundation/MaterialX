#ifndef MATERIALXFORMAYA_FILETRANSLATOR_H
#define MATERIALXFORMAYA_FILETRANSLATOR_H

#include <maya/MPxFileTranslator.h>

class FileTranslator : public MPxFileTranslator {
public:
    FileTranslator();
    virtual ~FileTranslator();

    static void* creator();

    MStatus reader(const MFileObject& file, const MString& optionsString, FileAccessMode mode) override;
    MStatus writer(const MFileObject& file, const MString& optionsString, FileAccessMode mode) override;

    bool haveReadMethod () const override;
    bool haveWriteMethod () const override;

    MString defaultExtension() const override;
    MFileKind identifyFile(const MFileObject& fileName, const char* buffer, short size) const override;

    static const MString kTranslatorName;
    static const MString kOptionScript;
    static const MString kDefaultOptions;
};

#endif // MATERIALXFORMAYA_FILETRANSLATOR_H
