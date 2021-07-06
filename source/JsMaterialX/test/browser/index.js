// This will search for files ending in .spec.js and require them
// so that they are added to the webpack bundle
const context = require.context(
    "mocha-loader!./", // Process through mocha-loader
    false, // Skip recursive processing
    /\.spec.js$/ // Pick only files ending with .spec.js
  );
context.keys().forEach(context);