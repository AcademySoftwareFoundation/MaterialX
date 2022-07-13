//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass.js';

import { GammaCorrectionShader } from 'three/examples/jsm/shaders/GammaCorrectionShader.js';

import { Viewer } from './viewer.js'

let renderer, composer, orbitControls;

// Turntable option. For now the step size is fixed.
let turntableEnabled = false;
let turntableSteps = 360;
let turntableStep = 0;

// Get URL options. Fallback to defaults if not specified.
let materialFilename = new URLSearchParams(document.location.search).get("file");
if (!materialFilename) {
    materialFilename = 'Materials/Examples/StandardSurface/standard_surface_default.mtlx';
}

let viewer = Viewer.create();
init();
viewer.getEditor().updateProperties(0.9);

function init() 
{
    let canvas = document.getElementById('webglcanvas');
    let context = canvas.getContext('webgl2');

    // Handle material selection changes
    let materialsSelect = document.getElementById('materials');
    materialsSelect.value = materialFilename;
    materialsSelect.addEventListener('change', (e) => {
        materialFilename = e.target.value;
        viewer.getEditor().clearFolders();
        viewer.getMaterial().loadMaterials(viewer, materialFilename);
        viewer.getEditor().updateProperties(0.9);
    });

    // Handle geometry selection changes
    const scene = viewer.getScene();
    let geometrySelect = document.getElementById('geometry');
    geometrySelect.value = scene.getGeometryURL();
    geometrySelect.addEventListener('change', (e) => {
        scene.setGeometryURL(e.target.value);
        scene.loadGeometry(viewer, orbitControls);
    });

    // Set up scene
    scene.initialize();

    // Set up renderer
    renderer = new THREE.WebGLRenderer({ canvas, context });
    renderer.setSize(window.innerWidth, window.innerHeight);

    composer = new EffectComposer(renderer);
    const renderPass = new RenderPass(scene.getScene(), scene.getCamera());
    composer.addPass(renderPass);
    const gammaCorrectionPass = new ShaderPass(GammaCorrectionShader);
    composer.addPass(gammaCorrectionPass);

    window.addEventListener('resize', onWindowResize);

    // Set up controls
    orbitControls = new OrbitControls(scene.getCamera(), renderer.domElement);

    // Load model and shaders

    // Initialize editor
    viewer.getEditor().initialize();

    const hdrLoader = viewer.getHdrLoader();
    const fileLooder = viewer.getFileLoader();
    Promise.all([
        new Promise(resolve => hdrLoader.setDataType(THREE.FloatType).load('Lights/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => fileLooder.load('Lights/san_giuseppe_bridge_split.mtlx', resolve)),
        new Promise(resolve => hdrLoader.setDataType(THREE.FloatType).load('Lights/irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(function (resolve) {
            MaterialX().then((module) => {
                resolve(module);
            });
        }) 
    ]).then(async ([loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture, mxIn]) => 
    {
        // Initialize viewer + lighting
        await viewer.initialize(mxIn, renderer, loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture);

        // Load geometry  
        let scene = viewer.getScene();
        scene.loadGeometry(viewer, orbitControls);

        // Load materials
        viewer.getMaterial().loadMaterials(viewer, materialFilename);

        // Update assignments
        viewer.getMaterial().updateMaterialAssignments(viewer);

        canvas.addEventListener("keydown", handleKeyEvents, true);
        
    }).then(() => {
        animate();
    }).catch(err => {
        console.error(Number.isInteger(err) ? this.getMx().getExceptionMessage(err) : err);
    })

}

function onWindowResize() 
{
    viewer.getScene().updateCamera();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() 
{
    requestAnimationFrame(animate);

    if (turntableEnabled)
    {
        turntableStep = (turntableStep + 1) % 360;
        var turntableAngle = turntableStep * (360.0 / turntableSteps) / 180.0 * Math.PI;
        viewer.getScene()._scene.rotation.y = turntableAngle ;
    }

    composer.render();
    viewer.getScene().updateTransforms();
}

function handleKeyEvents(event)
{
    const V_KEY = 86;
    const P_KEY = 80;

    if (event.keyCode == V_KEY)
    {
        viewer.getScene().toggleBackgroundTexture();
    }
    else if (event.keyCode == P_KEY)
    {
        turntableEnabled = !turntableEnabled;
    }
}
