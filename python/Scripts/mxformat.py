#!/usr/bin/env python
'''
Reformat a folder of MaterialX documents in place, optionally upgrading
the documents to the latest version of the standard.
'''

import argparse
import os
import xml.etree.ElementTree as ET

import MaterialX as mx

def is_well_formed(xml_string):
    error = ''
    try:
        ET.fromstring(xml_string)  
    except ET.ParseError as e:
        error = str(e)

def main():
    parser = argparse.ArgumentParser(description="Reformat a folder of MaterialX documents in place.")
    parser.add_argument('-y', '--yes', dest='yes', action="store_true", help="Proceed without asking for confirmation from the user.")
    parser.add_argument('-u', '--upgrade', dest='upgrade', action="store_true", help='Upgrade documents to the latest version of the standard.')
    parser.add_argument('-v', '--validate', dest='validate', action="store_true", help='Perform MaterialX validation on documents after reformatting.')
    parser.add_argument('-x', '--xml_syntax', dest='xml_syntax', action="store_true", help='Check XML syntax after reformatting.')
    parser.add_argument(dest="inputFolder", help="An input folder to scan for MaterialX documents.")
    opts = parser.parse_args()

    validDocs = dict()
    for root, dirs, files in os.walk(opts.inputFolder):
        for filename in files:
            fullpath = os.path.join(root, filename)
            if fullpath.endswith('.mtlx'):
                doc = mx.createDocument()
                try:
                    readOptions = mx.XmlReadOptions()
                    readOptions.readComments = True
                    readOptions.readNewlines = True    
                    readOptions.upgradeVersion = opts.upgrade
                    try:
                        mx.readFromXmlFile(doc, fullpath, mx.FileSearchPath(), readOptions)
                    except Exception as err:
                        print('Skipping "' + filename + '" due to exception: ' + str(err))
                        continue
                    validDocs[fullpath] = doc
                except mx.Exception:
                    pass

    if not validDocs:
        print('No MaterialX documents were found in "%s"' % (opts.inputFolder))
        return

    print('Found %s MaterialX files in "%s"' % (len(validDocs), opts.inputFolder))

    mxVersion = mx.getVersionIntegers()

    if not opts.yes:
        if opts.upgrade:
            question = 'Would you like to upgrade all %i documents to MaterialX v%i.%i in place (y/n)?' % (len(validDocs), mxVersion[0], mxVersion[1])
        else:
            question = 'Would you like to reformat all %i documents in place (y/n)?' % len(validDocs)
        answer = input(question)
        if answer != 'y' and answer != 'Y':
            return

    validate = opts.validate
    if validate:
        print(f'- Validate documents')
    xml_syntax = opts.xml_syntax
    if xml_syntax:
        print(f'- Check XML syntax')
    for (filename, doc) in validDocs.items():
        if xml_syntax:
            xml_string = mx.writeToXmlString(doc)
            errors = is_well_formed(xml_string)
            if errors:
                print(f'- Warning: Document {filename} is not well-formed XML: {errors}') 
        if validate:
            is_valid, errors = doc.validate()
            if not is_valid:
                print(f'- Warning: Document {filename} is invalid. Errors {errors}.')
        mx.writeToXmlFile(doc, filename)

    if opts.upgrade:
        print('Upgraded %i documents to MaterialX v%i.%i' % (len(validDocs), mxVersion[0], mxVersion[1]))
    else:
        print('Reformatted %i documents ' % len(validDocs))

if __name__ == '__main__':
    main()
