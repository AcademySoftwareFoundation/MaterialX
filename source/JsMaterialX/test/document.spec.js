import { expect } from 'chai';
import { initMaterialX } from './testHelpers';

describe('Build Document', () => {
    let mx, doc;
    before(async () => {
        mx = await initMaterialX();
        // Create a document.
        doc = mx.createDocument();
    });

    function expectError(type, cb) {
        try {
            cb();
        } catch (exceptionPtr) {
            const message = mx.getExceptionMessage(exceptionPtr);
            expect(message.indexOf(type) !== -1).to.be.true;
        }
    }

    let nodeGraph;
    it('Create a node graph with constant and image sources.', () => {
        nodeGraph = doc.addNodeGraph();
        expect(nodeGraph).to.exist;
        expectError('Child name is not unique: nodegraph1', () => {
            doc.addNodeGraph(nodeGraph.getName());
        });
    });

    let output1, output2, constant, image;
    it('Connect sources to outputs.', () => {
        constant = nodeGraph.addNode('constant');
        image = nodeGraph.addNode('image');
        output1 = nodeGraph.addOutput();
        output2 = nodeGraph.addOutput();
        output1.setConnectedNode(constant);
        output2.setConnectedNode(image);
        expect(output1.getConnectedNode()).to.eql(constant);
        expect(output2.getConnectedNode()).to.eql(image);
        expect(output1.getUpstreamElement()).to.eql(constant);
        expect(output2.getUpstreamElement()).to.eql(image);
    });

    it('Set constant node color', () => {
        const color = new mx.Color3(0.1, 0.2, 0.3);
        constant.setInputValuecolor3('value', color);
        expect(constant.getInputValue('value').getData()).to.eql(color);
    });

    it('Set image node file', () => {
        const file = 'image1.tif';
        image.setInputValuestring('file', file, 'filename');
        expect(image.getInputValue('file').getData()).to.eql(file);
    });

    it('Create a custom nodedef', () => {
        const nodeDef = doc.addNodeDef('nodeDef1', 'float', 'turbulence3d');
        nodeDef.setInputValueinteger('octaves', 3);
        nodeDef.setInputValuefloat('lacunarity', 2.0);
        nodeDef.setInputValuefloat('gain', 0.5);
        const custom = nodeGraph.addNode('turbulence3d', 'turbulence1', 'float');
        expect(custom.getInputValue('octaves').getData()).to.equal(3);
        custom.setInputValueinteger('octaves', 5);
        expect(custom.getInputValue('octaves').getData()).to.equal(5);
    });

    it('Validate the document', () => {
        expect(doc.validate()).to.be.true;
    });

    it('Test scoped attributes', () => {
        nodeGraph.setFilePrefix('folder/');
        nodeGraph.setColorSpace('lin_rec709');
        expect(image.getInput('file').getResolvedValueString()).to.equal('folder/image1.tif');
        expect(constant.getActiveColorSpace()).to.equal('lin_rec709');
    });

    // let diffColor, specColor, roughness, texId;
    // it('Create a simple shader interface', () => {
    //     const shaderDef = doc.addNodeDef('shader1', 'surfaceshader', 'simpleSrf');
    //     diffColor = shaderDef.setInputValuecolor3('diffColor', new mx.Color3(1.0, 1.0, 1.0));
    //     specColor = shaderDef.setInputValuecolor3('specColor', new mx.Color3(0.0, 0.0, 0.0));
    //     roughness = shaderDef.setInputValuefloat('roughness', 0.25);
    //     texId = shaderDef.setTokenValue('texId', '01');
    //     expect(roughness.getIsUniform()).to.equal(false);
    //     roughness.setIsUniform(true);
    //     expect(roughness.getIsUniform()).to.equal(true);
    //     expect(roughness.getValue().getData()).to.equal(0.25);
    // });

    // let material, shaderRef;
    // it('Create a material that instantiates the shader', () => {
    //     material = doc.addMaterial();
    //     shaderRef = material.addShaderRef('shaderRef1', 'simpleSrf');
    //     expect(material.getPrimaryShaderName()).to.equal('simpleSrf');
    //     expect(material.getPrimaryShaderInputs().length).to.equal(3);
    //     expect(material.getPrimaryShaderTokens().length).to.equal(1);
    //     expect(roughness.getBoundValue(material).getData()).to.equal(0.25);
    // });

    // it('Bind a shader input to a float value', () => {
    //     const bindInput = shaderRef.addBindInput('roughness');
    //     bindInput.setValuefloat(0.5);
    //     expect(roughness.getBoundValue(material).getData()).to.equal(0.5);
    //     expect(roughness.getDefaultValue().getData()).to.equal(0.25);
    // });

    // let bindInput;
    // it('Bind a shader input to a color value', () => {
    //     bindInput = shaderRef.addBindInput('specColor');
    //     bindInput.setValuecolor3(new mx.Color3(0.5, 0.5, 0.5));
    //     expect(specColor.getBoundValue(material).getData()).to.eql(new mx.Color3(0.5, 0.5, 0.5));
    //     expect(specColor.getDefaultValue().getData()).to.eql(new mx.Color3(0.0, 0.0, 0.0));
    // });

    // it('Bind a shader input to a graph output', () => {
    //     bindInput = shaderRef.addBindInput('diffColor');
    //     bindInput.setConnectedOutput(output2);
    //     expect(diffColor.getUpstreamElement(material)).to.eql(output2);
    //     expect(diffColor.getBoundValue(material)).to.be.null;
    //     expect(diffColor.getDefaultValue().getData()).to.eql(new mx.Color3(1.0, 1.0, 1.0));
    // });

    // it('Bind a shader token to a value', () => {
    //     const bindToken = shaderRef.addBindToken('texId');
    //     bindToken.setValuestring('02');
    //     expect(texId.getBoundValue(material).getData()).to.equal('02');
    //     expect(texId.getDefaultValue().getData()).to.equal('01');
    // });

    // it('Create an inherited material', () => {
    //     const material2 = doc.addMaterial();
    //     material2.setInheritsFrom(material);
    //     expect(roughness.getBoundValue(material2).getData()).to.equal(0.5);
    //     expect(diffColor.getUpstreamElement(material2)).to.eql(output2);
    // });

    // let look;
    // it('Create a look for the material', () => {
    //     look = doc.addLook();
    //     expect(doc.getLooks().length).to.equal(1);
    // });

    // it('Bind the material to a geometry string', () => {
    //     const matAssign1 = look.addMaterialAssign('matAssign1', material.getName());
    //     matAssign1.setGeom('/robot1');
    //     expect(matAssign1.getReferencedMaterial()).to.eql(material);
    //     expect(material.getGeometryBindings('/robot1').length).to.equal(1);
    //     expect(material.getGeometryBindings('/robot2').length).to.equal(0);
    // });

    // it('Bind the material to a collection', () => {
    //     const matAssign2 = look.addMaterialAssign('matAssign2', material.getName());
    //     const collection = doc.addCollection();
    //     collection.setIncludeGeom('/robot2');
    //     collection.setExcludeGeom('/robot2/left_arm');
    //     matAssign2.setCollection(collection);
    //     expect(material.getGeometryBindings('/robot2').length).to.equal(1);
    //     expect(material.getGeometryBindings('/robot2/right_arm').length).to.equal(1);
    //     expect(material.getGeometryBindings('/robot2/left_arm').length).to.equal(0);
    // });

    // it('Create a property assignment', () => {
    //     const propertyAssign = look.addPropertyAssign();
    //     propertyAssign.setProperty('twosided');
    //     propertyAssign.setGeom('/robot1');
    //     propertyAssign.setValueboolean(true);
    //     expect(propertyAssign.getProperty()).to.equal('twosided');
    //     expect(propertyAssign.getGeom()).to.equal('/robot1');
    //     expect(propertyAssign.getValue().getData()).to.equal(true);
    // });

    // it('Create a property set assignment', () => {
    //     const propertySet = doc.addPropertySet();
    //     propertySet._setPropertyValueboolean('matte', false, '');
    //     expect(propertySet._getPropertyValue('matte').getData()).to.be.false;
    //     const propertySetAssign = look.addPropertySetAssign();
    //     propertySetAssign.setPropertySet(propertySet);
    //     propertySetAssign.setGeom('/robot1');
    //     expect(propertySetAssign.getPropertySet()).to.eql(propertySet);
    //     expect(propertySetAssign.getGeom()).to.equal('/robot1');
    // });

    // it('Create a variant set', () => {
    //     const variantSet = doc.addVariantSet();
    //     variantSet.addVariant('original');
    //     variantSet.addVariant('damaged');
    //     expect(variantSet.getVariants().length).to.equal(2);
    // });

    // it('Disconnect outputs from sources', () => {
    //     output1.setConnectedNode(null);
    //     output2.setConnectedNode(null);
    //     expect(output1.getConnectedNode()).to.be.null;
    //     expect(output2.getConnectedNode()).to.be.null;
    // });
});
