// MaterialX is served through a script tag in the test setup.

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

describe('Generate ESSL Shaders', function () {
    let mx;
    const canvas = document.createElement('canvas');
    const gl = canvas.getContext('webgl2');
    
    this.timeout(60000);

    before(async function () {
        mx = await MaterialX();
    });

    it('Compile Shaders', () => {
        const doc = createStandardSurfaceMaterial(mx);

        const gen = new mx.EsslShaderGenerator();
        const genContext = new mx.GenContext(gen);
        const stdlib = mx.loadStandardLibraries(genContext);

        doc.importLibrary(stdlib);

        const elem = mx.findRenderableElement(doc);
        const mxShader = gen.generate(elem.getNamePath(), elem, genContext);

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
});