import Module from './_build/JsMaterialXCore.js';
import { expect } from 'chai';;
import { getMtlxStrings } from './testHelpers';

describe('GraphIO', () => {
    let mx;
    before(async () => {
        mx = await Module();
    });

    // These should be relative to cwd
    const includeTestPath = 'data/includes';
    const libraryPath = '../../libraries';
    const examplesPath = '../../resources/Materials/Examples';
    // TODO: Is there a better way to get these filenames than hardcoding them here?
    // The C++ tests load all files in the given directories. This would work in Node, but not in the browser.
    // Should we use a pre-test script that fetches the files and makes them available somehow?
    const libraryFilenames = ['/stdlib/stdlib_defs.mtlx', '/stdlib/stdlib_ng.mtlx', '/pbrlib/pbrlib_defs.mtlx',
                              '/pbrlib/pbrlib_ng.mtlx', 'bxdf/standard_surface.mtlx',
                              '/bxdf/usd_preview_surface.mtlx'];
    const exampleFilenames = [
        'StandardSurface/standard_surface_brick_procedural.mtlx',
        'StandardSurface/standard_surface_marble_solid.mtlx',
        'UsdPreviewSurface/usd_preview_surface_gold.mtlx',
    ];

    async function readStdLibrary(asString = false) {
        const libs = [];
        let iterable = libraryFilenames;
        if (asString) {
            const libraryMtlxStrings = getMtlxStrings(libraryFilenames, libraryPath);
            iterable = libraryMtlxStrings;
        }
        for (let file of iterable) {
            const lib = mx.createDocument();
            if (asString) {
                await mx.readFromXmlString(lib, file, libraryPath);
            } else {
                await mx.readFromXmlFile(lib, file, libraryPath);
            }
            libs.push(lib);
        };
        return libs;
    }    
    
    async function graphExamples(examples, libraries, readFunc, searchPath = undefined) 
    {
        for (let file of examples) 
        {
            const doc = mx.createDocument();
            await readFunc(doc, file, searchPath);
            // Import stdlib into the current document and validate it.
            for (let lib of libraries) {
                doc.importLibrary(lib);
            }

            let nodes = doc.getMaterialNodes();
            for (let node of nodes)
            {
                let graphOutput = "";

                // By default we don't want the per graph headers and will add them
                // in as a wrapper outside this function.
                let graphOptions = new mx.GraphIoGenOptions();
                graphOptions.setWriteGraphHeader(true);
                graphOptions.setWriteCategories(false);
                graphOptions.setWriteSubgraphs(true);
                graphOptions.setOrientation(mx.GraphOrientation.LEFT_RIGHT);
            
                // Test Mermaid
                let graphioMermaid = mx.MermaidGraphIo.create();        
                graphioMermaid.setGenOptions(graphOptions);
                let outputList = new mx.StringVec();
                outputList.push_back( node.getNamePath() )
                graphOutput = graphioMermaid.write(doc, outputList);
                expect(graphOutput.search('graph LR') > -1).to.be.true;

                // Test GraphViz dot
                let graphioDot = mx.DotGraphIo.create();
                graphioDot.setGenOptions(graphOptions);
                outputList = new mx.StringVec();
                outputList.push_back( node.getNamePath() )
                graphOutput = graphioDot.write(doc, outputList);
                expect(graphOutput.search('digraph') > -1).to.be.true;
            }
        }
    }

    it('Create graphs', async () => {

        const libs = await readStdLibrary(false);

        await graphExamples(exampleFilenames, libs,
            async (document, file, sp) => {
                await mx.readFromXmlFile(document, file, sp);
            }, examplesPath);
    });
});
