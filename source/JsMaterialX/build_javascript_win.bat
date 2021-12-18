set EMSDK_LOCATION=C:/GitHub/emsdk
set MATERIALX_LOCATION=C:/GitHub/MaterialX
@echo on
@echo
@echo *** Setup Emscripten ***
call %EMSDK_LOCATION%/emsdk.bat install latest
call %EMSDK_LOCATION%/emsdk.bat activate latest
pause
@echo on
@echo
@echo *** Setup Build Configuration ***
cd %MATERIALX_LOCATION%
mkdir javascript_build
cd javascript_build
cmake -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH=%EMSDK_LOCATION% -G Ninja -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_BUILD_GEN_OSL=OFF -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_WARNINGS_AS_ERRORS=OFF  ..
cmake --build . --target install --config  RelWithDebInfo --parallel 2
pause
@echo on
@echo
@echo *** Run Bindings and Browser Tests ***
cd ..
cd source/JsMaterialX/test
call npm install
call npm run test
call npm run test:browser
pause
@echo on
@echo
@echo *** Run Interactive Viewer ***
cd ..
cd JsMaterialXView
call npm install
call npm run build
call npm install http-server -g
call http-server . -p 8000
pause
