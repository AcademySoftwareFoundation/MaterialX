#!/usr/bin/env python
'''
Format MaterialX document content by reanding and writing files. Optionally
upgrade the document version. 
'''

import sys, os, argparse
import MaterialX as mx

def getFiles(rootPath):
    filelist = []
    for subdir, dirs, files in os.walk(rootPath):
        for file in files:
            if file.endswith('mtlx'):
                filelist.append(os.path.join(subdir, file)) 
    return filelist

def main():
    parser = argparse.ArgumentParser(description="Format document by reading the file and writing it back out. Optionally update to the latest version.")
    parser.add_argument("--yes", dest="yes", action="store_true", help="Proceed without asking for confirmation from the user.")
    parser.add_argument('--checkForChanges', dest='checkForChanges', type=mx.stringToBoolean, default=True, help='Check if a file has changed. Default is True')
    parser.add_argument('--upgradeVersion', dest='upgradeVersion', type=mx.stringToBoolean, default=False, help='Upgrade the document version. Default is False')
    parser.add_argument(dest="inputFolder", help="An input folder to scan for MaterialX documents.")
    opts = parser.parse_args()

    fileList = []
    rootPath = opts.inputFolder
    if os.path.isdir(rootPath): 
        fileList = getFiles(rootPath)
    else:
        fileList.append(rootPath)

    # Preserve version, comments and newlines
    readOptions = mx.XmlReadOptions()
    readOptions.readComments = True
    readOptions.readNewlines = True    
    readOptions.upgradeVersion = opts.upgradeVersion

    validDocs = dict()
    for filename in fileList:
        doc = mx.createDocument()
        try:
            mx.readFromXmlFile(doc, filename, mx.FileSearchPath(), readOptions)
            validDocs[filename] = doc
        except mx.Exception:
            pass

    if not validDocs:
        print('No MaterialX documents were found in "%s"' % (opts.inputFolder))
        return

    print('Found %s MaterialX files in "%s"' % (len(validDocs), opts.inputFolder))

    mxVersion = mx.getVersionIntegers()

    if not opts.yes:
        question = 'Would you like to update all %i documents in place (y/n)?' % len(validDocs)
        if opts.upgradeVersion:
            question = 'Would you like to update all %i documents to MaterialX v%i.%i in place (y/n)?' % (len(validDocs), mxVersion[0], mxVersion[1])
        answer = input(question)
        if answer != 'y' and answer != 'Y':
            return

    writeCount = 0
    for (filename, doc) in validDocs.items():
        writeFile = True
        if opts.checkForChanges:
            origString = mx.readFile(filename)
            docString = mx.writeToXmlString(doc)
            if origString == docString:
                writeFile = False

        if writeFile:
            writeCount = writeCount + 1
            print('- Updated file %s.' % filename)
            mx.writeToXmlFile(doc, filename)

    if opts.upgradeVersion:
        print('Updated %i documents to MaterialX v%i.%i' % (writeCount, mxVersion[0], mxVersion[1]))
    else:
        print('Updated %i documents ' % writeCount)

if __name__ == '__main__':
    main()
