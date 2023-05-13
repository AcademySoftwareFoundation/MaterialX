import os, argparse
import MaterialX as mx

def getFiles(rootPath):
    filelist = []
    for subdir, dirs, files in os.walk(rootPath):
        for file in files:
            if file.endswith('mtlx'):
                filelist.append(os.path.join(subdir, file)) 
    return filelist



    return inlineFilesFound

def main():
    parser = argparse.ArgumentParser(description="Format document by reading the file and writing it back out.")
    parser.add_argument(dest="inputFilename", help="Path of file or folder to format")

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

    for file in fileList:
        doc = mx.createDocument()              
        mx.readFromXmlFile(doc, file, mx.FileSearchPath(), readOptions)
        writeOptions = mx.XmlWriteOptions() 
        mx.writeToXmlFile(doc, file, writeOptions)

if __name__ == '__main__':
    main()