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
import { color } from 'dat.gui';

let camera, scene, model, renderer, composer, controls, mx;

let normalMat = new THREE.Matrix3();
let viewProjMat = new THREE.Matrix4();
let worldViewPos = new THREE.Vector3();

var gui;

// Get URL options. Fallback to defaults if not specified.
let materialFilename = new URLSearchParams(document.location.search).get("file");
if (!materialFilename) {
  materialFilename = 'Materials/Examples/StandardSurface/standard_surface_default.mtlx';
}
let geometryFilename = new URLSearchParams(document.location.search).get("geom");
if (!geometryFilename) {
  geometryFilename = 'Geometry/shaderball.glb';
}

init();

// If no material file is selected, we programmatically create a default material as a fallback
function fallbackMaterial(doc)
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

function loadGeometry(scene, gltfLoader, filename, resolve)
{
  // Clear the scene first
  while (scene.children.length > 0) {
    scene.remove(scene.children[0]);
  }

  gltfLoader.load(filename, resolve); 
}

function changeGuiOpacity(targetOpacity=1){
	Array.from(document.getElementsByClassName('dg')).forEach(
    function(element, index, array) {
		element.style.opacity = targetOpacity;
    }
	);
}

function generateMaterial(elem, gen, genContext, lights, lightData, 
  textureLoader, radianceTexture, irradianceTexture)
{  
  // Perform transparency check on renderable item
  const isTransparent = mx.isTransparentSurface(elem, gen.getTarget());
  genContext.getOptions().hwTransparency = isTransparent;

  // Generate GLES code
  console.log('Generate shader');
  let shader = gen.generate(elem.getNamePath(), elem, genContext);

  // Get shaders and uniform values
  let vShader = shader.getSourceCode("vertex");
  let fShader = shader.getSourceCode("pixel");

  let uniforms = {
    ...getUniformValues(shader.getStage('vertex'), textureLoader),
    ...getUniformValues(shader.getStage('pixel'), textureLoader),
  }

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
  
  console.log(threeMaterial.uniforms);

  gui = new GUI();
  var matUI = gui.addFolder('Material');
  const uniformBlocks = Object.values(shader.getStage('pixel').getUniformBlocks());
  let uniformToUpdate;
  uniformBlocks.forEach(uniforms => {
    console.log(uniforms);  
    if (!uniforms.empty()) {
      for (let i = 0; i < uniforms.size(); ++i) {
        const variable = uniforms.get(i);
        const value = variable.getValue()?.getData();
        const name = variable.getVariable();
        console.log("Scan uniform: " + name + ". Type: " + variable.getType().getName() + ". Value: " + value);
        switch (variable.getType().getName()) {
          
          case 'float':
          case 'integer':
            uniformToUpdate = threeMaterial.uniforms[name];
            if (uniformToUpdate && value) {
              matUI.add(threeMaterial.uniforms[name], 'value', 0).name(name);
            }
            break;

          case 'boolean':
            uniformToUpdate = threeMaterial.uniforms[name];
            if (uniformToUpdate && value) {
              matUI.add(threeMaterial.uniforms[name], 'value').name(name);
            }
            break;

          case 'vector2':
          case 'vector3':
          case 'vector4':
            uniformToUpdate = threeMaterial.uniforms[name];
            if (uniformToUpdate && value) {
              let vecFolder = matUI.addFolder(name);
              Object.keys(threeMaterial.uniforms[name].value).forEach((key) => {
                vecFolder.add(threeMaterial.uniforms[name].value, key, 0.0).name(name + "." + key);
              })
            }
            break;

          case 'color3':
            // Irksome way to mape arrays to colors and back
            uniformToUpdate = threeMaterial.uniforms[name];
            if (uniformToUpdate && value) {
              var dummy = {
                color: 0xFF0000
              };
              const color3 = new THREE.Color(dummy.color);
              color3.fromArray(threeMaterial.uniforms[name].value);
              dummy.color = color3.getHex();
              matUI.addColor(dummy, 'color').name(name)
                .onChange(function (value) {
                  const color3 = new THREE.Color(value);
                  threeMaterial.uniforms[name].value.set(color3.toArray());
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
            uniformToUpdate = threeMaterial.uniforms[name];
            if (uniformToUpdate && value) {
              item = matUI.add(threeMaterial.uniforms[name], 'value');
              item.name(name);
              item.readonly(true);
            }            
            break;
          default:
            console.log("SKIP: " + variable.getType().getName());
            break;
        }
      }
    }
  });  
  
  changeGuiOpacity(0.9);

  return threeMaterial;
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
      `${window.location.origin}${window.location.pathname}?file=${materialFilename}&geom=${geometryFilename}`;
    });

    // Geometry selection
    let geometrySelect = document.getElementById('geometry');
    geometrySelect.value = geometryFilename;
    geometrySelect.addEventListener('change', (e) => {
      geometryFilename = e.target.value;
      window.location.href = 
      `${window.location.origin}${window.location.pathname}?file=${materialFilename}&geom=${geometryFilename}`;
    });

    camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 1, 100);

    // Set up scene
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x4c4c52);
    scene.background.convertSRGBToLinear();

    // Set up renderer
    renderer = new THREE.WebGLRenderer({canvas, context});
    renderer.setSize(window.innerWidth, window.innerHeight);

    composer = new EffectComposer( renderer );
    const renderPass = new RenderPass( scene, camera );
    composer.addPass( renderPass );
    const gammaCorrectionPass = new ShaderPass( GammaCorrectionShader );
    composer.addPass( gammaCorrectionPass );

    window.addEventListener('resize', onWindowResize);

    // Set up controls
    controls = new OrbitControls(camera, renderer.domElement);

    // Load model and shaders
    const fileloader = new THREE.FileLoader();
    const gltfLoader = new GLTFLoader();
    const hdrloader = new RGBELoader();
    const textureLoader = new THREE.TextureLoader();

    const geometryFile = 'Geometry/boombox.glb';
    Promise.all([
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => fileloader.load('Lights/san_giuseppe_bridge_split.mtlx', resolve)),
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => loadGeometry(scene, gltfLoader, geometryFilename, resolve)),
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

      const threeMaterial = generateMaterial(elem, gen, genContext, lights, lightData, 
        textureLoader, radianceTexture, irradianceTexture);

        if (!obj) {
            const geometry = new THREE.BoxGeometry(1, 1, 1);
            const material = new THREE.MeshBasicMaterial({ color: 0xdddddd });
            const cube = new THREE.Mesh(geometry, material);
            obj = new Group();
            obj.add(geometry);
        }

        model = obj;
        scene.add(model);

        const bbox = new THREE.Box3().setFromObject(model);
        const bsphere = new THREE.Sphere();
        bbox.getBoundingSphere(bsphere);

        model.traverse((child) => {
            if (child.isMesh)
            {
              if (!child.geometry.attributes.uv)
              {
                const posCount = child.geometry.attributes.position.count; 
                const uvs = [];
                const pos = child.geometry.attributes.position.array;
      
                for (let i = 0; i < posCount ; i++) {
                  uvs.push( (pos[i*3] - bsphere.center.x ) /  bsphere.radius);
                  uvs.push( (pos[i*3+1] - bsphere.center.y) / bsphere.radius);
                }
                
                child.geometry.setAttribute('uv', new THREE.BufferAttribute(new Float32Array(uvs), 2));                
              }

              if (!child.geometry.attributes.normal)
              {
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

              child.material = threeMaterial;
            }
        });

        // Fit camera to model
        controls.target = bsphere.center;
        camera.position.y = bsphere.center.y;
        camera.position.z = bsphere.radius * 2.0;
        controls.update();

        camera.far = 5000.0;
        camera.updateProjectionMatrix();

        //
/*
        gui.add( material.uniforms.nearClipping, 'value', 1, 10000, 1.0 ).name( 'nearClipping' );
				gui.add( material.uniforms.farClipping, 'value', 1, 10000, 1.0 ).name( 'farClipping' );
				gui.add( material.uniforms.pointSize, 'value', 1, 10, 1.0 ).name( 'pointSize' );
				gui.add( material.uniforms.zOffset, 'value', 0, 4000, 1.0 ).name( 'zOffset' );
*/
		        
        //matUI.add(threeMaterial, 'transparent').name('Transparent').listen();
        //matUI.open(); 

    }).then(() => {
        animate();
    }).catch(err => {
        console.error(Number.isInteger(err) ? mx.getExceptionMessage(err) : err);
    }) 

}

function onWindowResize()
{
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate()
{
    requestAnimationFrame(animate);

    composer.render();

    model.traverse((child) => {
      if (child.isMesh) 
      {
        const uniforms = child.material.uniforms;
        if (uniforms) 
        {
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
