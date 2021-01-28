#!/usr/bin/env python
'''
Print markdown documentation for each nodedef in the given document.
'''

import sys, os, argparse
import MaterialX as mx

HEADERS = ('Name', 'Type', 'Default Value',
           'Uniform', 'UI min', 'UI max', 'UI Soft Min', 'UI Soft Max', 'UI step', 'UI group', 'UI Advanced')

ATTR_NAMES = ('uniform', 'uimin', 'uimax', 'uisoftmin', 'uisoftmax', 'uistep', 'uifolder', 'uiadvanced')

def main():
    parser = argparse.ArgumentParser(description="Print markdown documentation for each nodedef in the given document.")
    parser.add_argument(dest="inputFilename", help="Filename of the input MaterialX document.")
    opts = parser.parse_args()

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    for nd in doc.getNodeDefs():
        print('- *Nodedef*: %s' % nd.getName())
        print('- *Type*: %s' % nd.getType())
        print('- *Doc*: %s\n' % nd.getAttribute('doc'))
        print('| ' + ' | '.join(HEADERS) + ' |')
        print('|' + ' ---- |' * len(HEADERS) + '')
        for inp in nd.getInputs():
            infos = []
            infos.append(inp.getName())
            infos.append(inp.getType())
            val = inp.getValue()
            if inp.getType() == "float":
                val = round(val, 6)
            infos.append(str(val))
            for attrname in ATTR_NAMES:
                infos.append(inp.getAttribute(attrname))
            print('| ' + " | ".join(infos) + ' |')

if __name__ == '__main__':
    main()
