//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

import * as THREE from 'three';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass.js';

import { GammaCorrectionShader } from 'three/examples/jsm/shaders/GammaCorrectionShader.js';

import { prepareEnvTexture, findLights, registerLights, getUniformValues } from './helper.js'
import { Group } from 'three';
import { GUI } from 'dat.gui';

let  renderer, composer, orbitControls, mx, currentMaterial;

// Get URL options. Fallback to defaults if not specified.
let materialFilename = new URLSearchParams(document.location.search).get("file");
if (!materialFilename) {
    materialFilename = 'Materials/Examples/StandardSurface/standard_surface_default.mtlx';
}

/*
    Scene management
*/
class Scene 
{
    constructor() 
    {
        this._geometryURL = new URLSearchParams(document.location.search).get("geom");
        if (!this._geometryURL)
        {
            this._geometryURL = 'Geometry/shaderball.glb'; 
        }
    }

    initialize() 
    {
        this._scene = new THREE.Scene();
        this._scene.background = new THREE.Color(this.#_backgroundColor);
        this._scene.background.convertSRGBToLinear();

        this._camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.001, 1000);
        this._camera.far = 5000.0;

        this.#_gltfLoader = new GLTFLoader();

        this.#_normalMat = new THREE.Matrix3();
        this.#_viewProjMat = new THREE.Matrix4();
        this.#_worldViewPos = new THREE.Vector3();
    }

    /* Utility to perform load */
    loadModel(geometryFilename, loader) 
    {
        return new Promise((resolve, reject) => {
            loader.load(geometryFilename, data => resolve(data), null, reject);
        });
    }

