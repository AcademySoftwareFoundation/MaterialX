@rem This script builds the MaterialX system on Windows. The final command starts a local server so that you can then 
@rem run the interactive viewer locally. Here is a typical line to paste into your browser at that point (withouth the @rem):
@rem   http://localhost:8080/index.html?file=Materials/Examples/UsdPreviewSurface/usd_preview_surface_default.mtlx
@echo --------------------- Setup Emscripten ---------------------
@echo on
@rem You will likely need to change these paths to wherever you store your Github repositories
set EMSDK_LOCATION=C:/GitHub/emsdk
set MATERIALX_LOCATION=C:/GitHub/MaterialX
call %EMSDK_LOCATION%/emsdk.bat install latest
call %EMSDK_LOCATION%/emsdk.bat activate latest
if NOT ["%errorlevel%"]==["0"] pause
@echo --------------------- Build MaterialX With JavaScript ---------------------
@echo on
cd %MATERIALX_LOCATION%
cmake -S . -B javascript/build -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH=%EMSDK_LOCATION% -G Ninja
cmake --build javascript/build --target install --config RelWithDebInfo --parallel 2
if NOT ["%errorlevel%"]==["0"] pause
@echo --------------------- Run JavaScript Tests ---------------------
@echo on
cd javascript/MaterialXTest
call npm install
call npm run test
call npm run test:browser
if NOT ["%errorlevel%"]==["0"] pause
@echo --------------------- Run Interactive Viewer ---------------------
@echo on
cd ../MaterialXView
call npm install
call npm run build
call npm run start
if NOT ["%errorlevel%"]==["0"] pause
