//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

import * as THREE from 'three';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';
import { DRACOLoader } from 'three/examples/jsm/loaders/DRACOLoader';
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass.js';

import { GammaCorrectionShader } from 'three/examples/jsm/shaders/GammaCorrectionShader.js';

import { prepareEnvTexture, toThreeUniforms, findLights, registerLights } from './helper.js'

let camera, scene, model, renderer, composer, controls, mx;

let normalMat = new THREE.Matrix3();
let viewProjMat = new THREE.Matrix4();
let worldViewPos = new THREE.Vector3();

const materialFilename = new URLSearchParams(document.location.search).get("file");

init();

// If no material file is selected, we programmatically create a jade material as a fallback
function fallbackMaterial(doc) {
    const ssName = 'SR_default';
    const ssNode = doc.addChildOfCategory('standard_surface', ssName);
    ssNode.setType('surfaceshader');
    ssNode.setInputValueFloat('base', 1.0);
    ssNode.setInputValueColor3('base_color', new mx.Color3(0.8, 0.8, 0.8));
    ssNode.setInputValueFloat('diffuse_roughness', 0);
    ssNode.setInputValueFloat('specular', 1);
    ssNode.setInputValueColor3('specular_color', new mx.Color3(1, 1, 1));
    ssNode.setInputValueFloat('specular_roughness', 0.2);
    ssNode.setInputValueFloat('specular_IOR', 1.5);
    ssNode.setInputValueFloat('specular_anisotropy', 0);
    ssNode.setInputValueFloat('specular_rotation', 0);
    ssNode.setInputValueFloat('metalness', 0);
    ssNode.setInputValueFloat('transmission', 0);
    ssNode.setInputValueColor3('transmission_color', new mx.Color3(1, 1, 1));
    ssNode.setInputValueFloat('transmission_depth', 0);
    ssNode.setInputValueColor3('transmission_scatter', new mx.Color3(0, 0, 0));
    ssNode.setInputValueFloat('transmission_scatter_anisotropy', 0);
    ssNode.setInputValueFloat('transmission_dispersion', 0);
    ssNode.setInputValueFloat('transmission_extra_roughness', 0);
    ssNode.setInputValueFloat('subsurface', 0)
    ssNode.setInputValueColor3('subsurface_color', new mx.Color3(1, 1, 1));
    ssNode.setInputValueColor3('subsurface_radius', new mx.Color3(1, 1, 1));
    ssNode.setInputValueFloat('subsurface_scale', 1);
    ssNode.setInputValueFloat('subsurface_anisotropy', 0);
    ssNode.setInputValueFloat('sheen', 0);
    ssNode.setInputValueColor3('sheen_color', new mx.Color3(1, 1, 1));
    ssNode.setInputValueFloat('sheen_roughness', 0.3);
    ssNode.setInputValueBoolean('thin_walled', false);
    ssNode.setInputValueFloat('coat', 0);
    ssNode.setInputValueColor3('coat_color', new mx.Color3(1, 1, 1));
    ssNode.setInputValueFloat('coat_roughness', 0.1);
    ssNode.setInputValueFloat('coat_anisotropy', 0.0);
    ssNode.setInputValueFloat('coat_rotation', 0.0);
    ssNode.setInputValueFloat('coat_IOR', 1.5);
    ssNode.setInputValueFloat('coat_affect_color', 0);
    ssNode.setInputValueFloat('coat_affect_roughness', 0);
    ssNode.setInputValueFloat('thin_film_thickness', 0);
    ssNode.setInputValueFloat('thin_film_IOR', 1.5);
    ssNode.setInputValueFloat('emission', 0);
    ssNode.setInputValueColor3('emission_color', new mx.Color3(1, 1, 1));
    ssNode.setInputValueColor3('opacity', new mx.Color3(1, 1, 1));

    const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
    smNode.setType('material');
    const shaderElement = smNode.addInput('surfaceshader');
    shaderElement.setType('surfaceshader');
    shaderElement.setNodeName(ssName);
}

