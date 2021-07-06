//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

import * as THREE from 'three';

const IMAGE_PROPERTY_SEPARATOR = "_";
const UADDRESS_MODE_SUFFIX = IMAGE_PROPERTY_SEPARATOR + "uaddressmode";
const VADDRESS_MODE_SUFFIX = IMAGE_PROPERTY_SEPARATOR + "vaddressmode";
const FILTER_TYPE_SUFFIX = IMAGE_PROPERTY_SEPARATOR + "filtertype";

/**
 * Adds a tangent BufferAttribute to the passed in geometry.
 * See MaterialXRender/Mesh.cpp
 * @param {THREE.BufferGeometry} geometry 
 */
 export function generateTangents(geometry) {
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
                const texture = textureLoader.load(value);
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
