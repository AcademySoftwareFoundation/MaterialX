#!/usr/bin/env bash
# This script builds MaterialX JavaScript on macOS / Linux. The final command starts a local server, allowing you to
# run the MaterialX Web Viewer locally by entering 'http://localhost:8080' in the search bar of your browser.
# Run from the Github root folder.
echo "--------------------- Setup Emscripten ---------------------"
# Usage: build_javascript.sh [emsdk_location] [materialx_location]
# Pass the Emscripten and MaterialX project locations as arguments, or use the defaults below.
EMSDK_LOCATION="${1:-../emsdk}"
MATERIALX_LOCATION="${2:-.}"
source "$EMSDK_LOCATION/emsdk_env.sh" install 4.0.8 || exit 1
source "$EMSDK_LOCATION/emsdk_env.sh" activate 4.0.8 || exit 1
echo "--------------------- Setup Node.js ---------------------"
# The JavaScript tooling (Playwright, Webpack, ...) requires Node.js >= 18, and some
# packages require >= 20. If the active Node is too old, try to switch via nvm.
NODE_MAJOR="$(node -e 'process.stdout.write(String(process.versions.node.split(".")[0]))' 2>/dev/null || echo 0)"
if [ "$NODE_MAJOR" -lt 18 ]; then
    if [ -s "$HOME/.nvm/nvm.sh" ]; then
        . "$HOME/.nvm/nvm.sh"
    fi
    if command -v nvm >/dev/null 2>&1; then
        nvm use 22.16.0 >/dev/null 2>&1 || nvm use 20 >/dev/null 2>&1 || nvm install 22 >/dev/null 2>&1
        NODE_MAJOR="$(node -e 'process.stdout.write(String(process.versions.node.split(".")[0]))' 2>/dev/null || echo 0)"
    fi
fi
if [ "$NODE_MAJOR" -lt 18 ]; then
    echo "Error: Node.js >= 18 is required, but '$(node --version 2>/dev/null || echo none)' was found." >&2
    exit 1
fi
echo "Using Node $(node --version)"
echo "--------------------- Build MaterialX With JavaScript ---------------------"
cd "$MATERIALX_LOCATION" || exit 1
cmake -S . -B javascript/build -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH="$EMSDK_LOCATION" -G Ninja || exit 1
cmake --build javascript/build --target install --config RelWithDebInfo --parallel 2 || exit 1
echo "--------------------- Install JavaScript Dependencies ---------------------"
cd javascript || exit 1
npm install || exit 1
echo "--------------------- Run JavaScript Tests ---------------------"
cd MaterialXTest || exit 1
npx playwright install chromium || exit 1
npm run test || exit 1
npm run test:browser || exit 1
echo "--------------------- Run Interactive Viewer ---------------------"
cd ../MaterialXView || exit 1
npm run build || exit 1
npm run start