function init() {
    let canvas = document.getElementById('webglcanvas');
    let materialsSelect = document.getElementById('materials');
    if (materialFilename) {
      materialsSelect.value = materialFilename;
    }
    let context = canvas.getContext('webgl2');

    materialsSelect.addEventListener('change', (e) => {
      window.location.href = `${window.location.origin}${window.location.pathname}?file=${e.target.value}`;
    });

    camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 1, 100);

    // Set up scene
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x4c4c52);
    scene.background.convertSRGBToLinear();


    renderer = new THREE.WebGLRenderer({canvas, context});
    renderer.setSize(window.innerWidth, window.innerHeight);

    composer = new EffectComposer( renderer );
    const renderPass = new RenderPass( scene, camera );
    composer.addPass( renderPass );
    const gammaCorrectionPass = new ShaderPass( GammaCorrectionShader );
    composer.addPass( gammaCorrectionPass );

    window.addEventListener('resize', onWindowResize);

    // controls
    controls = new OrbitControls(camera, renderer.domElement);

    // Load model and shaders
    const fileloader = new THREE.FileLoader();
    const dracoLoader = new DRACOLoader();
    dracoLoader.setDecoderPath( 'draco/' );
    const gltfLoader = new GLTFLoader();
    gltfLoader.setDRACOLoader( dracoLoader );
    const hdrloader = new RGBELoader();
    const textureLoader = new THREE.TextureLoader();

    Promise.all([
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => fileloader.load('Lights/san_giuseppe_bridge_split.mtlx', resolve)),
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => gltfLoader.load('Geometry/shaderball.glb', resolve)),
        new Promise(function (resolve) { 
          MaterialX().then((module) => { 
            resolve(module); 
          }); }),
        new Promise(resolve => materialFilename ? fileloader.load(materialFilename, resolve) : resolve())
    ]).then(async ([loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture, {scene: obj}, mxIn, mtlxMaterial]) => {
        // Initialize MaterialX and the shader generation context
        mx = mxIn;
        let doc = mx.createDocument();
        let gen = new mx.EsslShaderGenerator();
        let genContext = new mx.GenContext(gen);
        let stdlib = mx.loadStandardLibraries(genContext);
        doc.importLibrary(stdlib);        

        // Load material
        if (mtlxMaterial)
            await mx.readFromXmlString(doc, mtlxMaterial);
        else
            fallbackMaterial(doc);

        let elem = mx.findRenderableElement(doc);

        // Handle transparent materials
        const isTransparent = mx.isTransparentSurface(elem, gen.getTarget());
        genContext.getOptions().hwTransparency = isTransparent;

        // Load lighting setup into document
        const lightRigDoc = mx.createDocument();
        await mx.readFromXmlString(lightRigDoc, loadedLightSetup);
        doc.importLibrary(lightRigDoc);

        // Register lights with generation context
        const lights = findLights(doc);
        const lightData = registerLights(mx, lights, genContext);

        let shader = gen.generate(elem.getNamePath(), elem, genContext);

        // Get GL ES shaders and uniform values
        let fShader = shader.getSourceCode("pixel");       
        let vShader = shader.getSourceCode("vertex");
        let uniforms = {
          ...toThreeUniforms(JSON.parse(shader.getUniformValues("vertex")), textureLoader),
          ...toThreeUniforms(JSON.parse(shader.getUniformValues("pixel")), textureLoader)
        }

        const radianceTexture = prepareEnvTexture(loadedRadianceTexture, renderer.capabilities);
        const irradianceTexture = prepareEnvTexture(loadedIrradianceTexture, renderer.capabilities);

        Object.assign(uniforms, {
          u_numActiveLightSources: {value: lights.length},
          u_lightData: {value: lightData},
          u_envMatrix: {value: new THREE.Matrix4().makeRotationY(Math.PI)},
          u_envRadiance: {value: radianceTexture},
          u_envRadianceMips: {value: Math.trunc(Math.log2(Math.max(radianceTexture.image.width, radianceTexture.image.height))) + 1},
          u_envRadianceSamples: {value: 16},
          u_envIrradiance: {value: irradianceTexture}
        });

        // Create Three JS Material
        const threeMaterial = new THREE.RawShaderMaterial({
          uniforms: uniforms,
          vertexShader: vShader,
          fragmentShader: fShader,
          transparent: isTransparent,
          blendEquation: THREE.AddEquation,
          blendSrc: THREE.OneMinusSrcAlphaFactor,
          blendDst: THREE.SrcAlphaFactor
        });

        obj.traverse((child) => {
            if (child.isMesh) {
              child.geometry.computeTangents();
             
              // Use default MaterialX naming convention.
              child.geometry.attributes.i_position = child.geometry.attributes.position;
              child.geometry.attributes.i_normal = child.geometry.attributes.normal;
              child.geometry.attributes.i_tangent = child.geometry.attributes.tangent;
              child.geometry.attributes.i_texcoord_0 = child.geometry.attributes.uv;

              child.material = threeMaterial;
            }
        });
        model = obj;
        scene.add(model);

        // Fit camera to model
        const bbox = new THREE.Box3().setFromObject(model);
        const bsphere = new THREE.Sphere();
        bbox.getBoundingSphere(bsphere);

        controls.target = bsphere.center;
        camera.position.y = camera.position.z = bsphere.radius * 2.5;
        controls.update();

        camera.far = bsphere.radius * 10;
        camera.updateProjectionMatrix();

    }).then(() => {
        animate();
    }).catch(err => {
        console.error(Number.isInteger(err) ? mx.getExceptionMessage(err) : err);
    }) 

}

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() {  
    requestAnimationFrame(animate);

    composer.render();

    model.traverse((child) => {
      if (child.isMesh) {
        const uniforms = child.material.uniforms;
        if(uniforms) {
          uniforms.u_worldMatrix.value = child.matrixWorld;
          uniforms.u_viewProjectionMatrix.value = viewProjMat.multiplyMatrices(camera.projectionMatrix, camera.matrixWorldInverse);
          
          if (uniforms.u_viewPosition) 
            uniforms.u_viewPosition.value = camera.getWorldPosition(worldViewPos);

          if (uniforms.u_worldInverseTransposeMatrix)
            uniforms.u_worldInverseTransposeMatrix.value = new THREE.Matrix4().setFromMatrix3(normalMat.getNormalMatrix(child.matrixWorld));
        }
      }
    });
}
