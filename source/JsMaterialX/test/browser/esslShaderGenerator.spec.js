const chai = require('chai');
const expect = chai.expect;

function createStandardSurfaceMaterial(mx) {
    const doc = mx.createDocument();
    const ssName = 'SR_default';
    const ssNode = doc.addChildOfCategory('standard_surface', ssName);
    ssNode.setType('surfaceshader');
    const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
    smNode.setType('material');
    const shaderElement = smNode.addInput('surfaceshader');
    shaderElement.setType('surfaceshader');
    shaderElement.setNodeName(ssName);
    return doc;
}

describe('Generate ESSL Shaders', () => {
    let mx;
    let canvas = document.getElementById('canvas');
    let gl = canvas.getContext('webgl2');

    before(async () => {
        mx = await MaterialX();
    });

    it('Compile Shaders', () => {
        let doc = createStandardSurfaceMaterial(mx);

        let gen = new mx.EsslShaderGenerator();
        let genContext = new mx.GenContext(gen);
        let stdlib = mx.loadStandardLibraries(genContext);

        doc.importLibrary(stdlib);

        let elem = mx.findRenderableElement(doc);
        let mxShader = gen.generate(elem.getNamePath(), elem, genContext);

        const fShader = mxShader.getSourceCode("pixel");
        const vShader = mxShader.getSourceCode("vertex");

        const glVertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(glVertexShader, vShader);
        gl.compileShader(glVertexShader);

        const glPixelShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(glPixelShader, fShader);
        gl.compileShader(glPixelShader);

        expect(gl.getShaderParameter(glVertexShader, gl.COMPILE_STATUS)).to.equal(true);
        expect(gl.getShaderParameter(glPixelShader, gl.COMPILE_STATUS)).to.equal(true);
    });
}).timeout(100000);