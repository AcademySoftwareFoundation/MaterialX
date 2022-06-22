//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

import * as THREE from 'three';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader.js';

import { prepareEnvTexture, findLights, registerLights, getUniformValues } from './helper.js'
import { Group } from 'three';
import { GUI } from 'dat.gui';

const ALL_GEOMETRY_SPECIFIER = "*";
const NO_GEOMETRY_SPECIFIER = "";
const DAG_PATH_SEPERATOR = "/";

/*
    Scene management
*/
export class Scene 
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

        const aspectRatio = window.innerWidth / window.innerHeight;
        const cameraNearDist = 0.05;
        const cameraFarDist = 100.0;
        const cameraFOV = 60.0;
        this._camera = new THREE.PerspectiveCamera(cameraFOV, aspectRatio, cameraNearDist, cameraFarDist);

        this.#_gltfLoader = new GLTFLoader();

        this.#_normalMat = new THREE.Matrix3();
        this.#_viewProjMat = new THREE.Matrix4();
        this.#_worldViewPos = new THREE.Vector3();
    }

    // Set whether to flip UVs in V for loaded geometry
    setFlipGeometryV(val)
    {
        this.#_flipV = val;
    }

    // Get whether to flip UVs in V for loaded geometry
    getFlipGeometryV()
    {
        return this.#_flipV;
    }

    // Utility to perform geometry file load
    loadGeometryFile(geometryFilename, loader) 
    {
        return new Promise((resolve, reject) => {
            loader.load(geometryFilename, data => resolve(data), null, reject);
        });
    }

    //
    // Load in geometry specified by a given file name,
    // then update the scene geometry and camera.
    //
    async loadGeometry(viewer, orbitControls)
    {
        const gltfData = await this.loadGeometryFile(this.getGeometryURL(), this.#_gltfLoader);

        const scene = this.getScene(); 
        while (scene.children.length > 0) {
            scene.remove(scene.children[0]);
        }

        this.#_rootNode = null;
        const model = gltfData.scene;
        if (!model)
        {
            const geometry = new THREE.BoxGeometry(1, 1, 1);
            const material = new THREE.MeshBasicMaterial({ color: 0xdddddd });
            const cube = new THREE.Mesh(geometry, material);
            obj = new Group();
            obj.add(geometry);
        }
        else
        {
            this.#_rootNode = model;
        } 
        scene.add(model);

        // Always reset controls based on camera for each load. 
        orbitControls.reset();
        this.updateScene(viewer, orbitControls);
        viewer.getMaterial().updateMaterialAssignments(viewer);
    }

    //
    // Update the geometry buffer, assigned materials, and camera controls.
    //
    updateScene(viewer, orbitControls)
    {
        const bbox = new THREE.Box3().setFromObject(this._scene);
        const bsphere = new THREE.Sphere();
        bbox.getBoundingSphere(bsphere);

        let theScene = viewer.getScene();
        let flipV = theScene.getFlipGeometryV();
    
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
                else if (flipV)
                {
                    const uvCount = child.geometry.attributes.position.count;
                    const uvs = child.geometry.attributes.uv.array;
                    for (let i = 0; i < uvCount; i++) {
                        let v = 1.0-(uvs[i*2 +1]);
                        uvs[i*2+1] = v;
                    }
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
            }
        });
    
        // Update the background
        this._scene.background = this.getBackground();

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

    // Determine string DAG path based on individual node names.
    getDagPath(node)
    {
        const rootNode = this.#_rootNode;

        let path = [node.name];
        while (node.parent)
        {
            node = node.parent;
            if (node)
            {
                // Stop at the root of the scene read in.
                if (node == rootNode)
                {
                    break;
                }
                path.unshift(node.name);
            }
        }
        return path;
    }

    // Assign material shader to associated geometry
    updateMaterial(matassign)
    {
        let assigned = 0;

        const shader = matassign.getShader();
        const material = matassign.getMaterial().getName();
        const geometry = matassign.getGeometry();
        const collection = matassign.getCollection();

        const scene = this.getScene();
        const camera = this.getCamera();
        scene.traverse((child) => 
        {
            if (child.isMesh) 
            {
                const dagPath = this.getDagPath(child).join('/');

                // Note that this is a very simplistic
                // assignment resolve and assumes basic
                // regular expression name match.
                let matches = (geometry == ALL_GEOMETRY_SPECIFIER);
                if (!matches)
                {
                    if (collection)
                    {
                        if (collection.matchesGeomString(dagPath))
                        {
                            matches = true;
                        }
                    }
                    else
                    {
                        const paths = geometry.split(',');
                        for (let path of paths)
                        {
                            if (dagPath.match(path))
                            {
                                matches = true;
                                break;
                            }
                        }
                    }
                }
                if (matches)
                {
                    console.log('Assign material: ', material, ' to geometry: ', dagPath);
                    child.material = shader;
                    child.material.needsUpdate = true;
                    assigned++;
                }
            }
        });

        return assigned;
    }

    updateCamera()
    {
        const camera = this.getCamera();
        camera.aspect = window.innerWidth / window.innerHeight;
        camera.updateProjectionMatrix();
    }

    getScene() {
        return this._scene;
    }

    getCamera() {
        return this._camera;
    }

    getGeometryURL() {
        return this._geometryURL;
    }

    setGeometryURL(url) {
        this._geometryURL = url;
    }

    setBackgroundTexture(texture)
    {
        this.#_backgroundTexture = texture;
    }

    getShowBackgroundTexture()
    {
        return this.#_showBackgroundTexture;
    }

    setShowBackgroundTexture(enable)
    {
        this.#_showBackgroundTexture = enable;
    }

    getBackground() 
    {
        if (this.#_backgroundTexture && this.#_showBackgroundTexture)
        {
            return this.#_backgroundTexture;
        }
        var color = new THREE.Color(this.#_backgroundColor);
        color.convertSRGBToLinear();
        return color;
    }

    toggleBackgroundTexture()
    {
        this.#_showBackgroundTexture = !this.#_showBackgroundTexture;
        this._scene.background = this.getBackground();
    }

    // Geometry file
    #_geometryURL = '';
    // Geometry loader
    #_gltfLoader = null;
    // Flip V coordinate of texture coordinates.
    // Set to true to be consistent with desktop viewer.
    #_flipV = true;

    // Scene
    #_scene = null;

    // Camera
    #_camera = null;

    // Background color
    #_backgroundColor = 0x4c4c52;

    // Background texture
    #_backgroundTexture = null;
    #_showBackgroundTexture = true;

    // Transform matrices
    #_normalMat = new THREE.Matrix3();
    #_viewProjMat = new THREE.Matrix4();
    #_worldViewPos = new THREE.Vector3();

    // Root node of imported scene
    #_rootNode = null;
}

/* 
    Property editor
*/
export class Editor
{
    // Update ui properties
    // - Hide close button
    // - Update transparency so scene shows through if overlapping
    updateProperties(targetOpacity = 1) 
    {
        // Set opacity
        Array.from(document.getElementsByClassName('dg')).forEach(
            function (element, index, array) {
                element.style.opacity = targetOpacity;
            }
        );
    
        // Hide close button
        if (this._hideCloseControl)
        {
            Array.from(document.getElementsByClassName('close-button')).forEach(
                function (element, index, array) {
                    element.style.display = "none";
                }
            );
        }
    }

    //
    // Clear folders with children contain elements for any previous material
    // and recreate top gui.
    //
    clearFolders()
    {
        Array.from(document.getElementsByClassName('folder')).forEach(
            function (element, index, array) {
                if (element.className) {
                    let child = element.firstElementChild;
                    if (child && child.className == 'dg') {
                        element.remove();
                    }
                }
            }
        );

        // Create new GUI.
        this._gui = new GUI();
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
        this._gui.open();
        
        return this._gui;
    }
    
    getGUI() 
    {
        return this._gui;
    }

    _gui = null;
    _hideCloseControl = false;
}

class MaterialAssign
{
    constructor(material, geometry, collection)
    {
        this._material = material;
        this._geometry = geometry;
        this._collection = collection;
        this._shader = null;
    }

    setShader(shader)
    {
        this._shader = shader;
    }

    getShader()
    {
        return this._shader;
    }

    getMaterial()
    {
        return this._material;
    }
    
    getGeometry()
    {
        return this._geometry;
    }

    getCollection()
    {
        return this._collection;
    }

    // MaterialX material node name
    _material;

    // MaterialX assignment geometry string
    _geometry;

    // MaterialX assignment collection
    _collection;

    // THREE.JS shader
    _shader;
}

export class Material
{
    constructor()
    {
        this._materials = [];
        this._defaultMaterial = null;
    }

    // If no material file is selected, we programmatically create a default material as a fallback
    static createFallbackMaterial(doc) 
    {
        const ssName = 'SR_default';
        const ssNode = doc.addChildOfCategory('standard_surface', ssName);
        ssNode.setType('surfaceshader');
        const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
        smNode.setType('material');
        const shaderElement = smNode.addInput('surfaceshader');
        shaderElement.setType('surfaceshader');
        shaderElement.setNodeName(ssName);
    }

    async loadMaterialFile(loader, materialFilename)
    {
        return new Promise((resolve, reject) => {
            loader.load(materialFilename, data => resolve(data), null, reject);
        });
    }

    async loadMaterials(viewer, materialFilename)
    {
        const mx = viewer.getMx();

        // Re-initialize document
        var doc = mx.createDocument();
        doc.importLibrary(viewer.getLibrary());
        viewer.setDocument(doc);

        const fileloader = viewer.getFileLoader(); 

        let mtlxMaterial = await viewer.getMaterial().loadMaterialFile(fileloader, materialFilename);

        // Set search path. Assumes images are relative to current file
        // location.
        const paths = materialFilename.split('/');
        paths.pop(); 
        const searchPath = paths.join('/');

        // Load material
        if (mtlxMaterial)
            await mx.readFromXmlString(doc, mtlxMaterial, searchPath);
        else
            Material.createFallbackMaterial(doc);

        // Check if there are any looks defined in the document
        // If so then traverse the looks for all material assignments.
        // Generate code and compile for any associated surface shader
        // and assign to the associatged geometry. If there are no looks
        // then the first material is found and assignment to all the
        // geometry. 
        this._materials = [];
        this._defaultMaterial = null;
        var looks = doc.getLooks();
        if (looks.length)
        {
            for (let look of looks)
            {
                const materialAssigns = look.getMaterialAssigns();
                for (let materialAssign of materialAssigns) 
                {
                    let matName = materialAssign.getMaterial();
                    if (matName)
                    {   
                        let mat = doc.getChild(matName);
                        var shader;
                        if (mat)
                        {
                            var shaders = mx.getShaderNodes(mat);
                            if (shaders.length)
                            {
                                shader = shaders[0];
                            }
                        }
                        let collection = materialAssign.getCollection();
                        let geom = materialAssign.getGeom();
                        let newAssignment;
                        if (collection || geom)
                        {
                            newAssignment = new MaterialAssign(shader, geom, collection);
                        }
                        else
                        {
                            newAssignment = new MaterialAssign(shader, NO_GEOMETRY_SPECIFIER);
                        }

                        if (newAssignment)
                        {
                            this._materials.push(newAssignment);
                        }
                    }
                }
            }
        }
        else
        {
            // Search for any surface shader. It
            // is assumed to be assigned to the entire scene
            // The identifier used is "*" to mean the entire scene. 
            let elem = mx.findRenderableElement(doc);
            if (elem)
            {
                this._materials.push(new MaterialAssign(elem, ALL_GEOMETRY_SPECIFIER));
            }
        }
        
        // Load lighting setup into document
        doc.importLibrary(viewer.getLightRig());

        // Create a new shader for each material node.
        // Only create the shader once even if assigned more than once.
        let shaderMap = new Map();
        for (let matassign of this._materials)
        {
            let materialName = matassign.getMaterial().getName();
            let shader = shaderMap[materialName];
            if (!shader)
            {
                shader = viewer.getMaterial().generateMaterial(matassign.getMaterial(), viewer, searchPath);
                shaderMap[materialName] = shader;
            }
            matassign.setShader(shader);
        }

        // Update scene shader assignments
        this.updateMaterialAssignments(viewer);
    }

    //
    // Update the assignments for scene objects based on the
    // material assignment information stored in the viewer.
    // Note: If none of the MaterialX assignments match the geometry
    // in the scene, then the first material assignment shader is assigned
    // to the entire scene.
    //
    updateMaterialAssignments(viewer)
    {
        let assigned = 0;
        for (let matassign of this._materials)
        {
            if (matassign.getShader())
            {
                assigned += viewer.getScene().updateMaterial(matassign);
            }
        }
        if (assigned == 0 && this._materials.length)
        {
            this._defaultMaterial = new MaterialAssign(this._materials[0].getMaterial(), ALL_GEOMETRY_SPECIFIER);
            this._defaultMaterial.setShader(this._materials[0].getShader());
            viewer.getScene().updateMaterial(this._defaultMaterial);
        }
    }

    // 
    // Generate a new material for a given element
    //
    generateMaterial(elem, viewer, searchPath) 
    {
        const mx = viewer.getMx();
        const textureLoader = new THREE.TextureLoader();

        const lights = viewer.getLights();
        const lightData = viewer.getLightData();
        const radianceTexture = viewer.getRadianceTexture();
        const irradianceTexture = viewer.getIrradianceTexture();
        const gen = viewer.getGenerator();
        const genContext = viewer.getGenContext();

        // Perform transparency check on renderable item
        const isTransparent = mx.isTransparentSurface(elem, gen.getTarget());
        genContext.getOptions().hwTransparency = isTransparent;

        // Generate GLES code
        let shader = gen.generate(elem.getNamePath(), elem, genContext);

        // Get shaders and uniform values
        let vShader = shader.getSourceCode("vertex");
        let fShader = shader.getSourceCode("pixel");

        let theScene = viewer.getScene();
        let flipV = theScene.getFlipGeometryV();
        let uniforms = {
            ...getUniformValues(shader.getStage('vertex'), textureLoader, searchPath, flipV),
            ...getUniformValues(shader.getStage('pixel'), textureLoader, searchPath, flipV),
        }

        Object.assign(uniforms, {
            u_numActiveLightSources: { value: lights.length },
            u_lightData: { value: lightData },
            u_envMatrix: { value: new THREE.Matrix4().makeRotationY(Math.PI) },
            u_envRadiance: { value: radianceTexture },
            u_envRadianceMips: { value: Math.trunc(Math.log2(Math.max(radianceTexture.image.width, radianceTexture.image.height))) + 1 },
            u_envRadianceSamples: { value: 16 },
            u_envIrradiance: { value: irradianceTexture },
            u_refractionEnv: { value: true }
        });

        // Create Three JS Material
        let newMaterial  = new THREE.RawShaderMaterial({
            uniforms: uniforms,
            vertexShader: vShader,
            fragmentShader: fShader,
            transparent: isTransparent,
            blendEquation: THREE.AddEquation,
            blendSrc: THREE.OneMinusSrcAlphaFactor,
            blendDst: THREE.SrcAlphaFactor,
            side: THREE.DoubleSide
        });

        // Update property editor
        const gui = viewer.getEditor().getGUI();
        this.updateEditor(elem, shader, newMaterial, gui);

        return newMaterial;
    }

    //
    // Update property editor for a given MaterialX element, it's shader, and
    // Three material
    //
    updateEditor(elem, shader, material, gui)
    {
        const elemPath = elem.getNamePath();
        var matUI = gui.addFolder(elemPath + ' Properties');
        const uniformBlocks = Object.values(shader.getStage('pixel').getUniformBlocks());
        var uniformToUpdate;
        const ignoreList = ['u_envRadianceMips', 'u_envRadianceSamples', 'u_alphaThreshold'];

        var folderList = new Map();
        folderList[elemPath] = matUI;

        uniformBlocks.forEach(uniforms => 
        {
            if (!uniforms.empty()) 
            {
                for (let i = 0; i < uniforms.size(); ++i) 
                {
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
                        const graph = currentNode.getParent();
                        if (graph)
                        {
                            const graphInput = graph.getInput(interfaceName);
                            if (graphInput) {
                                let uiname = graphInput.getAttribute('uiname');
                                if (uiname.length) {
                                    path = uiname;
                                }
                                else {
                                    path = graphInput.getName();
                                }
                            }
                        }
                        else
                        {
                            path = interfaceName;
                        }
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
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null) {
                                currentFolder.add(material.uniforms[name], 'value').name(path);
                            }
                            break;

                        case 'integer':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null) {
                                currentFolder.add(material.uniforms[name], 'value').name(path);
                            }
                            break;

                        case 'boolean':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null) {
                                currentFolder.add(material.uniforms[name], 'value').name(path);
                            }
                            break;

                        case 'vector2':
                        case 'vector3':
                        case 'vector4':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null) {
                                let vecFolder = currentFolder.addFolder(path);
                                Object.keys(material.uniforms[name].value).forEach((key) => {
                                    vecFolder.add(material.uniforms[name].value, key).name(path + "." + key);
                                })
                            }
                            break;

                        case 'color3':
                            // Irksome way to mape arrays to colors and back
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null) {
                                var dummy = {
                                    color: 0xFF0000
                                };
                                const color3 = new THREE.Color(dummy.color);
                                color3.fromArray(material.uniforms[name].value);
                                dummy.color = color3.getHex();
                                currentFolder.addColor(dummy, 'color').name(path)
                                    .onChange(function (value) {
                                        const color3 = new THREE.Color(value);
                                        material.uniforms[name].value.set(color3.toArray());
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
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null) {
                                item = currentFolder.add(material.uniforms[name], 'value');
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
    }

    // List of material assignments: { MaterialX node, geometry assignment string, and hardware shader }
    _materials;

    // Fallback material if nothing was assigned explicitly
    _defaultMaterial;
}

/*
    Viewer class

    Keeps track of local scene, and property editor as well as current MaterialX document 
    and assocaited material, shader and lighting information.
*/
export class Viewer 
{
    static create()
    {
        return new Viewer();
    }

    constructor()
    {
        this.scene = new Scene();
        this.editor = new Editor();
        this.materials.push(new Material());

        this.fileLoader = new THREE.FileLoader();
        this.hdrLoader = new RGBELoader();    
    }

    //
    // Create shader generator, generation context and "base" document which
    // contains the standard definition libraries and lighting elements.
    //
    async initialize(mtlxIn, renderer, loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture)
    {
        this.mx = mtlxIn;

        // Initialize base document
        this.generator = new this.mx.EsslShaderGenerator();
        this.genContext = new this.mx.GenContext(this.generator);

        this.document = this.mx.createDocument();
        this.stdlib = this.mx.loadStandardLibraries(this.genContext);
        this.document.importLibrary(this.stdlib);

        this.initializeLighting(renderer, loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture);

        loadedRadianceTexture.mapping = THREE.EquirectangularReflectionMapping;
        this.getScene().setBackgroundTexture(loadedRadianceTexture);
    }

    //
    // Load in lighting rig document and register lights with generation context
    // Initialize environment lighting (IBLs).
    //
    async initializeLighting(renderer, loadedRadianceTexture, loadedLightSetup, loadedIrradianceTexture)
    {
        // Load lighting setup into document
        const mx = this.getMx();
        this.lightRigDoc = mx.createDocument();
        await mx.readFromXmlString(this.lightRigDoc, loadedLightSetup);
        this.document.importLibrary(this.lightRigDoc);

        // Register lights with generation context
        this.lights = findLights(this.document);
        this.lightData = registerLights(mx, this.lights, this.genContext);

        this.radianceTexture = prepareEnvTexture(loadedRadianceTexture, renderer.capabilities);
        this.irradianceTexture = prepareEnvTexture(loadedIrradianceTexture, renderer.capabilities);
    }

    getEditor() {
        return this.editor;
    }

    getScene() {
        return this.scene;
    }

    getMaterial() {
        return this.materials[0];
    }

    getFileLoader()
    {
        return this.fileLoader;
    }

    getHdrLoader()
    {
        return this.hdrLoader;
    }

    setDocument(doc) {
        this.doc = doc;
    }
    getDocument() {
        return this.doc;
    }

    getLibrary() {
        return this.stdlib;
    }

    getLightRig() {
        return this.lightRigDoc;
    }

    getMx() {
        return this.mx;
    }

    getGenerator() {
        return this.generator;
    }

    getGenContext() {
        return this.genContext;
    }

    getLights() {
        return this.lights;
    }

    getLightData() {
        return this.lightData;
    }

    getRadianceTexture() {
        return this.radianceTexture;
    }

    getIrradianceTexture() {
        return this.irradianceTexture;
    }

    // Three scene and materials. 
    scene = null;
    materials = [];

    // Property editor
    editor = null;

    // Utility loaders
    fileloader = null;
    hdrLoader = null;

    // MaterialX module, current document and support documents.
    mx = null;
    doc = null;
    stdlib = null;
    lightRigDoc = null;

    // MaterialX code generator and context
    generator = null;
    genContext = null;

    // Lighting information
    lights = null;
    lightData = null;
    radianceTexture = null;
    irradianceTexture = null;
}
