const path = require('path');
const fs = require('fs');
const CopyPlugin = require("copy-webpack-plugin");
const HtmlWebpackPlugin = require('html-webpack-plugin')

const stdSurfaceMaterials = "../../../resources/Materials/Examples/StandardSurface";
const stdSurfaceMaterialsBaseURL = "Materials/Examples/StandardSurface";

const materials = fs.readdirSync(stdSurfaceMaterials)
  .map((fileName) => ({name: fileName, value: `${stdSurfaceMaterialsBaseURL}/${fileName}`}));

module.exports = {
  entry: './src/index.js',
  output: {
    filename: 'main.js',
    path: path.resolve(__dirname, 'dist')
  },
  mode: "development",
  plugins: [
    new HtmlWebpackPlugin({
      templateParameters: {
        materials
      },
      template: 'index.ejs'
    }),
    new CopyPlugin({
      patterns: [
        { 
          context: "../../../resources/Images",
          from: "*.jpg", 
          to: "Images",
        },
        { from: "./public", to: 'public' },
        { from: "../../../resources/Images/greysphere_calibration.png", to: "Images" },
        { from: "../../../resources/Geometry/shaderball.glb",  to: "Geometry"},
        { from: "node_modules/three/examples/js/libs/draco",  to: "draco"},
        { from: "../../../resources/Lights/san_giuseppe_bridge_split.hdr", to: "Lights" },
        { from: "../../../resources/Lights/san_giuseppe_bridge_split.mtlx", to: "Lights" },
        { from: "../../../resources/Lights/irradiance/san_giuseppe_bridge_split.hdr", to: "Lights/irradiance" },
        { from: stdSurfaceMaterials, to: stdSurfaceMaterialsBaseURL },
        { from: "../../../javascript_build/bin/JsMaterialXGenShader.wasm" },
        { from: "../../../javascript_build/bin/JsMaterialXGenShader.js" },
        { from: "../../../javascript_build/bin/JsMaterialXGenShader.data" },
      ],
    }),
  ]
};