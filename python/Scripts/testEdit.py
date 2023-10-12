#!/usr/bin/env python

import sys, os, argparse
import MaterialX as mx

def main():
    parser = argparse.ArgumentParser(description="Test Python edit utility.")
    parser.add_argument(dest="inputFile", help="Document to edit.")
    opts = parser.parse_args()

    doc = mx.createDocument()
    print('Read file: ', opts.inputFile)
    mx.readFromXmlFile(doc, opts.inputFile)

    # Small example to remove the xpos, ypos position attributes
    for elem in doc.traverseTree():
        elem.removeAttribute("xpos")
        elem.removeAttribute("xpos")
    
    print('Wrote new version of file:', opts.inputFile)
    mx.writeToXmlFile(doc, opts.inputFile)

if __name__ == '__main__':
    main()