    /*
        Load in geometry specified by a given file name,
        then update the scene geometry and camera.
    */
    async loadGeometry()
    {
        const gltfData = await this.loadModel(this.getGeometryURL(), this.#_gltfLoader);

        const scene = this.getScene(); 
        while (scene.children.length > 0) {
            scene.remove(scene.children[0]);
        }

        const model = gltfData.scene;
        if (!model)
        {
            const geometry = new THREE.BoxGeometry(1, 1, 1);
            const material = new THREE.MeshBasicMaterial({ color: 0xdddddd });
            const cube = new THREE.Mesh(geometry, material);
            obj = new Group();
            obj.add(geometry);
        } 
        scene.add(model);

        // Always reset controls based on camera for each load. 
        orbitControls.reset();
        this.updateScene();
    }

    /*
        Update the geometry buffer, assigned materials, and camera controls.
    */
    updateScene()
    {
        const bbox = new THREE.Box3().setFromObject(this._scene);
        const bsphere = new THREE.Sphere();
        bbox.getBoundingSphere(bsphere);
    
        this._scene.traverse((child) => {
            if (child.isMesh) {
                if (!child.geometry.attributes.uv) {
                    const posCount = child.geometry.attributes.position.count;
                    const uvs = [];
                    const pos = child.geometry.attributes.position.array;
    
                    for (let i = 0; i < posCount; i++) {
                        uvs.push((pos[i * 3] - bsphere.center.x) / bsphere.radius);
                        uvs.push((pos[i * 3 + 1] - bsphere.center.y) / bsphere.radius);
                    }
    
                    child.geometry.setAttribute('uv', new THREE.BufferAttribute(new Float32Array(uvs), 2));
                }
    
                if (!child.geometry.attributes.normal) {
                    child.geometry.computeVertexNormals();
                }
    
                if (child.geometry.getIndex()) {
                    child.geometry.computeTangents();
                }
    
                // Use default MaterialX naming convention.
                child.geometry.attributes.i_position = child.geometry.attributes.position;
                child.geometry.attributes.i_normal = child.geometry.attributes.normal;
                child.geometry.attributes.i_tangent = child.geometry.attributes.tangent;
                child.geometry.attributes.i_texcoord_0 = child.geometry.attributes.uv;
    
                child.material = currentMaterial;
            }
        });
    
        // Fit camera to model
        const camera = this.getCamera();
        camera.position.y = bsphere.center.y;
        camera.position.z = bsphere.radius * 2.0;
        camera.updateProjectionMatrix();
    
        orbitControls.target = bsphere.center;
        orbitControls.update();
    }    

    updateTransforms()
    {
        const scene = this.getScene();
        const camera = this.getCamera();
        scene.traverse((child) => {
            if (child.isMesh) {
                const uniforms = child.material.uniforms;
                if (uniforms) {
                    uniforms.u_worldMatrix.value = child.matrixWorld;
                    uniforms.u_viewProjectionMatrix.value = this.#_viewProjMat.multiplyMatrices(camera.projectionMatrix, camera.matrixWorldInverse);

                    if (uniforms.u_viewPosition)
                        uniforms.u_viewPosition.value = camera.getWorldPosition(this.#_worldViewPos);

                    if (uniforms.u_worldInverseTransposeMatrix)
                        uniforms.u_worldInverseTransposeMatrix.value = 
                        new THREE.Matrix4().setFromMatrix3(this.#_normalMat.getNormalMatrix(child.matrixWorld));
                }
            }
        });
    }

    getScene() {
        return this._scene;
    }

    getCamera() {
        return this._camera;
    }

    getGeometryURL() {
        console.log("Get geometry URL: ", this._geometryURL);
        return this._geometryURL;
    }

    setGeometryURL(url) {
        this._geometryURL = url;
    }

    // Geometry file
    #_geometryURL = '';
    // Geometry loader
    #_gltfLoader = null;

    // Scene
    #_scene = null;

    // Camera
    #_camera = null;

    // Background color
    #_backgroundColor = 0x4c4c52;

    // Transform matrices
    #_normalMat = new THREE.Matrix3();
    #_viewProjMat = new THREE.Matrix4();
    #_worldViewPos = new THREE.Vector3();
}


class Editor
{
    // Update ui properties
    updateProperties(targetOpacity = 1) 
    {
        // Set opacity
        Array.from(document.getElementsByClassName('dg')).forEach(
            function (element, index, array) {
                element.style.opacity = targetOpacity;
            }
        );
    
        // Hide close button
        Array.from(document.getElementsByClassName('close-button')).forEach(
            function (element, index, array) {
                element.style.display = "none";
            }
        );
    }
    
    // Create the editor
    initialize() 
    {
        // Search document to find GUI elements and remove them
        // If not done then multiple GUIs will be created from different
        // threads.
        Array.from(document.getElementsByClassName('dg')).forEach(
            function (element, index, array) {
                if (element.className) {
                    element.remove();
                }
            }
        );
    
        // Create new GUI. 
        this._gui = new GUI();
        return this._gui;
    }
    
    getGUI() 
    {
        return this._gui;
    }

    _gui = null;
}

let theScene = new Scene();
let theEditor = new Editor();

init();
theEditor.updateProperties(0.9);

// If no material file is selected, we programmatically create a default material as a fallback
function fallbackMaterial(doc) {
    const ssName = 'SR_default';
    const ssNode = doc.addChildOfCategory('standard_surface', ssName);
    ssNode.setType('surfaceshader');
    const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
    smNode.setType('material');
    const shaderElement = smNode.addInput('surfaceshader');
    shaderElement.setType('surfaceshader');
    shaderElement.setNodeName(ssName);
}

function generateMaterial(elem, gen, genContext, lights, lightData,
                         textureLoader, radianceTexture, irradianceTexture) 
{
    // Perform transparency check on renderable item
    const isTransparent = mx.isTransparentSurface(elem, gen.getTarget());
    genContext.getOptions().hwTransparency = isTransparent;

    // Generate GLES code
    let shader = gen.generate(elem.getNamePath(), elem, genContext);

    // Get shaders and uniform values
    let vShader = shader.getSourceCode("vertex");
    let fShader = shader.getSourceCode("pixel");

    let uniforms = {
        ...getUniformValues(shader.getStage('vertex'), textureLoader),
        ...getUniformValues(shader.getStage('pixel'), textureLoader),
    }

    Object.assign(uniforms, {
        u_numActiveLightSources: { value: lights.length },
        u_lightData: { value: lightData },
        u_envMatrix: { value: new THREE.Matrix4().makeRotationY(Math.PI) },
        u_envRadiance: { value: radianceTexture },
        u_envRadianceMips: { value: Math.trunc(Math.log2(Math.max(radianceTexture.image.width, radianceTexture.image.height))) + 1 },
        u_envRadianceSamples: { value: 16 },
        u_envIrradiance: { value: irradianceTexture }
    });

    // Create Three JS Material
    const newMaterial = new THREE.RawShaderMaterial({
        uniforms: uniforms,
        vertexShader: vShader,
        fragmentShader: fShader,
        transparent: isTransparent,
        blendEquation: THREE.AddEquation,
        blendSrc: THREE.OneMinusSrcAlphaFactor,
        blendDst: THREE.SrcAlphaFactor
    });
    newMaterial.side = THREE.DoubleSide;

    const elemPath = elem.getNamePath();
    const gui = theEditor.getGUI();
    var matUI = gui.addFolder(elemPath + ' Properties');
    const uniformBlocks = Object.values(shader.getStage('pixel').getUniformBlocks());
    var uniformToUpdate;
    const ignoreList = ['u_envRadianceMips', 'u_envRadianceSamples', 'u_alphaThreshold'];

    var folderList = new Map();
    folderList[elemPath] = matUI;

    uniformBlocks.forEach(uniforms => {
        if (!uniforms.empty()) {

            for (let i = 0; i < uniforms.size(); ++i) {
                const variable = uniforms.get(i);
                const value = variable.getValue()?.getData();
                let name = variable.getVariable();

                if (ignoreList.includes(name)) {
                    continue;
                }

                let currentFolder = matUI;
                let currentElemPath = variable.getPath();
                if (!currentElemPath || currentElemPath.length == 0) {
                    continue;
                }
                let currentElem = elem.getDocument().getDescendant(currentElemPath);
                if (!currentElem) {
                    continue;
                }

                let currentNode = currentElem ? currentElem.getParent() : null;
                let uiname;
                if (currentNode) {

                    let currentNodePath = currentNode.getNamePath();
                    var pathSplit = currentNodePath.split('/');
                    if (pathSplit.length) {
                        currentNodePath = pathSplit[0];
                    }
                    currentFolder = folderList[currentNodePath];
                    if (!currentFolder) {
                        currentFolder = matUI.addFolder(currentNodePath);
                        folderList[currentNodePath] = currentFolder;
                    }

                    // Check for ui attributes
                    var nodeDef = currentNode.getNodeDef();
                    if (nodeDef) {
                        let input = nodeDef.getActiveInput(name);
                        if (input) {
                            uiname = input.getAttribute('uiname');
                            let uifolderName = input.getAttribute('uifolder');
                            if (uifolderName && uifolderName.length) {
                                let newFolderName = currentNodePath + '/' + uifolderName;
                                currentFolder = folderList[newFolderName];
                                if (!currentFolder) {
                                    currentFolder = matUI.addFolder(uifolderName);
                                    folderList[newFolderName] = currentFolder;
                                }
                            }
                        }
                    }
                }

                // Determine UI name to use
                let path = name;
                let interfaceName = currentElem.getAttribute("interfacename");
                if (interfaceName && interfaceName.length) {
                    path = interfaceName;
                }
                else {
                    if (!uiname) {
                        uiname = currentElem.getAttribute('uiname');
                    }
                    if (uiname && uiname.length) {
                        path = uiname;
                    }
                }

                switch (variable.getType().getName()) {

                    case 'float':
                        uniformToUpdate = newMaterial.uniforms[name];
                        if (uniformToUpdate && value != null) {
                            currentFolder.add(newMaterial.uniforms[name], 'value').name(path);
                        }
                        break;

                    case 'integer':
                        uniformToUpdate = newMaterial.uniforms[name];
                        if (uniformToUpdate && value != null) {
                            currentFolder.add(newMaterial.uniforms[name], 'value').name(path);
                        }
                        break;

                    case 'boolean':
                        uniformToUpdate = newMaterial.uniforms[name];
                        if (uniformToUpdate && value != null) {
                            currentFolder.add(newMaterial.uniforms[name], 'value').name(path);
                        }
                        break;

                    case 'vector2':
                    case 'vector3':
                    case 'vector4':
                        uniformToUpdate = newMaterial.uniforms[name];
                        if (uniformToUpdate && value != null) {
                            let vecFolder = currentFolder.addFolder(path);
                            Object.keys(newMaterial.uniforms[name].value).forEach((key) => {
                                vecFolder.add(newMaterial.uniforms[name].value, key).name(path + "." + key);
                            })
                        }
                        break;

                    case 'color3':
                        // Irksome way to mape arrays to colors and back
                        uniformToUpdate = newMaterial.uniforms[name];
                        if (uniformToUpdate && value != null) {
                            var dummy = {
                                color: 0xFF0000
                            };
                            const color3 = new THREE.Color(dummy.color);
                            color3.fromArray(newMaterial.uniforms[name].value);
                            dummy.color = color3.getHex();
                            currentFolder.addColor(dummy, 'color').name(path)
                                .onChange(function (value) {
                                    const color3 = new THREE.Color(value);
                                    newMaterial.uniforms[name].value.set(color3.toArray());
                                }
                                );
                        }
                        break;

                    case 'color4':
                        break;

                    case 'matrix33':
                    case 'matrix44':
                    case 'samplerCube':
                    case 'filename':
                        break;
                    case 'string':
                        uniformToUpdate = newMaterial.uniforms[name];
                        if (uniformToUpdate && value != null) {
                            item = currentFolder.add(newMaterial.uniforms[name], 'value');
                            item.name(path);
                            item.readonly(true);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    });

    return newMaterial;
}

function init() 
{
    let canvas = document.getElementById('webglcanvas');
    let context = canvas.getContext('webgl2');

    // Material selection
    let materialsSelect = document.getElementById('materials');
    materialsSelect.value = materialFilename;
    materialsSelect.addEventListener('change', (e) => {
        materialFilename = e.target.value;
        window.location.href =
            `${window.location.origin}${window.location.pathname}?file=${materialFilename}`;
    });

    // Geometry selection
    let geometrySelect = document.getElementById('geometry');
    geometrySelect.value = theScene.getGeometryURL();
    geometrySelect.addEventListener('change', (e) => {
        theScene.setGeometryURL(e.target.value);
        theScene.loadGeometry();
    });

    // Set up scene
    theScene.initialize();

    // Set up renderer
    renderer = new THREE.WebGLRenderer({ canvas, context });
    renderer.setSize(window.innerWidth, window.innerHeight);

    composer = new EffectComposer(renderer);
    const renderPass = new RenderPass(theScene.getScene(), theScene.getCamera());
    composer.addPass(renderPass);
    const gammaCorrectionPass = new ShaderPass(GammaCorrectionShader);
    composer.addPass(gammaCorrectionPass);

    window.addEventListener('resize', onWindowResize);

    // Set up controls
    orbitControls = new OrbitControls(theScene.getCamera(), renderer.domElement);

    // Load model and shaders
    const fileloader = new THREE.FileLoader();
    const hdrloader = new RGBELoader();
    const textureLoader = new THREE.TextureLoader();

    // Initialize editor
    theEditor.initialize();

    Promise.all([
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => fileloader.load('Lights/san_giuseppe_bridge_split.mtlx', resolve)),
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(function (resolve) {
            MaterialX().then((module) => {
                resolve(module);
            });
        }),
        new Promise(resolve => materialFilename ? fileloader.load(materialFilename, resolve) : resolve())

    ]).then(async ([loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture, mxIn, mtlxMaterial]) => {

        // Initialize MaterialX and the shader generation context
        mx = mxIn;
        let doc = mx.createDocument();
        let gen = new mx.EsslShaderGenerator();
        let genContext = new mx.GenContext(gen);
        let stdlib = mx.loadStandardLibraries(genContext);
        doc.importLibrary(stdlib);

        // Set search path.
        const searchPath = 'Materials/Examples/StandardSurface/';

        // Load material
        if (mtlxMaterial)
            await mx.readFromXmlString(doc, mtlxMaterial, searchPath);
        else
            fallbackMaterial(doc);

        // Search for any renderable items
        let elem = mx.findRenderableElement(doc);

        // Load lighting setup into document
        const lightRigDoc = mx.createDocument();
        await mx.readFromXmlString(lightRigDoc, loadedLightSetup);
        doc.importLibrary(lightRigDoc);

        // Register lights with generation context
        const lights = findLights(doc);
        const lightData = registerLights(mx, lights, genContext);

        const radianceTexture = prepareEnvTexture(loadedRadianceTexture, renderer.capabilities);
        const irradianceTexture = prepareEnvTexture(loadedIrradianceTexture, renderer.capabilities);

        currentMaterial = generateMaterial(elem, gen, genContext, lights, lightData,
            textureLoader, radianceTexture, irradianceTexture);

        // Different on initial load is that new a camera is initialized
        theScene.loadGeometry();

    }).then(() => {
        animate();
    }).catch(err => {
        console.error(Number.isInteger(err) ? mx.getExceptionMessage(err) : err);
    })

}

function onWindowResize() 
{
    const camera = theScene.getCamera();
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() 
{
    requestAnimationFrame(animate);

    composer.render();

    theScene.updateTransforms();
}
