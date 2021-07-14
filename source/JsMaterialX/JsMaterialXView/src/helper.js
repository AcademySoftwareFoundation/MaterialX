//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

import * as THREE from 'three';

const IMAGE_PROPERTY_SEPARATOR = "_";
const UADDRESS_MODE_SUFFIX = IMAGE_PROPERTY_SEPARATOR + "uaddressmode";
const VADDRESS_MODE_SUFFIX = IMAGE_PROPERTY_SEPARATOR + "vaddressmode";
const FILTER_TYPE_SUFFIX = IMAGE_PROPERTY_SEPARATOR + "filtertype";
const FILE_PREFIX = '../../../Images/';
const TARGET_FILE_PREFIX = 'Images/';

/**
 * Initialized the environment texture as MaterialX expects it
 * @param {THREE.Texture} texture
 * @param {Object} capabilities
 * @returns {THREE.Texture}
 */
export function prepareEnvTexture(texture, capabilities) {
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

function toArray(value, dimension) {
    let outValue;
    if (Array.isArray(value) && value.length === dimension)
        outValue = value;
    else {
        outValue = []; 
        value = value ? value : 0.0;
        for(let i = 0; i < dimension; ++i)
            outValue.push(value);
    }

    return outValue;
}

function toThreeUniform(type, value, name, uniformJSON, textureLoader) {
    let outValue;  
    switch(type) {
        case 'int':
        case 'uint':
        case 'float':
        case 'bool':
            outValue = value;
            break;
        case 'vec2':
        case 'ivec2':
        case 'bvec2':      
            outValue = toArray(value, 2);
            break;
        case 'vec3':
        case 'ivec3':
        case 'bvec3':
            outValue = toArray(value, 3);
            break;
        case 'vec4':
        case 'ivec4':
        case 'bvec4':
        case 'mat2':
            outValue = toArray(value, 4);
            break;
        case 'mat3':
            outValue = toArray(value, 9);
            break;
        case 'mat4':
            outValue = toArray(value, 16);
            break;
        case 'sampler2D':
            if (value) {
                const mappedValue = value.replace(FILE_PREFIX, TARGET_FILE_PREFIX)
                const texture = textureLoader.load(mappedValue);
                // Set address & filtering mode
                setTextureParameters(texture, name, uniformJSON);
                outValue = texture;
            } 
            break;
        case 'samplerCube':
              break;        
        default:
            // struct
            outValue = toThreeUniform(value);
    }

    return outValue;
}

/**
 * Takes a uniform json object from MaterialX and maps it to Three
 */
export function toThreeUniforms(uniformJSON, textureLoader) {
    let threeUniforms = {};
    for (const [name, description] of Object.entries(uniformJSON)) {
        threeUniforms[name] = new THREE.Uniform(toThreeUniform(description.type, description.value, name, uniformJSON, textureLoader));
    }

    return threeUniforms;
}

function getWrapping(mode) {
    let wrap;
    switch(mode) {
        case 1:
            wrap = THREE.ClampToEdgeWrapping;
            break;
        case 2:
            wrap = THREE.RepeatWrapping;
            break;
        case 3:
            wrap = THREE.MirroredRepeatWrapping;
            break;
        default:
            wrap = THREE.RepeatWrapping;
            break;
    }
    return wrap;
}

function getMinFilter(type, generateMipmaps) {
    const filterType = generateMipmaps ? THREE.LinearMipMapLinearFilter : THREE.LinearFilter;
    if (type === 0)
    {
        filterType = generateMipmaps ? THREE.NearestMipMapNearestFilter : THREE.NearestFilter;
    }
    return filterType;
}

function setTextureParameters(texture, name, uniformJSON, generateMipmaps = true) {
    const idx = name.lastIndexOf(IMAGE_PROPERTY_SEPARATOR);
    const base = name.substring(0, idx) || name;

    texture.generateMipmaps = generateMipmaps;

    const uaddressmode = uniformJSON[base + UADDRESS_MODE_SUFFIX] ? uniformJSON[base + UADDRESS_MODE_SUFFIX].value : -1;
    const vaddressmode = uniformJSON[base + VADDRESS_MODE_SUFFIX] ? uniformJSON[base + VADDRESS_MODE_SUFFIX].value : -1;

    texture.wrapS = getWrapping(uaddressmode);
    texture.wrapT = getWrapping(vaddressmode);

    const filterType = uniformJSON[base + FILTER_TYPE_SUFFIX] ? uniformJSON[base + FILTER_TYPE_SUFFIX].value : -1;
    texture.magFilter = THREE.LinearFilter;
    texture.minFilter = getMinFilter(filterType, generateMipmaps);
}

/**
 * Returns all lights nodes in the document
 * @param {mx.Document} doc 
 * @returns {Array.<mx.Node>}
 */
export function findLights(doc) {
    let lights = [];
    for (let node of doc.getNodes())
    {
        if (node.getType() === "lightshader")
            lights.push(node);
    }

    return lights;
}

/**
 * Register lights in shader generation context
 * @param {Object} mx MaterialX Module
 * @param {Array.<mx.Node>} lights Light nodes
 * * @param {mx.GenContext} genContext Shader generation context
 * @returns {Array.<mx.Node>}
 */
export function registerLights(mx, lights, genContext) {
    mx.HwShaderGenerator.unbindLightShaders(genContext);

    const lightTypesBound = {};
    const lightData = [];
    let lightId = 1;
    for (let light of lights) {
        let nodeDef = light.getNodeDef();
        let nodeName = nodeDef.getName();
        if (!lightTypesBound[nodeName]) {
            lightTypesBound[nodeName] = lightId;
            mx.HwShaderGenerator.bindLightShader(nodeDef, lightId++, genContext);
        }

        const lightDirection = light.getValueElement("direction").getValue().data();
        const lightColor = light.getValueElement("color").getValue().data();
        const lightIntensity = light.getValueElement("intensity").getValue();

        lightData.push({
            type: lightTypesBound[nodeName],
            direction: new THREE.Vector3(...lightDirection),
            color: new THREE.Vector3(...lightColor), 
            intensity: lightIntensity
        });
    }

    // Make sure max light count is large enough
    genContext.getOptions().hwMaxActiveLightSources = Math.max(genContext.getOptions().hwMaxActiveLightSources, lights.length);

    return lightData;
}