@echo off
set MTLXBIN=D:\materialX\MaterialX_Windows_VS2022_x64_Python312_1.39.3\bin
set MESHDIR=.

if not exist images md images

rem Note that screenWidth/Height seem to be multiplied by the Windows UI scale, plus some border

rem First the procedural textures using a cube mesh
for %%i in (test_tex_*.mtlx) do (
  rem %MTLXBIN%\MaterialXView --material %%i --mesh %MESHDIR%\cube_12in_centerUV.obj --meshRotation 30,30,0 --screenWidth 512 --screenHeight 512 --captureFilename .\images\%%~ni.png
  %MTLXBIN%\MaterialXView --material %%i --mesh %MESHDIR%\cube_12in_centerUV.obj --cameraPosition 10,15,25 --cameraTarget 0,5,2 --cameraViewAngle 40 --screenWidth 512 --screenHeight 512 --captureFilename .\images\%%~ni.png
)

rem Second the materials using the default shaderball
for %%i in (test_mat_*.mtlx) do (
  %MTLXBIN%\MaterialXView --material %%i --cameraPosition 3,3,3 --cameraViewAngle 35 --screenWidth 512 --screenHeight 512 --captureFilename .\images\%%~ni.png
)
