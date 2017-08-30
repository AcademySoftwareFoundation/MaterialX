// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_FILETRANSLATOR_H
#define MATERIALXFORMAYA_FILETRANSLATOR_H

#include <maya/MPxFileTranslator.h>

namespace MaterialXForMaya
{

/// @class FileTranslator
/// Custom Maya File translator
class FileTranslator : public MPxFileTranslator {
  public:
    FileTranslator();
    virtual ~FileTranslator();

    /// Static creator
    static void* creator();

    /// Reader method
    MStatus reader(const MFileObject& file, const MString& optionsString, FileAccessMode mode) override;
    /// Writer method
    MStatus writer(const MFileObject& file, const MString& optionsString, FileAccessMode mode) override;

    /// Do we have a reader
    bool haveReadMethod () const override;
    /// Do we have a writer
    bool haveWriteMethod () const override;

    /// Default extention (MTLX)
    MString defaultExtension() const override;
    /// Identify file type
    MFileKind identifyFile(const MFileObject& fileName, const char* buffer, short size) const override;

    /// Name of translator
    static const MString kTranslatorName;
    /// Option script
    static const MString kOptionScript;
    /// Default options
    static const MString kDefaultOptions;
};

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_FILETRANSLATOR_H
