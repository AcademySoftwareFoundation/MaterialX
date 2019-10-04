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
        file.write('- *Doc*: %s\n' % nd.getAttribute('doc'))
        file.write('| Name | Type | Default Value | UI min | UI max | UI group | Description | UI Advanced | Connectable |\n')
        file.write('| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |\n')
        for inp in nd.getInputs():
            name = inp.getName()
            type = inp.getType();
            value = inp.getValue()
            uimin = inp.getAttribute('uimin')
            uimax = inp.getAttribute('uimax')
            uifolder = inp.getAttribute('uifolder')
            docattr = inp.getAttribute('doc')
            adv = inp.getAttribute('uiadvanced')
            buf = '| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n' % (name, type, value, uimin, uimax, uifolder, docattr, adv, "true")
            file.write(buf)
        for p in nd.getParameters():
            name = p.getName()
            type = p.getType();
            value = inp.getValue()
            uimin = p.getAttribute('uimin')
            uimax = p.getAttribute('uimax')
            uifolder = p.getAttribute('uifolder')
            docattr = p.getAttribute('doc')
            adv = inp.getAttribute('uiadvanced')
            buf = '| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n' % (name, type, value, uimin, uimax, uifolder, docattr, adv, "false")
            file.write(buf)

    file.close()

if __name__ == '__main__':
    main()

