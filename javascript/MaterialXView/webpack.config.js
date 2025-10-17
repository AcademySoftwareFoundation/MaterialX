const path = require('path');
const fs = require('fs');
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

module.exports = {
    entry: './source/index.js',
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'dist')
    },
    mode: "development",
    plugins: [
        new HtmlWebpackPlugin({
            templateParameters: {
                materials,
                geometry
            },
            template: 'index.ejs'
        }),
        new CopyPlugin({
            patterns: [
                {
                    context: "../../resources/Images",
                    from: "*.*",
                    to: "Images",
                },
                {
                    context: "../../resources/Geometry/",
                    from: "*.glb",
                    to: "Geometry",
                },
                { from: "./public", to: 'public' },
                { context: "../../resources/Lights", from: "*.*", to: "Lights" },
                { context: "../../resources/Lights/irradiance", from: "*.*", to: "Lights/irradiance" },
                // Dynamically generate material copy patterns from configuration
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
        }),
    ]
};
