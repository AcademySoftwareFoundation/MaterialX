//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

import * as THREE from 'three';
import { Viewer } from './viewer.js'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { dropHandler, dragOverHandler, setLoadingCallback, setSceneLoadingCallback } from './dropHandling.js';

// Rendering backend, selected at build time per bundle (see webpack.config.js). The WebGL
// bundle uses classic THREE.WebGLRenderer + RawShaderMaterial (ESSL); the WebGPU bundle
// aliases `three` to three/webgpu and uses WebGPURenderer + NodeMaterial (WGSL via the
// upstream WgslShaderGenerator). A toggle switches between the two HTML pages.
const BACKEND = (typeof __BACKEND__ !== 'undefined') ? __BACKEND__ : 'webgl';
let TSL = null; // three/tsl namespace, loaded for the WebGPU backend only

let renderer, orbitControls;
let webgpuSupported = null; // cached result of navigator.gpu probe

// FPS overlay state
let fpsOverlay = null;
let showFPS = true; 
let lastFrameTime = performance.now();
let frameCount = 0;
let fps = 0;

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

// Create FPS overlay
function createFPSOverlay() {
    fpsOverlay = document.createElement('div');
    fpsOverlay.className = 'fps-overlay';
    fpsOverlay.innerText = 'FPS: 0';
    document.body.appendChild(fpsOverlay);
}

function setFPSOverlayVisible(visible) {
    if (fpsOverlay) {
        fpsOverlay.style.display = visible ? 'block' : 'none';
    }
}

createFPSOverlay();
setFPSOverlayVisible(showFPS);

probeWebGPU().then(() =>
{
    init();
    viewer.getEditor().updateProperties(0.9);
});

