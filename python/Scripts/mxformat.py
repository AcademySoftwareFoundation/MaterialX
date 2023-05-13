#!/usr/bin/env python
'''
Format document contents by reading and writing files. 
'''

import os, argparse
import MaterialX as mx

def getFiles(rootPath):
    filelist = []
    for subdir, dirs, files in os.walk(rootPath):
        for file in files:
            if file.endswith('mtlx'):
                filelist.append(os.path.join(subdir, file)) 
    return filelist

def main():
    parser = argparse.ArgumentParser(description="Format document by reading the file and writing it back out.")
    parser.add_argument(dest="inputFilename", help="Path of file or folder to format")
    parser.add_argument('--checkForChanges', dest='checkForChanges', type=mx.stringToBoolean, default=True, help='Check if a file has changed. Default is True')

    opts = parser.parse_args()

    fileList = []
    rootPath = opts.inputFilename
    if os.path.isdir(rootPath): 
        fileList = getFiles(rootPath)
    else:
        fileList.append(rootPath)

    # Preserve version, comments and newlines
    readOptions = mx.XmlReadOptions()
    readOptions.readComments = True
    readOptions.readNewlines = True
    readOptions.upgradeVersion = False

    writeOptions = mx.XmlWriteOptions() 

    writeCount = 0
    for file in fileList:
        doc = mx.createDocument()              
        mx.readFromXmlFile(doc, file, mx.FileSearchPath(), readOptions)

        writeFile = True
        if opts.checkForChanges:
            origString = mx.readFile(file)
            docString = mx.writeToXmlString(doc)
            if origString == docString:
                writeFile = False

        if writeFile:
            writeCount = writeCount + 1
            print('- Updated file %s.' % file)
            mx.writeToXmlFile(doc, file, writeOptions)

    print('- Updated %d files.' % writeCount)

if __name__ == '__main__':
    main()