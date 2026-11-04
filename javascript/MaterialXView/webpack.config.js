const path = require('path');
const fs = require('fs');
const webpack = require('webpack');
const CopyPlugin = require("copy-webpack-plugin");
const HtmlWebpackPlugin = require('html-webpack-plugin')

// Load material configuration from external JSON file
const materialConfig = JSON.parse(fs.readFileSync('./example_materials.json', 'utf8'));

// Function to process materials from a given path
function processMaterialPath(materialPath, baseURL) {
    const dirent = fs.readdirSync(materialPath).filter(
        function (file) { if (file.lastIndexOf(".mtlx") > -1) return file; }
    );
    return dirent.map((fileName) => ({
        name: fileName,
        value: `${baseURL}/${fileName}`
    }));
}

// Generate materials array from configuration
let materials = [];
materialConfig.materials.forEach(materialType => {
    const materialFiles = processMaterialPath(materialType.path, materialType.baseURL);
    materials = materials.concat(materialFiles);
});

const geometryFiles = "../../resources/Geometry";
const geometryFilesURL = "Geometry";
dirent = fs.readdirSync(geometryFiles).filter(
    function (file) { if (file.lastIndexOf(".glb") > -1) return file; }
)
let geometry = dirent
    .map((fileName) => ({ name: fileName, value: `${geometryFilesURL}/${fileName}` }));

// Shared asset copy (WASM, libraries, textures, materials, geometry, lights). Applied to
// both backend configs so index-webgpu.html deploys standalone with all assets.
const copyPlugin = new CopyPlugin({
    patterns: [
        { context: "../../resources/Images", from: "*.*", to: "Images" },
        { context: "../../resources/Geometry/", from: "*.glb", to: "Geometry" },
        { from: "./public", to: 'public' },
        { context: "../../resources/Lights", from: "*.*", to: "Lights" },
        { context: "../../resources/Lights/irradiance", from: "*.*", to: "Lights/irradiance" },
        ...materialConfig.materials.map(materialType => ({
            from: materialType.path,
            to: materialType.baseURL
        })),
        { from: "../build/bin/JsMaterialXCore.wasm" },
        { from: "../build/bin/JsMaterialXCore.js" },
        { from: "../build/bin/JsMaterialXGenShader.wasm" },
        { from: "../build/bin/JsMaterialXGenShader.js" },
        { from: "../build/bin/JsMaterialXGenShader.data" },
    ],
});

// Build one config per rendering backend. Both compile the SAME source; a `__BACKEND__`
// define + a per-backend `three` alias select the WebGL (classic THREE.WebGLRenderer +
// RawShaderMaterial / ESSL) or WebGPU (WebGPURenderer + NodeMaterial / WGSL) path. The two
// HTML pages let a toggle switch backends by navigation (avoids loading both three builds).
function makeConfig(backend) {
    const isWebGPU = backend === 'webgpu';
    return {
        name: backend,
        entry: './source/index.js',
        output: {
            filename: isWebGPU ? 'main.webgpu.js' : 'main.js',
            path: path.resolve(__dirname, 'dist'),
        },
        mode: "development",
        // Each bundle references only its backend's renderer at runtime, but both code paths
        // exist in the shared source, so the unused one's renderer isn't in this build's three
        // export set. That's expected — silence the "export not found in 'three'" warning.
        ignoreWarnings: [ (w) => /was not found in 'three'/.test((w && w.message) || '') ],
        // For WebGPU, resolve the bare `three` specifier to the WebGPU build (exact match
        // via `three$`, so `three/tsl` and `three/webgpu` still resolve normally).
        resolve: isWebGPU ? { alias: { 'three$': require.resolve('three/webgpu') } } : {},
        plugins: [
            new webpack.DefinePlugin({ __BACKEND__: JSON.stringify(backend) }),
            new HtmlWebpackPlugin({
                filename: isWebGPU ? 'index-webgpu.html' : 'index.html',
                template: 'index.ejs',
                templateParameters: { materials, geometry, backend },
            }),
            copyPlugin,
        ],
    };
}

module.exports = [makeConfig('webgl'), makeConfig('webgpu')];
