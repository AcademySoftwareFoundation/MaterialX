import { expect } from 'chai';
import Module from './_build/JsMaterialX.js';
import { getMtlxStrings } from './testHelpers';

const _libraryFilenames = ['stdlib_defs.mtlx', 'stdlib_ng.mtlx', 'osl/stdlib_osl_impl.mtlx'];
const _exampleFilenames = [
    'CustomNode.mtlx',
    // 'Looks.mtlx', // <xi:include> is not yet supported
    'MaterialBasic.mtlx',
    'MultiOutput.mtlx',
    'NodeGraphs.mtlx',
    // 'PaintMaterials.mtlx', // <xi:include> is not yet supported
    // 'PostShaderComposite.mtlx', // <xi:include> is not yet supported
    'PreShaderComposite.mtlx',
];

it('Read XML', async () => {
    let mx;

    mx = await Module();

    // Read the standard library'
    let mtlxStrs = getMtlxStrings(_libraryFilenames, '../../../libraries/stdlib');
    const libs = [];
    mtlxStrs.forEach((mtlxStr) => {
        const lib = mx.createDocument();
        mx.readFromXmlString(lib, mtlxStr);
        libs.push(lib);
    });

    mtlxStrs = getMtlxStrings(_exampleFilenames, '../../../resources/Materials/Examples/Syntax');
    // Read and validate each example document.
    mtlxStrs.forEach((mtlxStr) => {
        const doc = mx.createDocument();
        mx.readFromXmlString(doc, mtlxStr);
        expect(doc.validate()).to.be.true;

        // Copy the document.
        const copiedDoc = doc.copy();
        expect(copiedDoc).to.eql(doc);
        copiedDoc.addLook();        
        // Make sure that the original doc does not contain any looks.
        expect(copiedDoc.getLooks().length).to.equal(1);
        expect(doc.getLooks().length).to.equal(0);

        // Traverse the document tree.
        let valueElementCount = 0;
        const treeIter = doc.traverseTree();
        for(const elem of treeIter) {
            if (elem instanceof mx.ValueElement) {
                valueElementCount++;
            }
        }
        expect(valueElementCount).to.be.greaterThan(0);


        // Serialize to XML.
        const writeOptions = new mx.XmlWriteOptions();
        writeOptions.writeXIncludeEnable = false;
        const xmlString = mx.writeToXmlString(doc, writeOptions);

        // Verify that the serialized document is identical.
        const writtenDoc = mx.createDocument();
        mx.readFromXmlString(writtenDoc, xmlString);
        expect(writtenDoc).to.eql(doc);

        // Combine document with the standard library.
        const doc2 = doc.copy();
        libs.forEach((lib) => {
            doc2.importLibrary(lib);
        });

        expect(doc2.validate()).to.be.true;
    });

    // Read the same document twice, and verify that duplicate elements
    // are skipped.
    const doc = mx.createDocument();
    const filenames = ['MaterialBasic.mtlx'];
    mtlxStrs = getMtlxStrings(filenames, '../../../resources/Materials/Examples/Syntax');
    mx.readFromXmlString(doc, mtlxStrs[0]);
    const readOptions = new mx.XmlReadOptions();
    mx.readFromXmlString(doc, mtlxStrs[0], readOptions);
    expect(doc.validate()).to.be.true;    
});
