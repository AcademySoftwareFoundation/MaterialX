//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

import * as THREE from 'three';
import { Viewer } from './viewer.js'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { dropHandler, dragOverHandler, setLoadingCallback, setSceneLoadingCallback } from './dropHandling.js';

let renderer, orbitControls;

// Turntable option. For now the step size is fixed.
let turntableEnabled = false;
let turntableSteps = 360;
let turntableStep = 0;

let captureRequested = false;

// Get URL options. Fallback to defaults if not specified.
let materialFilename = new URLSearchParams(document.location.search).get("file");
if (!materialFilename)
{
    materialFilename = 'Materials/Examples/StandardSurface/standard_surface_default.mtlx';
}

let viewer = Viewer.create();
init();
viewer.getEditor().updateProperties(0.9);

// Capture the current frame and save an image file.
function captureFrame()
{
    let canvas = document.getElementById('webglcanvas');
    var url = canvas.toDataURL();
    var link = document.createElement('a');
    link.setAttribute('href', url);
    link.setAttribute('target', '_blank');
    link.setAttribute('download', 'screenshot.png');
    link.click();
}

function init()
{
    let canvas = document.getElementById('webglcanvas');

    // Handle material selection changes
    let materialsSelect = document.getElementById('materials');
    materialsSelect.value = materialFilename;
    materialsSelect.addEventListener('change', (e) =>
    {
        materialFilename = e.target.value;
        viewer.getEditor().initialize();
        viewer.getMaterial().loadMaterials(viewer, materialFilename);
        viewer.getEditor().updateProperties(0.9);
        viewer.getScene().setUpdateTransforms();
    });

    // Handle geometry selection changes
    const scene = viewer.getScene();
    let geometrySelect = document.getElementById('geometry');
    geometrySelect.value = scene.getGeometryURL();
    geometrySelect.addEventListener('change', (e) =>
    {
        console.log('Change geometry to:', e.target.value);
        scene.setGeometryURL(e.target.value);
        scene.loadGeometry(viewer, orbitControls);
    });

    // Set up scene
    scene.initialize();

    // Set up renderer
    renderer = new THREE.WebGLRenderer({ antialias: true, canvas });
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.debug.checkShaderErrors = false;

    window.addEventListener('resize', onWindowResize);

    // Set up controls
    orbitControls = new OrbitControls(scene.getCamera(), renderer.domElement);
    orbitControls.addEventListener('change', () =>
    {
        viewer.getScene().setUpdateTransforms();
    })

    // Add hotkey 'f' to capture the current frame and save an image file.
    // See check inside the render loop when a capture can be performed.
    document.addEventListener('keydown', (event) =>
    {
        if (event.key === 'f')
        {
            captureRequested = true;
        }
    });

    // Initialize editor
    viewer.getEditor().initialize();

    const hdrLoader = viewer.getHdrLoader();
    const fileLoader = viewer.getFileLoader();
    Promise.all([
        new Promise(resolve => hdrLoader.load('Lights/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => hdrLoader.load('Lights/irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => fileLoader.load('Lights/san_giuseppe_bridge_split.mtlx', resolve)),
        new Promise(function (resolve)
        {
            MaterialX().then((module) =>
            {
                resolve(module);
            });
        })
    ]).then(async ([radianceTexture, irradianceTexture, lightRigXml, mxIn]) =>
    {
        // Initialize viewer + lighting
        await viewer.initialize(mxIn, renderer, radianceTexture, irradianceTexture, lightRigXml);

        // Load geometry  
        let scene = viewer.getScene();
        scene.loadGeometry(viewer, orbitControls);

        // Load materials
        viewer.getMaterial().loadMaterials(viewer, materialFilename);

        // Update assignments
        viewer.getMaterial().updateMaterialAssignments(viewer);

        canvas.addEventListener("keydown", handleKeyEvents, true);

    }).then(() =>
    {
        animate();
    }).catch(err =>
    {
        console.error(Number.isInteger(err) ? this.getMx().getExceptionMessage(err) : err);
    })

    // allow dropping files and directories
    document.addEventListener('drop', dropHandler, false);
    document.addEventListener('dragover', dragOverHandler, false);

    setLoadingCallback(file =>
    {
        materialFilename = file.fullPath || file.name;
        viewer.getEditor().initialize();
        viewer.getMaterial().loadMaterials(viewer, materialFilename);
        viewer.getEditor().updateProperties(0.9);
        viewer.getScene().setUpdateTransforms();
    });

    setSceneLoadingCallback(file =>
    {
        let glbFileName = file.fullPath || file.name;
        console.log('Drop geometry to:', glbFileName);
        scene.setGeometryURL(glbFileName);
        scene.loadGeometry(viewer, orbitControls);
    });

    // enable three.js Cache so that dropped files can reference each other
    THREE.Cache.enabled = true;
}

function onWindowResize()
{
    viewer.getScene().updateCamera();
    viewer.getScene().setUpdateTransforms();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate()
{
    requestAnimationFrame(animate);
    const scene = viewer.getScene();

    if (turntableEnabled)
    {
        turntableStep = (turntableStep + 1) % 360;
        var turntableAngle = turntableStep * (360.0 / turntableSteps) / 180.0 * Math.PI;
        scene._scene.rotation.y = turntableAngle;
        scene.setUpdateTransforms();
    }

    scene.updateUniforms();
    renderer.render(scene.getScene(), scene.getCamera());
    scene.updateTransforms();

    if (captureRequested)
    {
        captureFrame();
        captureRequested = false;
    }
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
