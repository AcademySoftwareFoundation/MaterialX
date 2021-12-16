import Module from './_build/JsMaterialXCore.js';
import { expect } from 'chai';
import { getMtlxStrings } from './testHelpers';

describe('XmlExport', () => {
    // These should be relative to cwd
    const includeTestPath = 'data/includes';
    const examplesPath = '../../../resources/Materials/Examples/Syntax';
    const libraryPath = '../../../libraries/stdlib';
    const exampleFilenames = [
        'Looks.mtlx',
        'PaintMaterials.mtlx',
        'PostShaderComposite.mtlx',
        'CustomNode.mtlx',
        'GeomInfos.mtlx',
        'MaterialBasic.mtlx',
        'MultiOutput.mtlx',
        'NodeGraphs.mtlx',
        'PreShaderComposite.mtlx',
        'SimpleSrf.mtlx',
        'SubGraphs.mtlx',
    ];

    let mx;
    before(async () => {
        mx = await Module();
    });

    it('Convert FilePath to string', () => {
        const exportOptions = new mx.XmlExportOptions();
        expect(typeof exportOptions.resolvedTexturePath).to.equal('string');
    });

    it('Export Document', async () => {
        const doc = mx.createDocument();
        await mx.readFromXmlFile(doc, "../../../resources/Materials/TestSuite/stdlib/looks/looks.mtlx");

        const exportOptions = new mx.XmlExportOptions();
        const xmlString = mx.exportToXmlString(doc, exportOptions);

        const exportedDoc = mx.createDocument();
        await mx.readFromXmlString(exportedDoc, xmlString);
    });
});

