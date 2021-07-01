import * as THREE from 'three';
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass.js';
import { GammaCorrectionShader } from 'three/examples/jsm/shaders/GammaCorrectionShader.js';

let camera, scene, model, renderer, composer, controls;

let normalMat = new THREE.Matrix3();
let viewProjMat = new THREE.Matrix4();
let worldViewPos = new THREE.Vector3();

init();

/**
 * Adds a tangent BufferAttribute to the passed in geometry.
 * See MaterialXRender/Mesh.cpp
 * @param {THREE.BufferGeometry} geometry 
 */
function generateTangents(geometry) {
    let p0 = new THREE.Vector3();
    let p1 = new THREE.Vector3();
    let p2 = new THREE.Vector3();

    let n0 = new THREE.Vector3();
    let n1 = new THREE.Vector3();
    let n2 = new THREE.Vector3();

    let w0 = new THREE.Vector2();
    let w1 = new THREE.Vector2();
    let w2 = new THREE.Vector2();

    let e1 = new THREE.Vector3();
    let e2 = new THREE.Vector3();
    
    let tangent = new THREE.Vector3();
    let t0 = new THREE.Vector3();
    let t1 = new THREE.Vector3();
    let t2 = new THREE.Vector3();

    const positions = geometry.attributes.position;
    const normals = geometry.attributes.normal;
    const uvs = geometry.attributes.uv;
    const length = positions.count * positions.itemSize;

    const tangentsdata = new Float32Array(length);

    for(let i = 0; i < positions.count; i += 3) {
        const idx = i * positions.itemSize;
        const uvidx = i * uvs.itemSize;

        p0.set(positions.array[idx], positions.array[idx + 1], positions.array[idx + 2]);
        p1.set(positions.array[idx + 3], positions.array[idx + 4], positions.array[idx + 5]);
        p2.set(positions.array[idx + 6], positions.array[idx + 7], positions.array[idx + 8]);

        n0.set(normals.array[idx], normals.array[idx + 1], normals.array[idx + 2]);
        n1.set(normals.array[idx + 3], normals.array[idx + 4], normals.array[idx + 5]);
        n2.set(normals.array[idx + 6], normals.array[idx + 7], normals.array[idx + 8]);

        w0.set(uvs.array[uvidx], uvs.array[uvidx + 1]);
        w1.set(uvs.array[uvidx + 2], uvs.array[uvidx + 3]);
        w2.set(uvs.array[uvidx + 4], uvs.array[uvidx + 5]);

        // Based on Eric Lengyel at http://www.terathon.com/code/tangent.html

        e1.subVectors(p1, p0)
        e2.subVectors(p2, p0);

        const x1 = w1.x - w0.x;
        const x2 = w2.x - w0.x;
        const y1 = w1.y - w0.y;
        const y2 = w2.y - w0.y;

        const denom = x1 * y2 - x2 * y1;
        const r = denom ? (1.0 / denom) : 0.0;

        tangent.subVectors(e1.clone().multiplyScalar(y2), e2.clone().multiplyScalar(y1)).multiplyScalar(r);

        // Gram-Schmidt process
        t0.subVectors(tangent, n0.multiplyScalar(n0.dot(tangent))).normalize();
        t1.subVectors(tangent, n1.multiplyScalar(n1.dot(tangent))).normalize();
        t2.subVectors(tangent, n2.multiplyScalar(n2.dot(tangent))).normalize();

        tangentsdata[idx] = t0.x;
        tangentsdata[idx + 1] = t0.y;
        tangentsdata[idx + 2] = t0.z;
        tangentsdata[idx + 3] = t1.x;
        tangentsdata[idx + 4] = t1.y;
        tangentsdata[idx + 5] = t1.z;
        tangentsdata[idx + 6] = t2.x;
        tangentsdata[idx + 7] = t2.y;
        tangentsdata[idx + 8] = t2.z;
    }

    geometry.setAttribute('tangent', new THREE.BufferAttribute( tangentsdata, 3));
}

/**
 * Create a new (half)float texture containing an alpha channel with a value of 1 from a RGB (half)float texture.
 * @param {THREE.Texture} texture 
 */
function RGBToRGBA_Float(texture) {
    const rgbData = texture.image.data;
    const length = (rgbData.length / 3) * 4;
    let rgbaData;

    switch (texture.type) {
        case THREE.FloatType:
            rgbaData = new Float32Array(length);
            break;
        case THREE.HalfFloatType:
            rgbaData = new Uint16Array(length);
            break;
        default:
          break;
    }

    if (rgbaData) {
        for (let i = 0; i < length / 4; i++) {
            rgbaData[(i * 4) + 0] = rgbData[(i * 3) + 0];
            rgbaData[(i * 4) + 1] = rgbData[(i * 3) + 1];
            rgbaData[(i * 4) + 2] = rgbData[(i * 3) + 2];
            rgbaData[(i * 4) + 3] = 1.0;
        }
        return new THREE.DataTexture(rgbaData, texture.image.width, texture.image.height, THREE.RGBAFormat, texture.type);
    }

    return texture;
}

