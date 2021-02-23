#!/usr/bin/env python
'''
Update MaterialX files within the given folder to the latest version.
'''

import sys, os, argparse
import MaterialX as mx

def main():
    parser = argparse.ArgumentParser(description="Update MaterialX files within the given folder to the latest version.")
    parser.add_argument("--yes", dest="yes", action="store_true", help="Proceed without asking for confirmation from the user.")
    parser.add_argument(dest="inputFolder", help="An input folder to scan for MaterialX documents.")
    opts = parser.parse_args()

    validDocs = dict()
    for root, dirs, files in os.walk(opts.inputFolder):
        for file in files:
            if file.endswith('.mtlx'):
                filename = os.path.join(root, file)
                doc = mx.createDocument()
                try:
                    readOptions = mx.XmlReadOptions()
                    readOptions.readComments = True
                    mx.readFromXmlFile(doc, filename, mx.FileSearchPath(), readOptions)
                    validDocs[filename] = doc
                except mx.Exception:
                    pass

    if not validDocs:
        print('No MaterialX documents were found in %s' % (opts.inputFolder))
        return

    print('Found %s MaterialX files in %s' % (len(validDocs), opts.inputFolder))

    mxVersion = mx.getVersionIntegers()

    if not opts.yes:
        question = 'Would you like to update all %i documents to MaterialX v%i.%i in place (y/n)?' % (len(validDocs), mxVersion[0], mxVersion[1])
        answer = input(question)
        if answer != 'y' and answer != 'Y':
            return

    for (filename, doc) in validDocs.items():
        mx.writeToXmlFile(doc, filename)

    print('Updated %i documents to MaterialX v%i.%i' % (len(validDocs), mxVersion[0], mxVersion[1]))

if __name__ == '__main__':
    main()
