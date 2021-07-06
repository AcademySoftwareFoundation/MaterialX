const path = require('path');
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
  entry: './src/index.js',
  output: {
    filename: 'main.js',
    path: path.resolve(__dirname, 'dist')
  },
  mode: "development",
  plugins: [
    new CopyPlugin({
      patterns: [
        { 
          context: "../../../resources/Images",
          from: "*.jpg", 
          to: "Images",
        },
        { from: "../../../resources/Images/greysphere_calibration.png", to: "Images" },
        { from: "../../../resources/Geometry/shaderball.obj",  to: "Geometry"},
        { from: "../../../resources/Lights/san_giuseppe_bridge_split.hdr", to: "Lights" },
        { from: "../../../resources/Lights/irradiance/san_giuseppe_bridge_split.hdr", to: "Lights/irradiance" },
        { from: "../../../resources/Materials/Examples/StandardSurface", to: "Materials/Examples/StandardSurface" },
        { from: "../../../build/bin/JsMaterialXGenShader.wasm" },
        { from: "../../../build/bin/JsMaterialXGenShader.js" },
        { from: "../../../build/bin/JsMaterialXGenShader.data" },
      ],
    }),
  ]
};