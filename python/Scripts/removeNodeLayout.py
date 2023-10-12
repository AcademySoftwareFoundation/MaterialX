#!/usr/bin/env python
'''
Utility to remove node layout formatting
'''

import argparse
import MaterialX as mx

def main():
    parser = argparse.ArgumentParser(description="Utility to remove node layout formatting.")
    parser.add_argument(dest="inputFile", help="Document to edit.")
    opts = parser.parse_args()

    doc = mx.createDocument()
    mx.readFromXmlFile(doc, opts.inputFile)

    modified = False
    attributesToRemove = ['xpos', 'ypos']
    for elem in doc.traverseTree():
        for attr in attributesToRemove:
            if elem.hasAttribute(attr):
                modified = True
                elem.removeAttribute(attr)
    
    if modified:
        print('Removed layout information from file:', opts.inputFile)
        mx.writeToXmlFile(doc, opts.inputFile)        

if __name__ == '__main__':
    main()
