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
        { from: "../../../resources/Geometry" },
        { from: "../../../resources/Lights" },
      ],
    }),
  ]
};