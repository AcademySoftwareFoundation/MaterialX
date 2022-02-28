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
    parser = argparse.ArgumentParser(description="Print documentation for each nodedef in the given document.")
    parser.add_argument(dest="inputFilename", help="Filename of the input MaterialX document.")
    parser.add_argument('--docType', dest='documentType', default='md', help='Document type. Default is "md" (markdown). "html" is also supported')
    opts = parser.parse_args()

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    for nd in doc.getNodeDefs():
        if opts.documentType == "html":
            print('<head><style>')
            print('table, th, td {')
            print('   border-bottom: 1px solid; border-collapse: collapse; padding: 1%;')
            print('}')
            print('</style></head>')
            print('<ul>')
            print('<li> <em>Nodedef</em>: %s' % nd.getName())
            print('<li> <em>Type</em>: %s' % nd.getType())
            print('<li> <em>Doc</em>: %s\n' % nd.getAttribute('doc'))
            print('</ul>')
            print('<table border-collapse=collapse><tr>')
            for h in HEADERS:
                print('<th>' + h + '</th>')
            print('</tr>')
            for inp in nd.getInputs():
                print('<tr>')
                infos = []
                infos.append(inp.getName())
                infos.append(inp.getType())
                val = inp.getValue()
                if inp.getType() == "float":
                    val = round(val, 6)
                infos.append(str(val))
                for attrname in ATTR_NAMES:
                    infos.append(inp.getAttribute(attrname))
                for info in infos:
                    print('<td>' + info + '</td>')
                print('</tr>')
            print('</table>')
        else:
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