/** Probe WebGPU availability once; used by the backend toggle and init fallback. */
async function probeWebGPU()
{
    if (webgpuSupported !== null) return webgpuSupported;
    if (!navigator.gpu)
    {
        webgpuSupported = false;
        return false;
    }
    try
    {
        const adapter = await navigator.gpu.requestAdapter();
        webgpuSupported = !!adapter;
    }
    catch
    {
        webgpuSupported = false;
    }
    return webgpuSupported;
}

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

    // Set up renderer for the selected backend.
    if (BACKEND === 'webgpu')
    {
        if (!webgpuSupported)
        {
            showWebGPUFallbackBanner();
            return;
        }
        renderer = new THREE.WebGPURenderer({ antialias: true, canvas });
    }
    else
    {
        renderer = new THREE.WebGLRenderer({ antialias: true, canvas });
        renderer.debug.checkShaderErrors = false;
    }
    renderer.setSize(window.innerWidth, window.innerHeight);
    // WebGL encodes sRGB in the pixel shader; WebGPU outputs linear and relies on this conversion.
    renderer.outputColorSpace = THREE.SRGBColorSpace;

    addBackendToggle(webgpuSupported);

    window.addEventListener('resize', onWindowResize);

    // Set up controls
    orbitControls = new OrbitControls(scene.getCamera(), renderer.domElement);

    // Add hotkey 'f' to capture the current frame and save an image file.
    // See check inside the render loop when a capture can be performed.

    document.addEventListener('keydown', (event) => {
        // Toggle FPS timer with T key
        if (event.key === 't' || event.key === 'T') {
            showFPS = !showFPS;
            setFPSOverlayVisible(showFPS);
            event.preventDefault();
        }
        // Capture frame with Shift+F
        else if ((event.key === 'f' || event.key === 'F') && event.shiftKey) {
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
        import(/* webpackIgnore: true */ './JsMaterialXGenShader.js')
            .then(({ default: MaterialX }) => MaterialX())
    ]).then(async ([radianceTexture, irradianceTexture, lightRigXml, mxIn]) =>
    {
        // WebGPU: load the TSL namespace (used by the WGSL→NodeMaterial bridge) and
        // initialize the device before the first render.
        if (BACKEND === 'webgpu')
        {
            TSL = await import('three/tsl');
            await renderer.init();
        }

        // Initialize viewer + lighting
        await viewer.initialize(mxIn, renderer, radianceTexture, irradianceTexture, lightRigXml,
            { backend: BACKEND, TSL });

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
        const mx = viewer.getMx();
        const message = (Number.isInteger(err) && mx)
            ? mx.getExceptionMessage(err)
            : (err && err.message ? err.message : err);
        console.error(message, err);
        if (BACKEND === 'webgpu')
            showWebGPUFallbackBanner('WebGPU initialization failed. Use the WebGL view instead.');
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
    renderer.setSize(window.innerWidth, window.innerHeight);
}

// Floating toggle to switch rendering backend. Each backend is a separate bundle/page
// (different three build), so switching navigates between index.html and index-webgpu.html
// while preserving the current ?file/?geom query so the comparison stays on the same content.
function addBackendToggle(gpuAvailable)
{
    const search = window.location.search;
    const wrap = document.createElement('div');
    // Bottom-center: clear of the material/geometry selectors (top-left), the property
    // editor (top-right), and the FPS overlay (bottom-left).
    wrap.style.cssText = 'position:fixed;bottom:10px;left:50%;transform:translateX(-50%);z-index:1000;' +
        'display:flex;gap:4px;flex-wrap:wrap;align-items:center;' +
        'font-family:sans-serif;font-size:12px;background:rgba(0,0,0,0.5);padding:4px 6px;border-radius:4px;max-width:90vw;';
    const label = document.createElement('span');
    label.textContent = 'Renderer:';
    label.style.cssText = 'color:#ccc;align-self:center;';
    wrap.appendChild(label);

    for (const [name, page] of [['WebGL', 'index.html'], ['WebGPU', 'index-webgpu.html']])
    {
        const isWebGPU = (name === 'WebGPU');
        const active = (name.toLowerCase() === BACKEND);
        const disabled = isWebGPU && gpuAvailable === false;
        const btn = document.createElement('button');
        btn.textContent = name;
        btn.title = disabled
            ? 'WebGPU is not available in this browser'
            : (isWebGPU
                ? 'WebGPU uses MaterialX WGSL + TSL NodeMaterial (same light rig and IBL as WebGL)'
                : 'WebGL uses full MaterialX light rig + GLSL ES shaders');
        btn.style.cssText = 'border:none;border-radius:3px;padding:3px 8px;' +
            (active ? 'background:#4a9;color:#fff;font-weight:bold;' :
                disabled ? 'background:#222;color:#666;cursor:not-allowed;' :
                    'background:#333;color:#bbb;cursor:pointer;');
        if (!active && !disabled)
        {
            btn.addEventListener('click', () =>
            {
                btn.textContent = '…';
                btn.disabled = true;
                window.location.href = page + search;
            });
        }
        wrap.appendChild(btn);
    }
    document.body.appendChild(wrap);
}

function showWebGPUFallbackBanner(message)
{
    const search = window.location.search;
    const banner = document.createElement('div');
    banner.style.cssText = 'position:fixed;inset:0;display:flex;align-items:center;justify-content:center;' +
        'background:rgba(0,0,0,0.85);color:#eee;font-family:sans-serif;font-size:14px;z-index:2000;padding:24px;text-align:center;';
    const link = 'index.html' + search;
    banner.innerHTML = '<div><p style="margin:0 0 12px;">' +
        (message || 'WebGPU is not available in this browser.') +
        '</p><a href="' + link + '" style="color:#4af;">Open WebGL view</a></div>';
    document.body.appendChild(banner);
    addBackendToggle(false);
}

function animate()
{
    requestAnimationFrame(animate);
    const scene = viewer.getScene();

    // Compute FPS and update overlay every 1/2 second.
    const now = performance.now();
    frameCount++;
    if (now - lastFrameTime >= 500) { 
        fps = Math.round((frameCount * 1000) / (now - lastFrameTime));
        if (fpsOverlay && showFPS) {
            fpsOverlay.innerText = `FPS: ${fps}`;
        }
        lastFrameTime = now;
        frameCount = 0;
    }

    if (turntableEnabled)
    {
        turntableStep = (turntableStep + 1) % 360;
        var turntableAngle = turntableStep * (360.0 / turntableSteps) / 180.0 * Math.PI;
        scene._scene.rotation.y = turntableAngle;
    }

    scene.updateTimeUniforms();
    renderer.render(scene.getScene(), scene.getCamera());

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
