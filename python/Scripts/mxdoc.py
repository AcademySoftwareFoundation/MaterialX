#!/usr/bin/env python
'''
Output documentation for nodedefs for a given MaterialX document.
'''

import sys, os, string; os.environ["PYTHONIOENCODING"] = "utf-8"
import MaterialX as mx

def usage():
    print 'mxdoc.py: Output documentation for MaterialX nodedefs in Markdown format.'
    print 'Usage:  mxdoc.py <file.mtlx> [<outputfile.md>]'
    print '- Default output file name is "nodedef_documentation.md"'

HEADERS = ('Name', 'Type', 'Default Value',
           'UI min', 'UI max',
           'UI Soft Min', 'UI Soft Max',
           'UI step', 'UI group',
           'Description', 'UI Advanced',
           'Connectable')

ATTR_NAMES = ('uimin', 'uimax',
              'uisoftmin', 'uisoftmax',
              'uistep', 'uifolder',
              'doc', 'uiadvanced')

def main():
    if len(sys.argv) < 2:
        usage()
        sys.exit(0)

    outfilename = 'nodedef_documentation.md'
    if len(sys.argv) > 2:
        outfilename = sys.argv[2]

    filename = sys.argv[1]

    doc = mx.createDocument()

    # The mx.readFromXmlFile() will fail with
    #   'MaterialX.ExceptionFileMissing' if the file is not found, or
    #   'MaterialX.ExceptionParseError' if the document is found but not readable.
    try:
        mx.readFromXmlFile(doc, filename)
    except mx.ExceptionFileMissing as err:
        print err
        return
    except mx.ExceptionParseError as err:
        print '%s is not a valid MaterialX file.\n%s' % (filename, err)
        return

    file = open(outfilename, 'w+')

    nodedefs = doc.getNodeDefs()

    for nd in nodedefs:
        file.write('- *Nodedef*: %s\n' % nd.getName())
        file.write('- *Type*: %s\n' % nd.getType())
        file.write('- *Doc*: %s\n\n' % nd.getAttribute('doc'))
        file.write('| ' + ' | '.join(HEADERS) + ' |\n')
        file.write('|' + ' ---- |' * len(HEADERS) + '\n')
        for inp in nd.getInputs():
            infos = []
            infos.append(inp.getName())
            infos.append(inp.getType())
            val = inp.getValue()
            if infos[1] == "float":
                val = round(val, 6)
            infos.append(str(val))
            for attrname in ATTR_NAMES:
                infos.append(inp.getAttribute(attrname))
            infos.append("true")
            buf = '| ' + " | ".join(infos) + ' |\n'
            file.write(buf)
        for p in nd.getParameters():
            infos = []
            infos.append(p.getName())
            infos.append(p.getType())
            val = p.getValue()
            if infos[1] == "float":
                val = round(val, 6)
            infos.append(str(val))
            for attrname in ATTR_NAMES:
                infos.append(p.getAttribute(attrname))
            infos.append("false")
            buf = '| ' + " | ".join(infos) + ' |\n'
            file.write(buf)

    file.close()

if __name__ == '__main__':
    main()