function prepareEnvTexture(texture, capabilities) {
    const rgbaTexture = RGBToRGBA_Float(texture);
    // RGBELoader sets flipY to true by default
    rgbaTexture.flipY = false;
    rgbaTexture.wrapS = THREE.RepeatWrapping;
    rgbaTexture.anisotropy = capabilities.getMaxAnisotropy();
    rgbaTexture.minFilter = THREE.LinearMipmapLinearFilter;
    rgbaTexture.magFilter = THREE.LinearFilter;
    rgbaTexture.generateMipmaps = true;
    rgbaTexture.needsUpdate = true;

    return rgbaTexture;
}

function init() {
    let canvas = document.getElementById('webglcanvas');
    let context = canvas.getContext('webgl2');

    camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 1, 100);

    // Set up scene
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x4c4c52);
    scene.background.convertSRGBToLinear();

    scene.add(new THREE.AmbientLight( 0x222222));
    const directionalLight = new THREE.DirectionalLight(new THREE.Color(1, 0.894474, 0.567234), 2.52776);
    directionalLight.position.set(-1, 1, 1).normalize();
    scene.add(directionalLight);
    const lightData = {
      type: 1,
      direction: directionalLight.position.negate(), 
      color: new THREE.Vector3(...directionalLight.color.toArray()), 
      intensity: directionalLight.intensity
    };

    renderer = new THREE.WebGLRenderer({canvas, context});
    renderer.setPixelRatio(window.devicePixelRatio);
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
    var fileloader = new THREE.FileLoader();
    const objLoader = new OBJLoader();
    const hdrloader = new RGBELoader();

    Promise.all([
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => objLoader.load('shaderball.obj', resolve)),
        new Promise(resolve => fileloader.load('shader-frag.glsl', resolve)),
        new Promise(resolve => fileloader.load('shader-vert.glsl', resolve))
    ]).then(([loadedRadianceTexture, loadedIrradianceTexture, obj, fShader, vShader]) => {

        const radianceTexture = prepareEnvTexture(loadedRadianceTexture, renderer.capabilities);
        const irradianceTexture = prepareEnvTexture(loadedIrradianceTexture, renderer.capabilities);

        const material = new THREE.RawShaderMaterial({
            uniforms: { 
              time: { value: 0.0 },

              base: {value: 1.0},
              base_color: {value: new THREE.Vector3(0.8, 0.8, 0.8)},
              diffuse_roughness: {value: 0.0},
              metalness: {value: 0.0},
              specular: {value: 1.0},
              specular_color: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              specular_roughness: {value: 0.2},
              specular_IOR: {value: 1.5},
              specular_anisotropy: {value: 0.0},
              specular_rotation: {value: 0.0},
              transmission: {value: 0.0},
              transmission_color: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              transmission_depth: {value: 0.0},
              transmission_scatter: {value: new THREE.Vector3(0.0, 0.0, 0.0)},
              transmission_scatter_anisotropy: {value: 0.0},
              transmission_dispersion: {value: 0.0},
              transmission_extra_roughness: {value: 0.0},
              subsurface: {value: 0.0},
              subsurface_color: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              subsurface_radius: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              subsurface_scale: {value: 1.0},
              subsurface_anisotropy: {value: 0.0},
              sheen: {value: 0.0},
              sheen_color: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              sheen_roughness: {value: 0.3},
              coat: {value: 0.0},
              coat_color: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              coat_roughness: {value: 0.1},
              coat_anisotropy: {value: 0.0},
              coat_rotation: {value: 0.0},
              coat_IOR: {value: 1.5},
              coat_affect_color: {value: 0.0},
              coat_affect_roughness: {value: 0.0},
              thin_film_thickness: {value: 0.0},
              thin_film_IOR: {value: 1.5},
              emission: {value: 0.0},
              emission_color: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              opacity: {value: new THREE.Vector3(1.0, 1.0, 1.0)},
              thin_walled: {value: false},

              u_numActiveLightSources: {value: 1},
              u_lightData: {value: [ lightData ]},

              u_envMatrix: {value: new THREE.Matrix4().makeRotationY(Math.PI)},
              u_envRadiance: {value: radianceTexture},
              u_envRadianceMips: {value: Math.trunc(Math.log2(Math.max(radianceTexture.image.width, radianceTexture.image.height))) + 1},
              u_envRadianceSamples: {value: 16},
              u_envIrradiance: {value: irradianceTexture},

              u_viewPosition: new THREE.Uniform( new THREE.Vector3() ),

              u_worldMatrix: new THREE.Uniform( new THREE.Matrix4() ),
              u_viewProjectionMatrix: new THREE.Uniform( new THREE.Matrix4() ),
              u_worldInverseTransposeMatrix: new THREE.Uniform( new THREE.Matrix4() )
            },
            vertexShader: vShader,
            fragmentShader: fShader,
        });

        obj.traverse((child) => {
            if (child.isMesh) {
              generateTangents(child.geometry);
              child.geometry.attributes.uv_0 = child.geometry.attributes.uv
              child.material = material;
            }
        });
        model = obj;
        scene.add(model);

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
          uniforms.time.value = performance.now() / 1000;
          uniforms.u_viewPosition.value = camera.getWorldPosition(worldViewPos);
          uniforms.u_worldMatrix.value = child.matrixWorld;
          uniforms.u_viewProjectionMatrix.value = viewProjMat.multiplyMatrices(camera.projectionMatrix, camera.matrixWorldInverse);
          uniforms.u_worldInverseTransposeMatrix.value.setFromMatrix3(normalMat.getNormalMatrix(child.matrixWorld));
        }
      }
    });
}
