//Maya ASCII 2020 scene
//Name: sphere_unit_test_geom.ma
//Last modified: Wed, Feb 16, 2022 11:42:13 AM
//Codeset: 1252
requires maya "2020";
requires "stereoCamera" "10.0";
requires "stereoCamera" "10.0";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
fileInfo "product" "Maya 2020";
fileInfo "version" "2020";
fileInfo "cutIdentifier" "201911140446-42a737a01c";
fileInfo "osv" "Microsoft Windows 10 Technical Preview  (Build 19042)\n";
fileInfo "UUID" "C8A41F3F-4746-00F3-6A28-7FB5C33DDC6E";
createNode transform -s -n "persp";
	rename -uid "3AB51B3D-4E54-BB23-E108-E29674BC1838";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 3.4289364542192073 2.6819987772420388 4.3124122945756174 ;
	setAttr ".r" -type "double3" -27.338352729800768 34.599999999999945 0 ;
createNode camera -s -n "perspShape" -p "persp";
	rename -uid "1DD357DC-4881-4F71-7A32-3E868963EF95";
	setAttr -k off ".v" no;
	setAttr ".fl" 34.999999999999993;
	setAttr ".coi" 5.997145883706076;
	setAttr ".imn" -type "string" "persp";
	setAttr ".den" -type "string" "persp_depth";
	setAttr ".man" -type "string" "persp_mask";
	setAttr ".tp" -type "double3" -2.9802322387695313e-07 0 -2.384185791015625e-07 ;
	setAttr ".hc" -type "string" "viewSet -p %camera";
createNode transform -s -n "top";
	rename -uid "B138368C-4635-69FE-B0BE-D5AF7794676E";
	setAttr ".v" no;
	setAttr ".t" -type "double3" -0.16231023052810645 1000.100405065934 0.025729672476989512 ;
	setAttr ".r" -type "double3" -90 0 0 ;
createNode camera -s -n "topShape" -p "top";
	rename -uid "65D4D8FA-4145-4C46-70C3-2FA110A2E2EC";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 1000.100405065934;
	setAttr ".ow" 2.9498412069446123;
	setAttr ".imn" -type "string" "top";
	setAttr ".den" -type "string" "top_depth";
	setAttr ".man" -type "string" "top_mask";
	setAttr ".tp" -type "double3" 0.99726104736328125 0 -2.384185791015625e-07 ;
	setAttr ".hc" -type "string" "viewSet -t %camera";
	setAttr ".o" yes;
createNode transform -s -n "front";
	rename -uid "67B8B2D1-4FB7-6122-FF73-C5B604E86252";
	setAttr ".v" no;
	setAttr ".t" -type "double3" -0.18799082655830257 -0.06470963359059817 1000.1 ;
createNode camera -s -n "frontShape" -p "front";
	rename -uid "A920E6D2-4866-30CA-B5D4-46B6688837C4";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 6.5598357159468685;
	setAttr ".imn" -type "string" "front";
	setAttr ".den" -type "string" "front_depth";
	setAttr ".man" -type "string" "front_mask";
	setAttr ".hc" -type "string" "viewSet -f %camera";
	setAttr ".o" yes;
createNode transform -s -n "side";
	rename -uid "C070DE1B-402B-4189-74FD-B1AEF5C3422B";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 1000.1005785860821 -0.086625536063679287 -0.39169086000181097 ;
	setAttr ".r" -type "double3" 0 90 0 ;
createNode camera -s -n "sideShape" -p "side";
	rename -uid "73394A67-477C-49AF-89A5-96892EEF40D9";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1060417000904;
	setAttr ".ow" 7.1122951879650858;
	setAttr ".imn" -type "string" "side";
	setAttr ".den" -type "string" "side_depth";
	setAttr ".man" -type "string" "side_mask";
	setAttr ".tp" -type "double3" -0.005463114008307457 -0.99657571315765381 -0.078288931399583817 ;
	setAttr ".hc" -type "string" "viewSet -s %camera";
	setAttr ".o" yes;
createNode transform -n "sphere1:pSphere1";
	rename -uid "BBA31C74-41D6-DC46-D5FC-7E99732701AB";
createNode mesh -n "sphere1:pSphereShape1" -p "sphere1:pSphere1";
	rename -uid "7D6F06F2-4E0F-37B5-95D9-AE9AF9D6E1FB";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".pv" -type "double2" 0.49999985098838806 0.5 ;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Diffuse";
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
	setAttr ".dmb" yes;
	setAttr ".bw" 3;
	setAttr ".difs" yes;
createNode transform -n "sphere1:pPlane1";
	rename -uid "49B939D2-4DBE-9952-466B-ADBC31CAB6FE";
createNode mesh -n "sphere1:pPlaneShape1" -p "sphere1:pPlane1";
	rename -uid "E7A29F55-417C-CBD5-49D5-4A96622354AE";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
createNode transform -n "back1";
	rename -uid "7A38B529-44CF-722B-0D59-35AFE0764D0D";
	setAttr ".t" -type "double3" 0 0 -1000.1 ;
	setAttr ".r" -type "double3" 0 180 0 ;
createNode camera -n "backShape1" -p "back1";
	rename -uid "D9A02514-4C74-1813-9524-86BFC73B2DA1";
	setAttr -k off ".v";
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 6.8404899425301879;
	setAttr ".imn" -type "string" "back1";
	setAttr ".den" -type "string" "back1_depth";
	setAttr ".man" -type "string" "back1_mask";
	setAttr ".hc" -type "string" "viewSet -b %camera";
	setAttr ".o" yes;
createNode transform -n "left1";
	rename -uid "57B55E7E-4A99-1969-E9ED-BB8A1DE72FE7";
	setAttr ".t" -type "double3" -1000.1 0 0 ;
	setAttr ".r" -type "double3" 0 -90 0 ;
createNode camera -n "leftShape1" -p "left1";
	rename -uid "7C3E763A-462E-146A-60E4-7CB9471DF1F3";
	setAttr -k off ".v";
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 6.3286562576142034;
	setAttr ".imn" -type "string" "left1";
	setAttr ".den" -type "string" "left1_depth";
	setAttr ".man" -type "string" "left1_mask";
	setAttr ".hc" -type "string" "viewSet -ls %camera";
	setAttr ".o" yes;
createNode transform -n "right1";
	rename -uid "40DFAF00-4C70-4A08-55C0-FF8D38861E9D";
	setAttr ".t" -type "double3" 1000.1 0 0 ;
	setAttr ".r" -type "double3" 0 90 0 ;
createNode camera -n "rightShape1" -p "right1";
	rename -uid "9FD18E54-4339-6D94-6846-71AB7F17E947";
	setAttr -k off ".v";
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 8.5753200155678559;
	setAttr ".imn" -type "string" "right1";
	setAttr ".den" -type "string" "right1_depth";
	setAttr ".man" -type "string" "right1_mask";
	setAttr ".hc" -type "string" "viewSet -rs %camera";
	setAttr ".o" yes;
createNode lightLinker -s -n "lightLinker1";
	rename -uid "9B0C2C3D-4122-AEF6-80FD-AF84E4DFBB35";
	setAttr -s 3 ".lnk";
	setAttr -s 3 ".slnk";
createNode shapeEditorManager -n "shapeEditorManager";
	rename -uid "9FCE8AD0-44D6-D729-E3B7-FB8E6B865B65";
createNode poseInterpolatorManager -n "poseInterpolatorManager";
	rename -uid "B76BD157-44EA-F7FF-0CB2-6DBBB9CD9297";
createNode displayLayerManager -n "layerManager";
	rename -uid "DEF5E9B9-4CA5-7135-2761-5CA57EE4E49A";
createNode displayLayer -n "defaultLayer";
	rename -uid "2BF55E74-48CD-AC1A-8BEE-FEA7859218A8";
createNode renderLayerManager -n "renderLayerManager";
	rename -uid "4AEC36C3-49EF-9226-41E1-68ADA711658C";
createNode renderLayer -n "defaultRenderLayer";
	rename -uid "951BD5D6-4CE4-54C9-6F09-02909E0F871B";
	setAttr ".g" yes;
createNode surfaceShader -n "sphere1:surfaceShader1";
	rename -uid "C552A5E2-4C62-D246-D2BA-8587B8A67C2F";
createNode shadingEngine -n "sphere1:surfaceShader1SG";
	rename -uid "64AEA64A-47E8-C1ED-F964-2DBDB49328FB";
	setAttr ".ihi" 0;
	setAttr -s 2 ".dsm";
	setAttr ".ro" yes;
createNode materialInfo -n "sphere1:materialInfo1";
	rename -uid "8CC8A512-45A3-9E34-63DF-80A519B80C96";
createNode file -n "sphere1:file1";
	rename -uid "54D03C3A-4D1A-BBE0-9394-E6BFF9A085A2";
	setAttr ".cg" -type "float3" 0.68373495 0.68373495 0.68373495 ;
	setAttr ".ftn" -type "string" "D:/Work/materialx/ADSK_materialx/resources/Images/grid.png";
	setAttr ".cs" -type "string" "Raw";
createNode place2dTexture -n "sphere1:place2dTexture1";
	rename -uid "4BD2C171-46B5-A7BA-21A3-F88F62B63DF2";
createNode script -n "uiConfigurationScriptNode";
	rename -uid "77EABC30-40ED-86CC-40BA-25BF09A58605";
	setAttr ".b" -type "string" (
		"// Maya Mel UI Configuration File.\n//\n//  This script is machine generated.  Edit at your own risk.\n//\n//\n\nglobal string $gMainPane;\nif (`paneLayout -exists $gMainPane`) {\n\n\tglobal int $gUseScenePanelConfig;\n\tint    $useSceneConfig = $gUseScenePanelConfig;\n\tint    $nodeEditorPanelVisible = stringArrayContains(\"nodeEditorPanel1\", `getPanel -vis`);\n\tint    $nodeEditorWorkspaceControlOpen = (`workspaceControl -exists nodeEditorPanel1Window` && `workspaceControl -q -visible nodeEditorPanel1Window`);\n\tint    $menusOkayInPanels = `optionVar -q allowMenusInPanels`;\n\tint    $nVisPanes = `paneLayout -q -nvp $gMainPane`;\n\tint    $nPanes = 0;\n\tstring $editorName;\n\tstring $panelName;\n\tstring $itemFilterName;\n\tstring $panelConfig;\n\n\t//\n\t//  get current state of the UI\n\t//\n\tsceneUIReplacement -update $gMainPane;\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Top View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Top View\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"top\" \n            -useInteractiveMode 0\n            -displayLights \"all\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 1\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 32768\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n"
		+ "            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n"
		+ "            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -greasePencils 1\n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1146\n            -height 925\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Side View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Side View\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"back1\" \n            -useInteractiveMode 0\n            -displayLights \"default\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 1\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n"
		+ "            -textureDisplay \"modulate\" \n            -textureMaxSize 32768\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n"
		+ "            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -greasePencils 1\n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1145\n            -height 443\n"
		+ "            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Front View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Front View\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"right1\" \n            -useInteractiveMode 0\n            -displayLights \"default\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n"
		+ "            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 1\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 32768\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n"
		+ "            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n"
		+ "            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -greasePencils 1\n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1146\n            -height 443\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Persp View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Persp View\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"persp\" \n            -useInteractiveMode 0\n            -displayLights \"all\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n"
		+ "            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 1\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 32768\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n"
		+ "            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n"
		+ "            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -greasePencils 1\n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1311\n            -height 684\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"outlinerPanel\" (localizedPanelLabel(\"ToggledOutliner\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\toutlinerPanel -edit -l (localizedPanelLabel(\"ToggledOutliner\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n        outlinerEditor -e \n            -showShapes 0\n            -showAssignedMaterials 0\n            -showTimeEditor 1\n            -showReferenceNodes 1\n            -showReferenceMembers 1\n            -showAttributes 0\n            -showConnected 0\n            -showAnimCurvesOnly 0\n            -showMuteInfo 0\n            -organizeByLayer 1\n            -organizeByClip 1\n            -showAnimLayerWeight 1\n            -autoExpandLayers 1\n            -autoExpand 0\n            -showDagOnly 1\n            -showAssets 1\n            -showContainedOnly 1\n            -showPublishedAsConnected 0\n            -showParentContainers 0\n            -showContainerContents 1\n            -ignoreDagHierarchy 0\n            -expandConnections 0\n            -showUpstreamCurves 1\n            -showUnitlessCurves 1\n            -showCompounds 1\n            -showLeafs 1\n            -showNumericAttrsOnly 0\n            -highlightActive 1\n            -autoSelectNewObjects 0\n            -doNotSelectNewObjects 0\n            -dropIsParent 1\n"
		+ "            -transmitFilters 0\n            -setFilter \"defaultSetFilter\" \n            -showSetMembers 1\n            -allowMultiSelection 1\n            -alwaysToggleSelect 0\n            -directSelect 0\n            -isSet 0\n            -isSetMember 0\n            -displayMode \"DAG\" \n            -expandObjects 0\n            -setsIgnoreFilters 1\n            -containersIgnoreFilters 0\n            -editAttrName 0\n            -showAttrValues 0\n            -highlightSecondary 0\n            -showUVAttrsOnly 0\n            -showTextureNodesOnly 0\n            -attrAlphaOrder \"default\" \n            -animLayerFilterOptions \"allAffecting\" \n            -sortOrder \"none\" \n            -longNames 0\n            -niceNames 1\n            -showNamespace 1\n            -showPinIcons 0\n            -mapMotionTrails 0\n            -ignoreHiddenAttribute 0\n            -ignoreOutlinerColor 0\n            -renderFilterVisible 0\n            -renderFilterIndex 0\n            -selectionOrder \"chronological\" \n            -expandAttribute 0\n            $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"outlinerPanel\" (localizedPanelLabel(\"Outliner\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\toutlinerPanel -edit -l (localizedPanelLabel(\"Outliner\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        outlinerEditor -e \n            -showShapes 0\n            -showAssignedMaterials 0\n            -showTimeEditor 1\n            -showReferenceNodes 0\n            -showReferenceMembers 0\n            -showAttributes 0\n            -showConnected 0\n            -showAnimCurvesOnly 0\n            -showMuteInfo 0\n            -organizeByLayer 1\n            -organizeByClip 1\n            -showAnimLayerWeight 1\n            -autoExpandLayers 1\n            -autoExpand 0\n            -showDagOnly 1\n            -showAssets 1\n            -showContainedOnly 1\n            -showPublishedAsConnected 0\n            -showParentContainers 0\n            -showContainerContents 1\n            -ignoreDagHierarchy 0\n"
		+ "            -expandConnections 0\n            -showUpstreamCurves 1\n            -showUnitlessCurves 1\n            -showCompounds 1\n            -showLeafs 1\n            -showNumericAttrsOnly 0\n            -highlightActive 1\n            -autoSelectNewObjects 0\n            -doNotSelectNewObjects 0\n            -dropIsParent 1\n            -transmitFilters 0\n            -setFilter \"defaultSetFilter\" \n            -showSetMembers 1\n            -allowMultiSelection 1\n            -alwaysToggleSelect 0\n            -directSelect 0\n            -displayMode \"DAG\" \n            -expandObjects 0\n            -setsIgnoreFilters 1\n            -containersIgnoreFilters 0\n            -editAttrName 0\n            -showAttrValues 0\n            -highlightSecondary 0\n            -showUVAttrsOnly 0\n            -showTextureNodesOnly 0\n            -attrAlphaOrder \"default\" \n            -animLayerFilterOptions \"allAffecting\" \n            -sortOrder \"none\" \n            -longNames 0\n            -niceNames 1\n            -showNamespace 1\n            -showPinIcons 0\n"
		+ "            -mapMotionTrails 0\n            -ignoreHiddenAttribute 0\n            -ignoreOutlinerColor 0\n            -renderFilterVisible 0\n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"graphEditor\" (localizedPanelLabel(\"Graph Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Graph Editor\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"OutlineEd\");\n            outlinerEditor -e \n                -showShapes 1\n                -showAssignedMaterials 0\n                -showTimeEditor 1\n                -showReferenceNodes 0\n                -showReferenceMembers 0\n                -showAttributes 1\n                -showConnected 1\n                -showAnimCurvesOnly 1\n                -showMuteInfo 0\n                -organizeByLayer 1\n                -organizeByClip 1\n                -showAnimLayerWeight 1\n                -autoExpandLayers 1\n"
		+ "                -autoExpand 1\n                -showDagOnly 0\n                -showAssets 1\n                -showContainedOnly 0\n                -showPublishedAsConnected 0\n                -showParentContainers 0\n                -showContainerContents 0\n                -ignoreDagHierarchy 0\n                -expandConnections 1\n                -showUpstreamCurves 1\n                -showUnitlessCurves 1\n                -showCompounds 0\n                -showLeafs 1\n                -showNumericAttrsOnly 1\n                -highlightActive 0\n                -autoSelectNewObjects 1\n                -doNotSelectNewObjects 0\n                -dropIsParent 1\n                -transmitFilters 1\n                -setFilter \"0\" \n                -showSetMembers 0\n                -allowMultiSelection 1\n                -alwaysToggleSelect 0\n                -directSelect 0\n                -displayMode \"DAG\" \n                -expandObjects 0\n                -setsIgnoreFilters 1\n                -containersIgnoreFilters 0\n                -editAttrName 0\n"
		+ "                -showAttrValues 0\n                -highlightSecondary 0\n                -showUVAttrsOnly 0\n                -showTextureNodesOnly 0\n                -attrAlphaOrder \"default\" \n                -animLayerFilterOptions \"allAffecting\" \n                -sortOrder \"none\" \n                -longNames 0\n                -niceNames 1\n                -showNamespace 1\n                -showPinIcons 1\n                -mapMotionTrails 1\n                -ignoreHiddenAttribute 0\n                -ignoreOutlinerColor 0\n                -renderFilterVisible 0\n                $editorName;\n\n\t\t\t$editorName = ($panelName+\"GraphEd\");\n            animCurveEditor -e \n                -displayValues 0\n                -snapTime \"integer\" \n                -snapValue \"none\" \n                -showPlayRangeShades \"on\" \n                -lockPlayRangeShades \"off\" \n                -smoothness \"fine\" \n                -resultSamples 1\n                -resultScreenSamples 0\n                -resultUpdate \"delayed\" \n                -showUpstreamCurves 1\n"
		+ "                -stackedCurvesMin -1\n                -stackedCurvesMax 1\n                -stackedCurvesSpace 0.2\n                -preSelectionHighlight 0\n                -constrainDrag 0\n                -valueLinesToggle 1\n                -highlightAffectedCurves 0\n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dopeSheetPanel\" (localizedPanelLabel(\"Dope Sheet\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Dope Sheet\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"OutlineEd\");\n            outlinerEditor -e \n                -showShapes 1\n                -showAssignedMaterials 0\n                -showTimeEditor 1\n                -showReferenceNodes 0\n                -showReferenceMembers 0\n                -showAttributes 1\n                -showConnected 1\n                -showAnimCurvesOnly 1\n                -showMuteInfo 0\n"
		+ "                -organizeByLayer 1\n                -organizeByClip 1\n                -showAnimLayerWeight 1\n                -autoExpandLayers 1\n                -autoExpand 0\n                -showDagOnly 0\n                -showAssets 1\n                -showContainedOnly 0\n                -showPublishedAsConnected 0\n                -showParentContainers 0\n                -showContainerContents 0\n                -ignoreDagHierarchy 0\n                -expandConnections 1\n                -showUpstreamCurves 1\n                -showUnitlessCurves 0\n                -showCompounds 1\n                -showLeafs 1\n                -showNumericAttrsOnly 1\n                -highlightActive 0\n                -autoSelectNewObjects 0\n                -doNotSelectNewObjects 1\n                -dropIsParent 1\n                -transmitFilters 0\n                -setFilter \"0\" \n                -showSetMembers 0\n                -allowMultiSelection 1\n                -alwaysToggleSelect 0\n                -directSelect 0\n                -displayMode \"DAG\" \n"
		+ "                -expandObjects 0\n                -setsIgnoreFilters 1\n                -containersIgnoreFilters 0\n                -editAttrName 0\n                -showAttrValues 0\n                -highlightSecondary 0\n                -showUVAttrsOnly 0\n                -showTextureNodesOnly 0\n                -attrAlphaOrder \"default\" \n                -animLayerFilterOptions \"allAffecting\" \n                -sortOrder \"none\" \n                -longNames 0\n                -niceNames 1\n                -showNamespace 1\n                -showPinIcons 0\n                -mapMotionTrails 1\n                -ignoreHiddenAttribute 0\n                -ignoreOutlinerColor 0\n                -renderFilterVisible 0\n                $editorName;\n\n\t\t\t$editorName = ($panelName+\"DopeSheetEd\");\n            dopeSheetEditor -e \n                -displayValues 0\n                -snapTime \"integer\" \n                -snapValue \"none\" \n                -outliner \"dopeSheetPanel1OutlineEd\" \n                -showSummary 1\n                -showScene 0\n"
		+ "                -hierarchyBelow 0\n                -showTicks 1\n                -selectionWindow 0 0 0 0 \n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"timeEditorPanel\" (localizedPanelLabel(\"Time Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Time Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"clipEditorPanel\" (localizedPanelLabel(\"Trax Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Trax Editor\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = clipEditorNameFromPanel($panelName);\n            clipEditor -e \n                -displayValues 0\n                -snapTime \"none\" \n                -snapValue \"none\" \n                -initialized 0\n"
		+ "                -manageSequencer 0 \n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"sequenceEditorPanel\" (localizedPanelLabel(\"Camera Sequencer\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Camera Sequencer\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = sequenceEditorNameFromPanel($panelName);\n            clipEditor -e \n                -displayValues 0\n                -snapTime \"none\" \n                -snapValue \"none\" \n                -initialized 0\n                -manageSequencer 1 \n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"hyperGraphPanel\" (localizedPanelLabel(\"Hypergraph Hierarchy\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Hypergraph Hierarchy\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\n\t\t\t$editorName = ($panelName+\"HyperGraphEd\");\n            hyperGraph -e \n                -graphLayoutStyle \"hierarchicalLayout\" \n                -orientation \"horiz\" \n                -mergeConnections 0\n                -zoom 1\n                -animateTransition 0\n                -showRelationships 1\n                -showShapes 0\n                -showDeformers 0\n                -showExpressions 0\n                -showConstraints 0\n                -showConnectionFromSelected 0\n                -showConnectionToSelected 0\n                -showConstraintLabels 0\n                -showUnderworld 0\n                -showInvisible 0\n                -transitionFrames 1\n                -opaqueContainers 0\n                -freeform 0\n                -imagePosition 0 0 \n                -imageScale 1\n                -imageEnabled 0\n                -graphType \"DAG\" \n                -heatMapDisplay 0\n                -updateSelection 1\n                -updateNodeAdded 1\n                -useDrawOverrideColor 0\n                -limitGraphTraversal -1\n"
		+ "                -range 0 0 \n                -iconSize \"smallIcons\" \n                -showCachedConnections 0\n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"hyperShadePanel\" (localizedPanelLabel(\"Hypershade\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Hypershade\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"visorPanel\" (localizedPanelLabel(\"Visor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Visor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"nodeEditorPanel\" (localizedPanelLabel(\"Node Editor\")) `;\n\tif ($nodeEditorPanelVisible || $nodeEditorWorkspaceControlOpen) {\n"
		+ "\t\tif (\"\" == $panelName) {\n\t\t\tif ($useSceneConfig) {\n\t\t\t\t$panelName = `scriptedPanel -unParent  -type \"nodeEditorPanel\" -l (localizedPanelLabel(\"Node Editor\")) -mbv $menusOkayInPanels `;\n\n\t\t\t$editorName = ($panelName+\"NodeEditorEd\");\n            nodeEditor -e \n                -allAttributes 0\n                -allNodes 0\n                -autoSizeNodes 1\n                -consistentNameSize 1\n                -createNodeCommand \"nodeEdCreateNodeCommand\" \n                -connectNodeOnCreation 0\n                -connectOnDrop 0\n                -copyConnectionsOnPaste 0\n                -connectionStyle \"bezier\" \n                -defaultPinnedState 0\n                -additiveGraphingMode 0\n                -settingsChangedCallback \"nodeEdSyncControls\" \n                -traversalDepthLimit -1\n                -keyPressCommand \"nodeEdKeyPressCommand\" \n                -nodeTitleMode \"name\" \n                -gridSnap 0\n                -gridVisibility 1\n                -crosshairOnEdgeDragging 0\n                -popupMenuScript \"nodeEdBuildPanelMenus\" \n"
		+ "                -showNamespace 1\n                -showShapes 1\n                -showSGShapes 0\n                -showTransforms 1\n                -useAssets 1\n                -syncedSelection 1\n                -extendToShapes 1\n                -editorMode \"default\" \n                -hasWatchpoint 0\n                $editorName;\n\t\t\t}\n\t\t} else {\n\t\t\t$label = `panel -q -label $panelName`;\n\t\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Node Editor\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"NodeEditorEd\");\n            nodeEditor -e \n                -allAttributes 0\n                -allNodes 0\n                -autoSizeNodes 1\n                -consistentNameSize 1\n                -createNodeCommand \"nodeEdCreateNodeCommand\" \n                -connectNodeOnCreation 0\n                -connectOnDrop 0\n                -copyConnectionsOnPaste 0\n                -connectionStyle \"bezier\" \n                -defaultPinnedState 0\n                -additiveGraphingMode 0\n                -settingsChangedCallback \"nodeEdSyncControls\" \n"
		+ "                -traversalDepthLimit -1\n                -keyPressCommand \"nodeEdKeyPressCommand\" \n                -nodeTitleMode \"name\" \n                -gridSnap 0\n                -gridVisibility 1\n                -crosshairOnEdgeDragging 0\n                -popupMenuScript \"nodeEdBuildPanelMenus\" \n                -showNamespace 1\n                -showShapes 1\n                -showSGShapes 0\n                -showTransforms 1\n                -useAssets 1\n                -syncedSelection 1\n                -extendToShapes 1\n                -editorMode \"default\" \n                -hasWatchpoint 0\n                $editorName;\n\t\t\tif (!$useSceneConfig) {\n\t\t\t\tpanel -e -l $label $panelName;\n\t\t\t}\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"createNodePanel\" (localizedPanelLabel(\"Create Node\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Create Node\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"polyTexturePlacementPanel\" (localizedPanelLabel(\"UV Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"UV Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"renderWindowPanel\" (localizedPanelLabel(\"Render View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Render View\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"shapePanel\" (localizedPanelLabel(\"Shape Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tshapePanel -edit -l (localizedPanelLabel(\"Shape Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n"
		+ "\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"posePanel\" (localizedPanelLabel(\"Pose Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tposePanel -edit -l (localizedPanelLabel(\"Pose Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dynRelEdPanel\" (localizedPanelLabel(\"Dynamic Relationships\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Dynamic Relationships\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"relationshipPanel\" (localizedPanelLabel(\"Relationship Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Relationship Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"referenceEditorPanel\" (localizedPanelLabel(\"Reference Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Reference Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"componentEditorPanel\" (localizedPanelLabel(\"Component Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Component Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dynPaintScriptedPanelType\" (localizedPanelLabel(\"Paint Effects\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Paint Effects\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"scriptEditorPanel\" (localizedPanelLabel(\"Script Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Script Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"profilerPanel\" (localizedPanelLabel(\"Profiler Tool\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Profiler Tool\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"contentBrowserPanel\" (localizedPanelLabel(\"Content Browser\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Content Browser\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"Stereo\" (localizedPanelLabel(\"Stereo\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Stereo\")) -mbv $menusOkayInPanels  $panelName;\n{ string $editorName = ($panelName+\"Editor\");\n            stereoCameraView -e \n                -camera \"persp\" \n                -useInteractiveMode 0\n                -displayLights \"default\" \n                -displayAppearance \"wireframe\" \n                -activeOnly 0\n                -ignorePanZoom 0\n                -wireframeOnShaded 0\n                -headsUpDisplay 1\n                -holdOuts 1\n                -selectionHiliteDisplay 1\n                -useDefaultMaterial 0\n                -bufferMode \"double\" \n                -twoSidedLighting 1\n                -backfaceCulling 0\n                -xray 0\n                -jointXray 0\n                -activeComponentsXray 0\n                -displayTextures 0\n"
		+ "                -smoothWireframe 0\n                -lineWidth 1\n                -textureAnisotropic 0\n                -textureHilight 1\n                -textureSampling 2\n                -textureDisplay \"modulate\" \n                -textureMaxSize 32768\n                -fogging 0\n                -fogSource \"fragment\" \n                -fogMode \"linear\" \n                -fogStart 0\n                -fogEnd 100\n                -fogDensity 0.1\n                -fogColor 0.5 0.5 0.5 1 \n                -depthOfFieldPreview 1\n                -maxConstantTransparency 1\n                -objectFilterShowInHUD 1\n                -isFiltered 0\n                -colorResolution 4 4 \n                -bumpResolution 4 4 \n                -textureCompression 0\n                -transparencyAlgorithm \"frontAndBackCull\" \n                -transpInShadows 0\n                -cullingOverride \"none\" \n                -lowQualityLighting 0\n                -maximumNumHardwareLights 0\n                -occlusionCulling 0\n                -shadingModel 0\n"
		+ "                -useBaseRenderer 0\n                -useReducedRenderer 0\n                -smallObjectCulling 0\n                -smallObjectThreshold -1 \n                -interactiveDisableShadows 0\n                -interactiveBackFaceCull 0\n                -sortTransparent 1\n                -controllers 1\n                -nurbsCurves 1\n                -nurbsSurfaces 1\n                -polymeshes 1\n                -subdivSurfaces 1\n                -planes 1\n                -lights 1\n                -cameras 1\n                -controlVertices 1\n                -hulls 1\n                -grid 1\n                -imagePlane 1\n                -joints 1\n                -ikHandles 1\n                -deformers 1\n                -dynamics 1\n                -particleInstancers 1\n                -fluids 1\n                -hairSystems 1\n                -follicles 1\n                -nCloths 1\n                -nParticles 1\n                -nRigids 1\n                -dynamicConstraints 1\n                -locators 1\n                -manipulators 1\n"
		+ "                -pluginShapes 1\n                -dimensions 1\n                -handles 1\n                -pivots 1\n                -textures 1\n                -strokes 1\n                -motionTrails 1\n                -clipGhosts 1\n                -greasePencils 1\n                -shadows 0\n                -captureSequenceNumber -1\n                -width 0\n                -height 0\n                -sceneRenderFilter 0\n                -displayMode \"centerEye\" \n                -viewColor 0 0 0 1 \n                -useCustomBackground 1\n                $editorName;\n            stereoCameraView -e -viewSelected 0 $editorName;\n            stereoCameraView -e \n                -pluginObjects \"gpuCacheDisplayFilter\" 1 \n                $editorName; };\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\tif ($useSceneConfig) {\n        string $configName = `getPanel -cwl (localizedPanelLabel(\"Current Layout\"))`;\n        if (\"\" != $configName) {\n\t\t\tpanelConfiguration -edit -label (localizedPanelLabel(\"Current Layout\")) \n"
		+ "\t\t\t\t-userCreated false\n\t\t\t\t-defaultImage \"\"\n\t\t\t\t-image \"\"\n\t\t\t\t-sc false\n\t\t\t\t-configString \"global string $gMainPane; paneLayout -e -cn \\\"quad\\\" -ps 1 50 66 -ps 2 50 66 -ps 3 50 34 -ps 4 50 34 $gMainPane;\"\n\t\t\t\t-removeAllPanels\n\t\t\t\t-ap false\n\t\t\t\t\t(localizedPanelLabel(\"Top View\")) \n\t\t\t\t\t\"modelPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `modelPanel -unParent -l (localizedPanelLabel(\\\"Top View\\\")) -mbv $menusOkayInPanels `;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -cam `findStartUpCamera top` \\n    -useInteractiveMode 0\\n    -displayLights \\\"all\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 32768\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -greasePencils 1\\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1146\\n    -height 925\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t\t\"modelPanel -edit -l (localizedPanelLabel(\\\"Top View\\\")) -mbv $menusOkayInPanels  $panelName;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -cam `findStartUpCamera top` \\n    -useInteractiveMode 0\\n    -displayLights \\\"all\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 32768\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -greasePencils 1\\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1146\\n    -height 925\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t-ap false\n\t\t\t\t\t(localizedPanelLabel(\"UV Editor\")) \n\t\t\t\t\t\"scriptedPanel\"\n\t\t\t\t\t\"$panelName = `scriptedPanel -unParent  -type \\\"polyTexturePlacementPanel\\\" -l (localizedPanelLabel(\\\"UV Editor\\\")) -mbv $menusOkayInPanels `\"\n\t\t\t\t\t\"scriptedPanel -edit -l (localizedPanelLabel(\\\"UV Editor\\\")) -mbv $menusOkayInPanels  $panelName\"\n\t\t\t\t-ap false\n\t\t\t\t\t(localizedPanelLabel(\"Side View\")) \n\t\t\t\t\t\"modelPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `modelPanel -unParent -l (localizedPanelLabel(\\\"Side View\\\")) -mbv $menusOkayInPanels `;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -camera \\\"back1\\\" \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 32768\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -greasePencils 1\\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1145\\n    -height 443\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t\t\"modelPanel -edit -l (localizedPanelLabel(\\\"Side View\\\")) -mbv $menusOkayInPanels  $panelName;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -camera \\\"back1\\\" \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 32768\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -greasePencils 1\\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1145\\n    -height 443\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t-ap false\n\t\t\t\t\t(localizedPanelLabel(\"Front View\")) \n\t\t\t\t\t\"modelPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `modelPanel -unParent -l (localizedPanelLabel(\\\"Front View\\\")) -mbv $menusOkayInPanels `;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -camera \\\"right1\\\" \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 32768\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -greasePencils 1\\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1146\\n    -height 443\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t\t\"modelPanel -edit -l (localizedPanelLabel(\\\"Front View\\\")) -mbv $menusOkayInPanels  $panelName;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -camera \\\"right1\\\" \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 32768\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -greasePencils 1\\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1146\\n    -height 443\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t$configName;\n\n            setNamedPanelLayout (localizedPanelLabel(\"Current Layout\"));\n        }\n\n        panelHistory -e -clear mainPanelHistory;\n        sceneUIReplacement -clear;\n\t}\n\n\ngrid -spacing 5 -size 12 -divisions 5 -displayAxes yes -displayGridLines yes -displayDivisionLines yes -displayPerspectiveLabels no -displayOrthographicLabels no -displayAxesBold yes -perspectiveLabelPosition axis -orthographicLabelPosition edge;\nviewManip -drawCompass 0 -compassAngle 0 -frontParameters \"\" -homeParameters \"\" -selectionLockParameters \"\";\n}\n");
	setAttr ".st" 3;
createNode script -n "sceneConfigurationScriptNode";
	rename -uid "1783C737-49CD-FA6D-31FD-098E0CD2A748";
	setAttr ".b" -type "string" "playbackOptions -min 1 -max 120 -ast 1 -aet 200 ";
	setAttr ".st" 6;
createNode polySphere -n "sphere1:polySphere1";
	rename -uid "90EB3591-4D32-A2F6-8CB6-F0A4FD9B8881";
	setAttr ".sa" 60;
	setAttr ".sh" 60;
createNode polyTweakUV -n "sphere1:polyTweakUV1";
	rename -uid "C256F7F9-4FC8-E376-6E61-83A413978BB8";
	setAttr ".uopa" yes;
	setAttr -s 3719 ".uvtk";
	setAttr ".uvtk[0:249]" -type "float2" 0.00049999356 0.0004833471 0.0004833471
		 0.0004833471 0.00046667084 0.0004833471 0.00044999272 0.0004833471 0.00043331832
		 0.0004833471 0.00041667372 0.0004833471 0.00039999932 0.0004833471 0.00038332492
		 0.0004833471 0.00036664307 0.0004833471 0.00034999847 0.0004833471 0.00033335388
		 0.0004833471 0.00031664968 0.0004833471 0.00029997528 0.0004833471 0.00028333068
		 0.0004833471 0.00026668608 0.0004833471 0.00024998188 0.0004833471 0.00023332238
		 0.0004833471 0.00021666288 0.0004833471 0.00020000339 0.0004833471 0.00018334389
		 0.0004833471 0.00016668439 0.0004833471 0.00014999509 0.0004833471 0.00013333559
		 0.0004833471 0.00011667609 0.0004833471 9.9986792e-05 0.0004833471 8.3327293e-05
		 0.0004833471 6.6667795e-05 0.0004833471 5.0008297e-05 0.0004833471 3.3348799e-05
		 0.0004833471 1.6659498e-05 0.0004833471 0 0.0004833471 -1.6689301e-05 0.0004833471
		 -3.3318996e-05 0.0004833471 -5.0008297e-05 0.0004833471 -6.6697598e-05 0.0004833471
		 -8.3327293e-05 0.0004833471 -0.00010001659 0.0004833471 -0.00011664629 0.0004833471
		 -0.00013333559 0.0004833471 -0.00015002489 0.0004833471 -0.00016665459 0.0004833471
		 -0.00018334389 0.0004833471 -0.00019997358 0.0004833471 -0.00021666288 0.0004833471
		 -0.00023335218 0.0004833471 -0.00024998188 0.0004833471 -0.00026667118 0.0004833471
		 -0.00028330088 0.0004833471 -0.00029999018 0.0004833471 -0.00031667948 0.0004833471
		 -0.00033336878 0.0004833471 -0.00034999847 0.0004833471 -0.00036662817 0.0004833471
		 -0.00038331747 0.0004833471 -0.00040000677 0.0004833471 -0.00041669607 0.0004833471
		 -0.00043332577 0.0004833471 -0.00044995546 0.0004833471 -0.00046664476 0.0004833471
		 -0.00048333406 0.0004833471 -0.00050002337 0.0004833471 0.00049999356 0.00046667084
		 0.0004833471 0.00046667084 0.00046667084 0.00046667084 0.00044999272 0.00046667084
		 0.00043331832 0.00046667084 0.00041667372 0.00046667084 0.00039999932 0.00046667084
		 0.00038332492 0.00046667084 0.00036664307 0.00046667084 0.00034999847 0.00046667084
		 0.00033335388 0.00046667084 0.00031664968 0.00046667084 0.00029997528 0.00046667084
		 0.00028333068 0.00046667084 0.00026668608 0.00046667084 0.00024998188 0.00046667084
		 0.00023332238 0.00046667084 0.00021666288 0.00046667084 0.00020000339 0.00046667084
		 0.00018334389 0.00046667084 0.00016668439 0.00046667084 0.00014999509 0.00046667084
		 0.00013333559 0.00046667084 0.00011667609 0.00046667084 9.9986792e-05 0.00046667084
		 8.3327293e-05 0.00046667084 6.6667795e-05 0.00046667084 5.0008297e-05 0.00046667084
		 3.3348799e-05 0.00046667084 1.6659498e-05 0.00046667084 0 0.00046667084 -1.6689301e-05
		 0.00046667084 -3.3318996e-05 0.00046667084 -5.0008297e-05 0.00046667084 -6.6697598e-05
		 0.00046667084 -8.3327293e-05 0.00046667084 -0.00010001659 0.00046667084 -0.00011664629
		 0.00046667084 -0.00013333559 0.00046667084 -0.00015002489 0.00046667084 -0.00016665459
		 0.00046667084 -0.00018334389 0.00046667084 -0.00019997358 0.00046667084 -0.00021666288
		 0.00046667084 -0.00023335218 0.00046667084 -0.00024998188 0.00046667084 -0.00026667118
		 0.00046667084 -0.00028330088 0.00046667084 -0.00029999018 0.00046667084 -0.00031667948
		 0.00046667084 -0.00033336878 0.00046667084 -0.00034999847 0.00046667084 -0.00036662817
		 0.00046667084 -0.00038331747 0.00046667084 -0.00040000677 0.00046667084 -0.00041669607
		 0.00046667084 -0.00043332577 0.00046667084 -0.00044995546 0.00046667084 -0.00046664476
		 0.00046667084 -0.00048333406 0.00046667084 -0.00050002337 0.00046667084 0.00049999356
		 0.00044999272 0.0004833471 0.00044999272 0.00046667084 0.00044999272 0.00044999272
		 0.00044999272 0.00043331832 0.00044999272 0.00041667372 0.00044999272 0.00039999932
		 0.00044999272 0.00038332492 0.00044999272 0.00036664307 0.00044999272 0.00034999847
		 0.00044999272 0.00033335388 0.00044999272 0.00031664968 0.00044999272 0.00029997528
		 0.00044999272 0.00028333068 0.00044999272 0.00026668608 0.00044999272 0.00024998188
		 0.00044999272 0.00023332238 0.00044999272 0.00021666288 0.00044999272 0.00020000339
		 0.00044999272 0.00018334389 0.00044999272 0.00016668439 0.00044999272 0.00014999509
		 0.00044999272 0.00013333559 0.00044999272 0.00011667609 0.00044999272 9.9986792e-05
		 0.00044999272 8.3327293e-05 0.00044999272 6.6667795e-05 0.00044999272 5.0008297e-05
		 0.00044999272 3.3348799e-05 0.00044999272 1.6659498e-05 0.00044999272 0 0.00044999272
		 -1.6689301e-05 0.00044999272 -3.3318996e-05 0.00044999272 -5.0008297e-05 0.00044999272
		 -6.6697598e-05 0.00044999272 -8.3327293e-05 0.00044999272 -0.00010001659 0.00044999272
		 -0.00011664629 0.00044999272 -0.00013333559 0.00044999272 -0.00015002489 0.00044999272
		 -0.00016665459 0.00044999272 -0.00018334389 0.00044999272 -0.00019997358 0.00044999272
		 -0.00021666288 0.00044999272 -0.00023335218 0.00044999272 -0.00024998188 0.00044999272
		 -0.00026667118 0.00044999272 -0.00028330088 0.00044999272 -0.00029999018 0.00044999272
		 -0.00031667948 0.00044999272 -0.00033336878 0.00044999272 -0.00034999847 0.00044999272
		 -0.00036662817 0.00044999272 -0.00038331747 0.00044999272 -0.00040000677 0.00044999272
		 -0.00041669607 0.00044999272 -0.00043332577 0.00044999272 -0.00044995546 0.00044999272
		 -0.00046664476 0.00044999272 -0.00048333406 0.00044999272 -0.00050002337 0.00044999272
		 0.00049999356 0.00043331832 0.0004833471 0.00043331832 0.00046667084 0.00043331832
		 0.00044999272 0.00043331832 0.00043331832 0.00043331832 0.00041667372 0.00043331832
		 0.00039999932 0.00043331832 0.00038332492 0.00043331832 0.00036664307 0.00043331832
		 0.00034999847 0.00043331832 0.00033335388 0.00043331832 0.00031664968 0.00043331832
		 0.00029997528 0.00043331832 0.00028333068 0.00043331832 0.00026668608 0.00043331832
		 0.00024998188 0.00043331832 0.00023332238 0.00043331832 0.00021666288 0.00043331832
		 0.00020000339 0.00043331832 0.00018334389 0.00043331832 0.00016668439 0.00043331832
		 0.00014999509 0.00043331832 0.00013333559 0.00043331832 0.00011667609 0.00043331832
		 9.9986792e-05 0.00043331832 8.3327293e-05 0.00043331832 6.6667795e-05 0.00043331832
		 5.0008297e-05 0.00043331832 3.3348799e-05 0.00043331832 1.6659498e-05 0.00043331832
		 0 0.00043331832 -1.6689301e-05 0.00043331832 -3.3318996e-05 0.00043331832 -5.0008297e-05
		 0.00043331832 -6.6697598e-05 0.00043331832 -8.3327293e-05 0.00043331832 -0.00010001659
		 0.00043331832 -0.00011664629 0.00043331832 -0.00013333559 0.00043331832 -0.00015002489
		 0.00043331832 -0.00016665459 0.00043331832 -0.00018334389 0.00043331832 -0.00019997358
		 0.00043331832 -0.00021666288 0.00043331832 -0.00023335218 0.00043331832 -0.00024998188
		 0.00043331832 -0.00026667118 0.00043331832 -0.00028330088 0.00043331832 -0.00029999018
		 0.00043331832 -0.00031667948 0.00043331832 -0.00033336878 0.00043331832 -0.00034999847
		 0.00043331832 -0.00036662817 0.00043331832 -0.00038331747 0.00043331832 -0.00040000677
		 0.00043331832 -0.00041669607 0.00043331832 -0.00043332577 0.00043331832 -0.00044995546
		 0.00043331832 -0.00046664476 0.00043331832 -0.00048333406 0.00043331832 -0.00050002337
		 0.00043331832 0.00049999356 0.00041667372 0.0004833471 0.00041667372 0.00046667084
		 0.00041667372 0.00044999272 0.00041667372 0.00043331832 0.00041667372 0.00041667372
		 0.00041667372;
	setAttr ".uvtk[250:499]" 0.00039999932 0.00041667372 0.00038332492 0.00041667372
		 0.00036664307 0.00041667372 0.00034999847 0.00041667372 0.00033335388 0.00041667372
		 0.00031664968 0.00041667372 0.00029997528 0.00041667372 0.00028333068 0.00041667372
		 0.00026668608 0.00041667372 0.00024998188 0.00041667372 0.00023332238 0.00041667372
		 0.00021666288 0.00041667372 0.00020000339 0.00041667372 0.00018334389 0.00041667372
		 0.00016668439 0.00041667372 0.00014999509 0.00041667372 0.00013333559 0.00041667372
		 0.00011667609 0.00041667372 9.9986792e-05 0.00041667372 8.3327293e-05 0.00041667372
		 6.6667795e-05 0.00041667372 5.0008297e-05 0.00041667372 3.3348799e-05 0.00041667372
		 1.6659498e-05 0.00041667372 0 0.00041667372 -1.6689301e-05 0.00041667372 -3.3318996e-05
		 0.00041667372 -5.0008297e-05 0.00041667372 -6.6697598e-05 0.00041667372 -8.3327293e-05
		 0.00041667372 -0.00010001659 0.00041667372 -0.00011664629 0.00041667372 -0.00013333559
		 0.00041667372 -0.00015002489 0.00041667372 -0.00016665459 0.00041667372 -0.00018334389
		 0.00041667372 -0.00019997358 0.00041667372 -0.00021666288 0.00041667372 -0.00023335218
		 0.00041667372 -0.00024998188 0.00041667372 -0.00026667118 0.00041667372 -0.00028330088
		 0.00041667372 -0.00029999018 0.00041667372 -0.00031667948 0.00041667372 -0.00033336878
		 0.00041667372 -0.00034999847 0.00041667372 -0.00036662817 0.00041667372 -0.00038331747
		 0.00041667372 -0.00040000677 0.00041667372 -0.00041669607 0.00041667372 -0.00043332577
		 0.00041667372 -0.00044995546 0.00041667372 -0.00046664476 0.00041667372 -0.00048333406
		 0.00041667372 -0.00050002337 0.00041667372 0.00049999356 0.00039999932 0.0004833471
		 0.00039999932 0.00046667084 0.00039999932 0.00044999272 0.00039999932 0.00043331832
		 0.00039999932 0.00041667372 0.00039999932 0.00039999932 0.00039999932 0.00038332492
		 0.00039999932 0.00036664307 0.00039999932 0.00034999847 0.00039999932 0.00033335388
		 0.00039999932 0.00031664968 0.00039999932 0.00029997528 0.00039999932 0.00028333068
		 0.00039999932 0.00026668608 0.00039999932 0.00024998188 0.00039999932 0.00023332238
		 0.00039999932 0.00021666288 0.00039999932 0.00020000339 0.00039999932 0.00018334389
		 0.00039999932 0.00016668439 0.00039999932 0.00014999509 0.00039999932 0.00013333559
		 0.00039999932 0.00011667609 0.00039999932 9.9986792e-05 0.00039999932 8.3327293e-05
		 0.00039999932 6.6667795e-05 0.00039999932 5.0008297e-05 0.00039999932 3.3348799e-05
		 0.00039999932 1.6659498e-05 0.00039999932 0 0.00039999932 -1.6689301e-05 0.00039999932
		 -3.3318996e-05 0.00039999932 -5.0008297e-05 0.00039999932 -6.6697598e-05 0.00039999932
		 -8.3327293e-05 0.00039999932 -0.00010001659 0.00039999932 -0.00011664629 0.00039999932
		 -0.00013333559 0.00039999932 -0.00015002489 0.00039999932 -0.00016665459 0.00039999932
		 -0.00018334389 0.00039999932 -0.00019997358 0.00039999932 -0.00021666288 0.00039999932
		 -0.00023335218 0.00039999932 -0.00024998188 0.00039999932 -0.00026667118 0.00039999932
		 -0.00028330088 0.00039999932 -0.00029999018 0.00039999932 -0.00031667948 0.00039999932
		 -0.00033336878 0.00039999932 -0.00034999847 0.00039999932 -0.00036662817 0.00039999932
		 -0.00038331747 0.00039999932 -0.00040000677 0.00039999932 -0.00041669607 0.00039999932
		 -0.00043332577 0.00039999932 -0.00044995546 0.00039999932 -0.00046664476 0.00039999932
		 -0.00048333406 0.00039999932 -0.00050002337 0.00039999932 0.00049999356 0.00038332492
		 0.0004833471 0.00038332492 0.00046667084 0.00038332492 0.00044999272 0.00038332492
		 0.00043331832 0.00038332492 0.00041667372 0.00038332492 0.00039999932 0.00038332492
		 0.00038332492 0.00038332492 0.00036664307 0.00038332492 0.00034999847 0.00038332492
		 0.00033335388 0.00038332492 0.00031664968 0.00038332492 0.00029997528 0.00038332492
		 0.00028333068 0.00038332492 0.00026668608 0.00038332492 0.00024998188 0.00038332492
		 0.00023332238 0.00038332492 0.00021666288 0.00038332492 0.00020000339 0.00038332492
		 0.00018334389 0.00038332492 0.00016668439 0.00038332492 0.00014999509 0.00038332492
		 0.00013333559 0.00038332492 0.00011667609 0.00038332492 9.9986792e-05 0.00038332492
		 8.3327293e-05 0.00038332492 6.6667795e-05 0.00038332492 5.0008297e-05 0.00038332492
		 3.3348799e-05 0.00038332492 1.6659498e-05 0.00038332492 0 0.00038332492 -1.6689301e-05
		 0.00038332492 -3.3318996e-05 0.00038332492 -5.0008297e-05 0.00038332492 -6.6697598e-05
		 0.00038332492 -8.3327293e-05 0.00038332492 -0.00010001659 0.00038332492 -0.00011664629
		 0.00038332492 -0.00013333559 0.00038332492 -0.00015002489 0.00038332492 -0.00016665459
		 0.00038332492 -0.00018334389 0.00038332492 -0.00019997358 0.00038332492 -0.00021666288
		 0.00038332492 -0.00023335218 0.00038332492 -0.00024998188 0.00038332492 -0.00026667118
		 0.00038332492 -0.00028330088 0.00038332492 -0.00029999018 0.00038332492 -0.00031667948
		 0.00038332492 -0.00033336878 0.00038332492 -0.00034999847 0.00038332492 -0.00036662817
		 0.00038332492 -0.00038331747 0.00038332492 -0.00040000677 0.00038332492 -0.00041669607
		 0.00038332492 -0.00043332577 0.00038332492 -0.00044995546 0.00038332492 -0.00046664476
		 0.00038332492 -0.00048333406 0.00038332492 -0.00050002337 0.00038332492 0.00049999356
		 0.00036664307 0.0004833471 0.00036664307 0.00046667084 0.00036664307 0.00044999272
		 0.00036664307 0.00043331832 0.00036664307 0.00041667372 0.00036664307 0.00039999932
		 0.00036664307 0.00038332492 0.00036664307 0.00036664307 0.00036664307 0.00034999847
		 0.00036664307 0.00033335388 0.00036664307 0.00031664968 0.00036664307 0.00029997528
		 0.00036664307 0.00028333068 0.00036664307 0.00026668608 0.00036664307 0.00024998188
		 0.00036664307 0.00023332238 0.00036664307 0.00021666288 0.00036664307 0.00020000339
		 0.00036664307 0.00018334389 0.00036664307 0.00016668439 0.00036664307 0.00014999509
		 0.00036664307 0.00013333559 0.00036664307 0.00011667609 0.00036664307 9.9986792e-05
		 0.00036664307 8.3327293e-05 0.00036664307 6.6667795e-05 0.00036664307 5.0008297e-05
		 0.00036664307 3.3348799e-05 0.00036664307 1.6659498e-05 0.00036664307 0 0.00036664307
		 -1.6689301e-05 0.00036664307 -3.3318996e-05 0.00036664307 -5.0008297e-05 0.00036664307
		 -6.6697598e-05 0.00036664307 -8.3327293e-05 0.00036664307 -0.00010001659 0.00036664307
		 -0.00011664629 0.00036664307 -0.00013333559 0.00036664307 -0.00015002489 0.00036664307
		 -0.00016665459 0.00036664307 -0.00018334389 0.00036664307 -0.00019997358 0.00036664307
		 -0.00021666288 0.00036664307 -0.00023335218 0.00036664307 -0.00024998188 0.00036664307
		 -0.00026667118 0.00036664307 -0.00028330088 0.00036664307 -0.00029999018 0.00036664307
		 -0.00031667948 0.00036664307 -0.00033336878 0.00036664307 -0.00034999847 0.00036664307
		 -0.00036662817 0.00036664307 -0.00038331747 0.00036664307 -0.00040000677 0.00036664307
		 -0.00041669607 0.00036664307 -0.00043332577 0.00036664307 -0.00044995546 0.00036664307
		 -0.00046664476 0.00036664307 -0.00048333406 0.00036664307 -0.00050002337 0.00036664307
		 0.00049999356 0.00034999847 0.0004833471 0.00034999847 0.00046667084 0.00034999847
		 0.00044999272 0.00034999847 0.00043331832 0.00034999847 0.00041667372 0.00034999847
		 0.00039999932 0.00034999847 0.00038332492 0.00034999847 0.00036664307 0.00034999847
		 0.00034999847 0.00034999847 0.00033335388 0.00034999847 0.00031664968 0.00034999847;
	setAttr ".uvtk[500:749]" 0.00029997528 0.00034999847 0.00028333068 0.00034999847
		 0.00026668608 0.00034999847 0.00024998188 0.00034999847 0.00023332238 0.00034999847
		 0.00021666288 0.00034999847 0.00020000339 0.00034999847 0.00018334389 0.00034999847
		 0.00016668439 0.00034999847 0.00014999509 0.00034999847 0.00013333559 0.00034999847
		 0.00011667609 0.00034999847 9.9986792e-05 0.00034999847 8.3327293e-05 0.00034999847
		 6.6667795e-05 0.00034999847 5.0008297e-05 0.00034999847 3.3348799e-05 0.00034999847
		 1.6659498e-05 0.00034999847 0 0.00034999847 -1.6689301e-05 0.00034999847 -3.3318996e-05
		 0.00034999847 -5.0008297e-05 0.00034999847 -6.6697598e-05 0.00034999847 -8.3327293e-05
		 0.00034999847 -0.00010001659 0.00034999847 -0.00011664629 0.00034999847 -0.00013333559
		 0.00034999847 -0.00015002489 0.00034999847 -0.00016665459 0.00034999847 -0.00018334389
		 0.00034999847 -0.00019997358 0.00034999847 -0.00021666288 0.00034999847 -0.00023335218
		 0.00034999847 -0.00024998188 0.00034999847 -0.00026667118 0.00034999847 -0.00028330088
		 0.00034999847 -0.00029999018 0.00034999847 -0.00031667948 0.00034999847 -0.00033336878
		 0.00034999847 -0.00034999847 0.00034999847 -0.00036662817 0.00034999847 -0.00038331747
		 0.00034999847 -0.00040000677 0.00034999847 -0.00041669607 0.00034999847 -0.00043332577
		 0.00034999847 -0.00044995546 0.00034999847 -0.00046664476 0.00034999847 -0.00048333406
		 0.00034999847 -0.00050002337 0.00034999847 0.00049999356 0.00033335388 0.0004833471
		 0.00033335388 0.00046667084 0.00033335388 0.00044999272 0.00033335388 0.00043331832
		 0.00033335388 0.00041667372 0.00033335388 0.00039999932 0.00033335388 0.00038332492
		 0.00033335388 0.00036664307 0.00033335388 0.00034999847 0.00033335388 0.00033335388
		 0.00033335388 0.00031664968 0.00033335388 0.00029997528 0.00033335388 0.00028333068
		 0.00033335388 0.00026668608 0.00033335388 0.00024998188 0.00033335388 0.00023332238
		 0.00033335388 0.00021666288 0.00033335388 0.00020000339 0.00033335388 0.00018334389
		 0.00033335388 0.00016668439 0.00033335388 0.00014999509 0.00033335388 0.00013333559
		 0.00033335388 0.00011667609 0.00033335388 9.9986792e-05 0.00033335388 8.3327293e-05
		 0.00033335388 6.6667795e-05 0.00033335388 5.0008297e-05 0.00033335388 3.3348799e-05
		 0.00033335388 1.6659498e-05 0.00033335388 0 0.00033335388 -1.6689301e-05 0.00033335388
		 -3.3318996e-05 0.00033335388 -5.0008297e-05 0.00033335388 -6.6697598e-05 0.00033335388
		 -8.3327293e-05 0.00033335388 -0.00010001659 0.00033335388 -0.00011664629 0.00033335388
		 -0.00013333559 0.00033335388 -0.00015002489 0.00033335388 -0.00016665459 0.00033335388
		 -0.00018334389 0.00033335388 -0.00019997358 0.00033335388 -0.00021666288 0.00033335388
		 -0.00023335218 0.00033335388 -0.00024998188 0.00033335388 -0.00026667118 0.00033335388
		 -0.00028330088 0.00033335388 -0.00029999018 0.00033335388 -0.00031667948 0.00033335388
		 -0.00033336878 0.00033335388 -0.00034999847 0.00033335388 -0.00036662817 0.00033335388
		 -0.00038331747 0.00033335388 -0.00040000677 0.00033335388 -0.00041669607 0.00033335388
		 -0.00043332577 0.00033335388 -0.00044995546 0.00033335388 -0.00046664476 0.00033335388
		 -0.00048333406 0.00033335388 -0.00050002337 0.00033335388 0.00049999356 0.00031664968
		 0.0004833471 0.00031664968 0.00046667084 0.00031664968 0.00044999272 0.00031664968
		 0.00043331832 0.00031664968 0.00041667372 0.00031664968 0.00039999932 0.00031664968
		 0.00038332492 0.00031664968 0.00036664307 0.00031664968 0.00034999847 0.00031664968
		 0.00033335388 0.00031664968 0.00031664968 0.00031664968 0.00029997528 0.00031664968
		 0.00028333068 0.00031664968 0.00026668608 0.00031664968 0.00024998188 0.00031664968
		 0.00023332238 0.00031664968 0.00021666288 0.00031664968 0.00020000339 0.00031664968
		 0.00018334389 0.00031664968 0.00016668439 0.00031664968 0.00014999509 0.00031664968
		 0.00013333559 0.00031664968 0.00011667609 0.00031664968 9.9986792e-05 0.00031664968
		 8.3327293e-05 0.00031664968 6.6667795e-05 0.00031664968 5.0008297e-05 0.00031664968
		 3.3348799e-05 0.00031664968 1.6659498e-05 0.00031664968 0 0.00031664968 -1.6689301e-05
		 0.00031664968 -3.3318996e-05 0.00031664968 -5.0008297e-05 0.00031664968 -6.6697598e-05
		 0.00031664968 -8.3327293e-05 0.00031664968 -0.00010001659 0.00031664968 -0.00011664629
		 0.00031664968 -0.00013333559 0.00031664968 -0.00015002489 0.00031664968 -0.00016665459
		 0.00031664968 -0.00018334389 0.00031664968 -0.00019997358 0.00031664968 -0.00021666288
		 0.00031664968 -0.00023335218 0.00031664968 -0.00024998188 0.00031664968 -0.00026667118
		 0.00031664968 -0.00028330088 0.00031664968 -0.00029999018 0.00031664968 -0.00031667948
		 0.00031664968 -0.00033336878 0.00031664968 -0.00034999847 0.00031664968 -0.00036662817
		 0.00031664968 -0.00038331747 0.00031664968 -0.00040000677 0.00031664968 -0.00041669607
		 0.00031664968 -0.00043332577 0.00031664968 -0.00044995546 0.00031664968 -0.00046664476
		 0.00031664968 -0.00048333406 0.00031664968 -0.00050002337 0.00031664968 0.00049999356
		 0.00029997528 0.0004833471 0.00029997528 0.00046667084 0.00029997528 0.00044999272
		 0.00029997528 0.00043331832 0.00029997528 0.00041667372 0.00029997528 0.00039999932
		 0.00029997528 0.00038332492 0.00029997528 0.00036664307 0.00029997528 0.00034999847
		 0.00029997528 0.00033335388 0.00029997528 0.00031664968 0.00029997528 0.00029997528
		 0.00029997528 0.00028333068 0.00029997528 0.00026668608 0.00029997528 0.00024998188
		 0.00029997528 0.00023332238 0.00029997528 0.00021666288 0.00029997528 0.00020000339
		 0.00029997528 0.00018334389 0.00029997528 0.00016668439 0.00029997528 0.00014999509
		 0.00029997528 0.00013333559 0.00029997528 0.00011667609 0.00029997528 9.9986792e-05
		 0.00029997528 8.3327293e-05 0.00029997528 6.6667795e-05 0.00029997528 5.0008297e-05
		 0.00029997528 3.3348799e-05 0.00029997528 1.6659498e-05 0.00029997528 0 0.00029997528
		 -1.6689301e-05 0.00029997528 -3.3318996e-05 0.00029997528 -5.0008297e-05 0.00029997528
		 -6.6697598e-05 0.00029997528 -8.3327293e-05 0.00029997528 -0.00010001659 0.00029997528
		 -0.00011664629 0.00029997528 -0.00013333559 0.00029997528 -0.00015002489 0.00029997528
		 -0.00016665459 0.00029997528 -0.00018334389 0.00029997528 -0.00019997358 0.00029997528
		 -0.00021666288 0.00029997528 -0.00023335218 0.00029997528 -0.00024998188 0.00029997528
		 -0.00026667118 0.00029997528 -0.00028330088 0.00029997528 -0.00029999018 0.00029997528
		 -0.00031667948 0.00029997528 -0.00033336878 0.00029997528 -0.00034999847 0.00029997528
		 -0.00036662817 0.00029997528 -0.00038331747 0.00029997528 -0.00040000677 0.00029997528
		 -0.00041669607 0.00029997528 -0.00043332577 0.00029997528 -0.00044995546 0.00029997528
		 -0.00046664476 0.00029997528 -0.00048333406 0.00029997528 -0.00050002337 0.00029997528
		 0.00049999356 0.00028333068 0.0004833471 0.00028333068 0.00046667084 0.00028333068
		 0.00044999272 0.00028333068 0.00043331832 0.00028333068 0.00041667372 0.00028333068
		 0.00039999932 0.00028333068 0.00038332492 0.00028333068 0.00036664307 0.00028333068
		 0.00034999847 0.00028333068 0.00033335388 0.00028333068 0.00031664968 0.00028333068
		 0.00029997528 0.00028333068 0.00028333068 0.00028333068 0.00026668608 0.00028333068
		 0.00024998188 0.00028333068 0.00023332238 0.00028333068 0.00021666288 0.00028333068;
	setAttr ".uvtk[750:999]" 0.00020000339 0.00028333068 0.00018334389 0.00028333068
		 0.00016668439 0.00028333068 0.00014999509 0.00028333068 0.00013333559 0.00028333068
		 0.00011667609 0.00028333068 9.9986792e-05 0.00028333068 8.3327293e-05 0.00028333068
		 6.6667795e-05 0.00028333068 5.0008297e-05 0.00028333068 3.3348799e-05 0.00028333068
		 1.6659498e-05 0.00028333068 0 0.00028333068 -1.6689301e-05 0.00028333068 -3.3318996e-05
		 0.00028333068 -5.0008297e-05 0.00028333068 -6.6697598e-05 0.00028333068 -8.3327293e-05
		 0.00028333068 -0.00010001659 0.00028333068 -0.00011664629 0.00028333068 -0.00013333559
		 0.00028333068 -0.00015002489 0.00028333068 -0.00016665459 0.00028333068 -0.00018334389
		 0.00028333068 -0.00019997358 0.00028333068 -0.00021666288 0.00028333068 -0.00023335218
		 0.00028333068 -0.00024998188 0.00028333068 -0.00026667118 0.00028333068 -0.00028330088
		 0.00028333068 -0.00029999018 0.00028333068 -0.00031667948 0.00028333068 -0.00033336878
		 0.00028333068 -0.00034999847 0.00028333068 -0.00036662817 0.00028333068 -0.00038331747
		 0.00028333068 -0.00040000677 0.00028333068 -0.00041669607 0.00028333068 -0.00043332577
		 0.00028333068 -0.00044995546 0.00028333068 -0.00046664476 0.00028333068 -0.00048333406
		 0.00028333068 -0.00050002337 0.00028333068 0.00049999356 0.00026668608 0.0004833471
		 0.00026668608 0.00046667084 0.00026668608 0.00044999272 0.00026668608 0.00043331832
		 0.00026668608 0.00041667372 0.00026668608 0.00039999932 0.00026668608 0.00038332492
		 0.00026668608 0.00036664307 0.00026668608 0.00034999847 0.00026668608 0.00033335388
		 0.00026668608 0.00031664968 0.00026668608 0.00029997528 0.00026668608 0.00028333068
		 0.00026668608 0.00026668608 0.00026668608 0.00024998188 0.00026668608 0.00023332238
		 0.00026668608 0.00021666288 0.00026668608 0.00020000339 0.00026668608 0.00018334389
		 0.00026668608 0.00016668439 0.00026668608 0.00014999509 0.00026668608 0.00013333559
		 0.00026668608 0.00011667609 0.00026668608 9.9986792e-05 0.00026668608 8.3327293e-05
		 0.00026668608 6.6667795e-05 0.00026668608 5.0008297e-05 0.00026668608 3.3348799e-05
		 0.00026668608 1.6659498e-05 0.00026668608 0 0.00026668608 -1.6689301e-05 0.00026668608
		 -3.3318996e-05 0.00026668608 -5.0008297e-05 0.00026668608 -6.6697598e-05 0.00026668608
		 -8.3327293e-05 0.00026668608 -0.00010001659 0.00026668608 -0.00011664629 0.00026668608
		 -0.00013333559 0.00026668608 -0.00015002489 0.00026668608 -0.00016665459 0.00026668608
		 -0.00018334389 0.00026668608 -0.00019997358 0.00026668608 -0.00021666288 0.00026668608
		 -0.00023335218 0.00026668608 -0.00024998188 0.00026668608 -0.00026667118 0.00026668608
		 -0.00028330088 0.00026668608 -0.00029999018 0.00026668608 -0.00031667948 0.00026668608
		 -0.00033336878 0.00026668608 -0.00034999847 0.00026668608 -0.00036662817 0.00026668608
		 -0.00038331747 0.00026668608 -0.00040000677 0.00026668608 -0.00041669607 0.00026668608
		 -0.00043332577 0.00026668608 -0.00044995546 0.00026668608 -0.00046664476 0.00026668608
		 -0.00048333406 0.00026668608 -0.00050002337 0.00026668608 0.00049999356 0.00024998188
		 0.0004833471 0.00024998188 0.00046667084 0.00024998188 0.00044999272 0.00024998188
		 0.00043331832 0.00024998188 0.00041667372 0.00024998188 0.00039999932 0.00024998188
		 0.00038332492 0.00024998188 0.00036664307 0.00024998188 0.00034999847 0.00024998188
		 0.00033335388 0.00024998188 0.00031664968 0.00024998188 0.00029997528 0.00024998188
		 0.00028333068 0.00024998188 0.00026668608 0.00024998188 0.00024998188 0.00024998188
		 0.00023332238 0.00024998188 0.00021666288 0.00024998188 0.00020000339 0.00024998188
		 0.00018334389 0.00024998188 0.00016668439 0.00024998188 0.00014999509 0.00024998188
		 0.00013333559 0.00024998188 0.00011667609 0.00024998188 9.9986792e-05 0.00024998188
		 8.3327293e-05 0.00024998188 6.6667795e-05 0.00024998188 5.0008297e-05 0.00024998188
		 3.3348799e-05 0.00024998188 1.6659498e-05 0.00024998188 0 0.00024998188 -1.6689301e-05
		 0.00024998188 -3.3318996e-05 0.00024998188 -5.0008297e-05 0.00024998188 -6.6697598e-05
		 0.00024998188 -8.3327293e-05 0.00024998188 -0.00010001659 0.00024998188 -0.00011664629
		 0.00024998188 -0.00013333559 0.00024998188 -0.00015002489 0.00024998188 -0.00016665459
		 0.00024998188 -0.00018334389 0.00024998188 -0.00019997358 0.00024998188 -0.00021666288
		 0.00024998188 -0.00023335218 0.00024998188 -0.00024998188 0.00024998188 -0.00026667118
		 0.00024998188 -0.00028330088 0.00024998188 -0.00029999018 0.00024998188 -0.00031667948
		 0.00024998188 -0.00033336878 0.00024998188 -0.00034999847 0.00024998188 -0.00036662817
		 0.00024998188 -0.00038331747 0.00024998188 -0.00040000677 0.00024998188 -0.00041669607
		 0.00024998188 -0.00043332577 0.00024998188 -0.00044995546 0.00024998188 -0.00046664476
		 0.00024998188 -0.00048333406 0.00024998188 -0.00050002337 0.00024998188 0.00049999356
		 0.00023332238 0.0004833471 0.00023332238 0.00046667084 0.00023332238 0.00044999272
		 0.00023332238 0.00043331832 0.00023332238 0.00041667372 0.00023332238 0.00039999932
		 0.00023332238 0.00038332492 0.00023332238 0.00036664307 0.00023332238 0.00034999847
		 0.00023332238 0.00033335388 0.00023332238 0.00031664968 0.00023332238 0.00029997528
		 0.00023332238 0.00028333068 0.00023332238 0.00026668608 0.00023332238 0.00024998188
		 0.00023332238 0.00023332238 0.00023332238 0.00021666288 0.00023332238 0.00020000339
		 0.00023332238 0.00018334389 0.00023332238 0.00016668439 0.00023332238 0.00014999509
		 0.00023332238 0.00013333559 0.00023332238 0.00011667609 0.00023332238 9.9986792e-05
		 0.00023332238 8.3327293e-05 0.00023332238 6.6667795e-05 0.00023332238 5.0008297e-05
		 0.00023332238 3.3348799e-05 0.00023332238 1.6659498e-05 0.00023332238 0 0.00023332238
		 -1.6689301e-05 0.00023332238 -3.3318996e-05 0.00023332238 -5.0008297e-05 0.00023332238
		 -6.6697598e-05 0.00023332238 -8.3327293e-05 0.00023332238 -0.00010001659 0.00023332238
		 -0.00011664629 0.00023332238 -0.00013333559 0.00023332238 -0.00015002489 0.00023332238
		 -0.00016665459 0.00023332238 -0.00018334389 0.00023332238 -0.00019997358 0.00023332238
		 -0.00021666288 0.00023332238 -0.00023335218 0.00023332238 -0.00024998188 0.00023332238
		 -0.00026667118 0.00023332238 -0.00028330088 0.00023332238 -0.00029999018 0.00023332238
		 -0.00031667948 0.00023332238 -0.00033336878 0.00023332238 -0.00034999847 0.00023332238
		 -0.00036662817 0.00023332238 -0.00038331747 0.00023332238 -0.00040000677 0.00023332238
		 -0.00041669607 0.00023332238 -0.00043332577 0.00023332238 -0.00044995546 0.00023332238
		 -0.00046664476 0.00023332238 -0.00048333406 0.00023332238 -0.00050002337 0.00023332238
		 0.00049999356 0.00021666288 0.0004833471 0.00021666288 0.00046667084 0.00021666288
		 0.00044999272 0.00021666288 0.00043331832 0.00021666288 0.00041667372 0.00021666288
		 0.00039999932 0.00021666288 0.00038332492 0.00021666288 0.00036664307 0.00021666288
		 0.00034999847 0.00021666288 0.00033335388 0.00021666288 0.00031664968 0.00021666288
		 0.00029997528 0.00021666288 0.00028333068 0.00021666288 0.00026668608 0.00021666288
		 0.00024998188 0.00021666288 0.00023332238 0.00021666288 0.00021666288 0.00021666288
		 0.00020000339 0.00021666288 0.00018334389 0.00021666288 0.00016668439 0.00021666288
		 0.00014999509 0.00021666288 0.00013333559 0.00021666288 0.00011667609 0.00021666288;
	setAttr ".uvtk[1000:1249]" 9.9986792e-05 0.00021666288 8.3327293e-05 0.00021666288
		 6.6667795e-05 0.00021666288 5.0008297e-05 0.00021666288 3.3348799e-05 0.00021666288
		 1.6659498e-05 0.00021666288 0 0.00021666288 -1.6689301e-05 0.00021666288 -3.3318996e-05
		 0.00021666288 -5.0008297e-05 0.00021666288 -6.6697598e-05 0.00021666288 -8.3327293e-05
		 0.00021666288 -0.00010001659 0.00021666288 -0.00011664629 0.00021666288 -0.00013333559
		 0.00021666288 -0.00015002489 0.00021666288 -0.00016665459 0.00021666288 -0.00018334389
		 0.00021666288 -0.00019997358 0.00021666288 -0.00021666288 0.00021666288 -0.00023335218
		 0.00021666288 -0.00024998188 0.00021666288 -0.00026667118 0.00021666288 -0.00028330088
		 0.00021666288 -0.00029999018 0.00021666288 -0.00031667948 0.00021666288 -0.00033336878
		 0.00021666288 -0.00034999847 0.00021666288 -0.00036662817 0.00021666288 -0.00038331747
		 0.00021666288 -0.00040000677 0.00021666288 -0.00041669607 0.00021666288 -0.00043332577
		 0.00021666288 -0.00044995546 0.00021666288 -0.00046664476 0.00021666288 -0.00048333406
		 0.00021666288 -0.00050002337 0.00021666288 0.00049999356 0.00020000339 0.0004833471
		 0.00020000339 0.00046667084 0.00020000339 0.00044999272 0.00020000339 0.00043331832
		 0.00020000339 0.00041667372 0.00020000339 0.00039999932 0.00020000339 0.00038332492
		 0.00020000339 0.00036664307 0.00020000339 0.00034999847 0.00020000339 0.00033335388
		 0.00020000339 0.00031664968 0.00020000339 0.00029997528 0.00020000339 0.00028333068
		 0.00020000339 0.00026668608 0.00020000339 0.00024998188 0.00020000339 0.00023332238
		 0.00020000339 0.00021666288 0.00020000339 0.00020000339 0.00020000339 0.00018334389
		 0.00020000339 0.00016668439 0.00020000339 0.00014999509 0.00020000339 0.00013333559
		 0.00020000339 0.00011667609 0.00020000339 9.9986792e-05 0.00020000339 8.3327293e-05
		 0.00020000339 6.6667795e-05 0.00020000339 5.0008297e-05 0.00020000339 3.3348799e-05
		 0.00020000339 1.6659498e-05 0.00020000339 0 0.00020000339 -1.6689301e-05 0.00020000339
		 -3.3318996e-05 0.00020000339 -5.0008297e-05 0.00020000339 -6.6697598e-05 0.00020000339
		 -8.3327293e-05 0.00020000339 -0.00010001659 0.00020000339 -0.00011664629 0.00020000339
		 -0.00013333559 0.00020000339 -0.00015002489 0.00020000339 -0.00016665459 0.00020000339
		 -0.00018334389 0.00020000339 -0.00019997358 0.00020000339 -0.00021666288 0.00020000339
		 -0.00023335218 0.00020000339 -0.00024998188 0.00020000339 -0.00026667118 0.00020000339
		 -0.00028330088 0.00020000339 -0.00029999018 0.00020000339 -0.00031667948 0.00020000339
		 -0.00033336878 0.00020000339 -0.00034999847 0.00020000339 -0.00036662817 0.00020000339
		 -0.00038331747 0.00020000339 -0.00040000677 0.00020000339 -0.00041669607 0.00020000339
		 -0.00043332577 0.00020000339 -0.00044995546 0.00020000339 -0.00046664476 0.00020000339
		 -0.00048333406 0.00020000339 -0.00050002337 0.00020000339 0.00049999356 0.00018334389
		 0.0004833471 0.00018334389 0.00046667084 0.00018334389 0.00044999272 0.00018334389
		 0.00043331832 0.00018334389 0.00041667372 0.00018334389 0.00039999932 0.00018334389
		 0.00038332492 0.00018334389 0.00036664307 0.00018334389 0.00034999847 0.00018334389
		 0.00033335388 0.00018334389 0.00031664968 0.00018334389 0.00029997528 0.00018334389
		 0.00028333068 0.00018334389 0.00026668608 0.00018334389 0.00024998188 0.00018334389
		 0.00023332238 0.00018334389 0.00021666288 0.00018334389 0.00020000339 0.00018334389
		 0.00018334389 0.00018334389 0.00016668439 0.00018334389 0.00014999509 0.00018334389
		 0.00013333559 0.00018334389 0.00011667609 0.00018334389 9.9986792e-05 0.00018334389
		 8.3327293e-05 0.00018334389 6.6667795e-05 0.00018334389 5.0008297e-05 0.00018334389
		 3.3348799e-05 0.00018334389 1.6659498e-05 0.00018334389 0 0.00018334389 -1.6689301e-05
		 0.00018334389 -3.3318996e-05 0.00018334389 -5.0008297e-05 0.00018334389 -6.6697598e-05
		 0.00018334389 -8.3327293e-05 0.00018334389 -0.00010001659 0.00018334389 -0.00011664629
		 0.00018334389 -0.00013333559 0.00018334389 -0.00015002489 0.00018334389 -0.00016665459
		 0.00018334389 -0.00018334389 0.00018334389 -0.00019997358 0.00018334389 -0.00021666288
		 0.00018334389 -0.00023335218 0.00018334389 -0.00024998188 0.00018334389 -0.00026667118
		 0.00018334389 -0.00028330088 0.00018334389 -0.00029999018 0.00018334389 -0.00031667948
		 0.00018334389 -0.00033336878 0.00018334389 -0.00034999847 0.00018334389 -0.00036662817
		 0.00018334389 -0.00038331747 0.00018334389 -0.00040000677 0.00018334389 -0.00041669607
		 0.00018334389 -0.00043332577 0.00018334389 -0.00044995546 0.00018334389 -0.00046664476
		 0.00018334389 -0.00048333406 0.00018334389 -0.00050002337 0.00018334389 0.00049999356
		 0.00016668439 0.0004833471 0.00016668439 0.00046667084 0.00016668439 0.00044999272
		 0.00016668439 0.00043331832 0.00016668439 0.00041667372 0.00016668439 0.00039999932
		 0.00016668439 0.00038332492 0.00016668439 0.00036664307 0.00016668439 0.00034999847
		 0.00016668439 0.00033335388 0.00016668439 0.00031664968 0.00016668439 0.00029997528
		 0.00016668439 0.00028333068 0.00016668439 0.00026668608 0.00016668439 0.00024998188
		 0.00016668439 0.00023332238 0.00016668439 0.00021666288 0.00016668439 0.00020000339
		 0.00016668439 0.00018334389 0.00016668439 0.00016668439 0.00016668439 0.00014999509
		 0.00016668439 0.00013333559 0.00016668439 0.00011667609 0.00016668439 9.9986792e-05
		 0.00016668439 8.3327293e-05 0.00016668439 6.6667795e-05 0.00016668439 5.0008297e-05
		 0.00016668439 3.3348799e-05 0.00016668439 1.6659498e-05 0.00016668439 0 0.00016668439
		 -1.6689301e-05 0.00016668439 -3.3318996e-05 0.00016668439 -5.0008297e-05 0.00016668439
		 -6.6697598e-05 0.00016668439 -8.3327293e-05 0.00016668439 -0.00010001659 0.00016668439
		 -0.00011664629 0.00016668439 -0.00013333559 0.00016668439 -0.00015002489 0.00016668439
		 -0.00016665459 0.00016668439 -0.00018334389 0.00016668439 -0.00019997358 0.00016668439
		 -0.00021666288 0.00016668439 -0.00023335218 0.00016668439 -0.00024998188 0.00016668439
		 -0.00026667118 0.00016668439 -0.00028330088 0.00016668439 -0.00029999018 0.00016668439
		 -0.00031667948 0.00016668439 -0.00033336878 0.00016668439 -0.00034999847 0.00016668439
		 -0.00036662817 0.00016668439 -0.00038331747 0.00016668439 -0.00040000677 0.00016668439
		 -0.00041669607 0.00016668439 -0.00043332577 0.00016668439 -0.00044995546 0.00016668439
		 -0.00046664476 0.00016668439 -0.00048333406 0.00016668439 -0.00050002337 0.00016668439
		 0.00049999356 0.00014999509 0.0004833471 0.00014999509 0.00046667084 0.00014999509
		 0.00044999272 0.00014999509 0.00043331832 0.00014999509 0.00041667372 0.00014999509
		 0.00039999932 0.00014999509 0.00038332492 0.00014999509 0.00036664307 0.00014999509
		 0.00034999847 0.00014999509 0.00033335388 0.00014999509 0.00031664968 0.00014999509
		 0.00029997528 0.00014999509 0.00028333068 0.00014999509 0.00026668608 0.00014999509
		 0.00024998188 0.00014999509 0.00023332238 0.00014999509 0.00021666288 0.00014999509
		 0.00020000339 0.00014999509 0.00018334389 0.00014999509 0.00016668439 0.00014999509
		 0.00014999509 0.00014999509 0.00013333559 0.00014999509 0.00011667609 0.00014999509
		 9.9986792e-05 0.00014999509 8.3327293e-05 0.00014999509 6.6667795e-05 0.00014999509
		 5.0008297e-05 0.00014999509 3.3348799e-05 0.00014999509 1.6659498e-05 0.00014999509;
	setAttr ".uvtk[1250:1499]" 0 0.00014999509 -1.6689301e-05 0.00014999509 -3.3318996e-05
		 0.00014999509 -5.0008297e-05 0.00014999509 -6.6697598e-05 0.00014999509 -8.3327293e-05
		 0.00014999509 -0.00010001659 0.00014999509 -0.00011664629 0.00014999509 -0.00013333559
		 0.00014999509 -0.00015002489 0.00014999509 -0.00016665459 0.00014999509 -0.00018334389
		 0.00014999509 -0.00019997358 0.00014999509 -0.00021666288 0.00014999509 -0.00023335218
		 0.00014999509 -0.00024998188 0.00014999509 -0.00026667118 0.00014999509 -0.00028330088
		 0.00014999509 -0.00029999018 0.00014999509 -0.00031667948 0.00014999509 -0.00033336878
		 0.00014999509 -0.00034999847 0.00014999509 -0.00036662817 0.00014999509 -0.00038331747
		 0.00014999509 -0.00040000677 0.00014999509 -0.00041669607 0.00014999509 -0.00043332577
		 0.00014999509 -0.00044995546 0.00014999509 -0.00046664476 0.00014999509 -0.00048333406
		 0.00014999509 -0.00050002337 0.00014999509 0.00049999356 0.00013333559 0.0004833471
		 0.00013333559 0.00046667084 0.00013333559 0.00044999272 0.00013333559 0.00043331832
		 0.00013333559 0.00041667372 0.00013333559 0.00039999932 0.00013333559 0.00038332492
		 0.00013333559 0.00036664307 0.00013333559 0.00034999847 0.00013333559 0.00033335388
		 0.00013333559 0.00031664968 0.00013333559 0.00029997528 0.00013333559 0.00028333068
		 0.00013333559 0.00026668608 0.00013333559 0.00024998188 0.00013333559 0.00023332238
		 0.00013333559 0.00021666288 0.00013333559 0.00020000339 0.00013333559 0.00018334389
		 0.00013333559 0.00016668439 0.00013333559 0.00014999509 0.00013333559 0.00013333559
		 0.00013333559 0.00011667609 0.00013333559 9.9986792e-05 0.00013333559 8.3327293e-05
		 0.00013333559 6.6667795e-05 0.00013333559 5.0008297e-05 0.00013333559 3.3348799e-05
		 0.00013333559 1.6659498e-05 0.00013333559 0 0.00013333559 -1.6689301e-05 0.00013333559
		 -3.3318996e-05 0.00013333559 -5.0008297e-05 0.00013333559 -6.6697598e-05 0.00013333559
		 -8.3327293e-05 0.00013333559 -0.00010001659 0.00013333559 -0.00011664629 0.00013333559
		 -0.00013333559 0.00013333559 -0.00015002489 0.00013333559 -0.00016665459 0.00013333559
		 -0.00018334389 0.00013333559 -0.00019997358 0.00013333559 -0.00021666288 0.00013333559
		 -0.00023335218 0.00013333559 -0.00024998188 0.00013333559 -0.00026667118 0.00013333559
		 -0.00028330088 0.00013333559 -0.00029999018 0.00013333559 -0.00031667948 0.00013333559
		 -0.00033336878 0.00013333559 -0.00034999847 0.00013333559 -0.00036662817 0.00013333559
		 -0.00038331747 0.00013333559 -0.00040000677 0.00013333559 -0.00041669607 0.00013333559
		 -0.00043332577 0.00013333559 -0.00044995546 0.00013333559 -0.00046664476 0.00013333559
		 -0.00048333406 0.00013333559 -0.00050002337 0.00013333559 0.00049999356 0.00011667609
		 0.0004833471 0.00011667609 0.00046667084 0.00011667609 0.00044999272 0.00011667609
		 0.00043331832 0.00011667609 0.00041667372 0.00011667609 0.00039999932 0.00011667609
		 0.00038332492 0.00011667609 0.00036664307 0.00011667609 0.00034999847 0.00011667609
		 0.00033335388 0.00011667609 0.00031664968 0.00011667609 0.00029997528 0.00011667609
		 0.00028333068 0.00011667609 0.00026668608 0.00011667609 0.00024998188 0.00011667609
		 0.00023332238 0.00011667609 0.00021666288 0.00011667609 0.00020000339 0.00011667609
		 0.00018334389 0.00011667609 0.00016668439 0.00011667609 0.00014999509 0.00011667609
		 0.00013333559 0.00011667609 0.00011667609 0.00011667609 9.9986792e-05 0.00011667609
		 8.3327293e-05 0.00011667609 6.6667795e-05 0.00011667609 5.0008297e-05 0.00011667609
		 3.3348799e-05 0.00011667609 1.6659498e-05 0.00011667609 0 0.00011667609 -1.6689301e-05
		 0.00011667609 -3.3318996e-05 0.00011667609 -5.0008297e-05 0.00011667609 -6.6697598e-05
		 0.00011667609 -8.3327293e-05 0.00011667609 -0.00010001659 0.00011667609 -0.00011664629
		 0.00011667609 -0.00013333559 0.00011667609 -0.00015002489 0.00011667609 -0.00016665459
		 0.00011667609 -0.00018334389 0.00011667609 -0.00019997358 0.00011667609 -0.00021666288
		 0.00011667609 -0.00023335218 0.00011667609 -0.00024998188 0.00011667609 -0.00026667118
		 0.00011667609 -0.00028330088 0.00011667609 -0.00029999018 0.00011667609 -0.00031667948
		 0.00011667609 -0.00033336878 0.00011667609 -0.00034999847 0.00011667609 -0.00036662817
		 0.00011667609 -0.00038331747 0.00011667609 -0.00040000677 0.00011667609 -0.00041669607
		 0.00011667609 -0.00043332577 0.00011667609 -0.00044995546 0.00011667609 -0.00046664476
		 0.00011667609 -0.00048333406 0.00011667609 -0.00050002337 0.00011667609 0.00049999356
		 9.9986792e-05 0.0004833471 9.9986792e-05 0.00046667084 9.9986792e-05 0.00044999272
		 9.9986792e-05 0.00043331832 9.9986792e-05 0.00041667372 9.9986792e-05 0.00039999932
		 9.9986792e-05 0.00038332492 9.9986792e-05 0.00036664307 9.9986792e-05 0.00034999847
		 9.9986792e-05 0.00033335388 9.9986792e-05 0.00031664968 9.9986792e-05 0.00029997528
		 9.9986792e-05 0.00028333068 9.9986792e-05 0.00026668608 9.9986792e-05 0.00024998188
		 9.9986792e-05 0.00023332238 9.9986792e-05 0.00021666288 9.9986792e-05 0.00020000339
		 9.9986792e-05 0.00018334389 9.9986792e-05 0.00016668439 9.9986792e-05 0.00014999509
		 9.9986792e-05 0.00013333559 9.9986792e-05 0.00011667609 9.9986792e-05 9.9986792e-05
		 9.9986792e-05 8.3327293e-05 9.9986792e-05 6.6667795e-05 9.9986792e-05 5.0008297e-05
		 9.9986792e-05 3.3348799e-05 9.9986792e-05 1.6659498e-05 9.9986792e-05 0 9.9986792e-05
		 -1.6689301e-05 9.9986792e-05 -3.3318996e-05 9.9986792e-05 -5.0008297e-05 9.9986792e-05
		 -6.6697598e-05 9.9986792e-05 -8.3327293e-05 9.9986792e-05 -0.00010001659 9.9986792e-05
		 -0.00011664629 9.9986792e-05 -0.00013333559 9.9986792e-05 -0.00015002489 9.9986792e-05
		 -0.00016665459 9.9986792e-05 -0.00018334389 9.9986792e-05 -0.00019997358 9.9986792e-05
		 -0.00021666288 9.9986792e-05 -0.00023335218 9.9986792e-05 -0.00024998188 9.9986792e-05
		 -0.00026667118 9.9986792e-05 -0.00028330088 9.9986792e-05 -0.00029999018 9.9986792e-05
		 -0.00031667948 9.9986792e-05 -0.00033336878 9.9986792e-05 -0.00034999847 9.9986792e-05
		 -0.00036662817 9.9986792e-05 -0.00038331747 9.9986792e-05 -0.00040000677 9.9986792e-05
		 -0.00041669607 9.9986792e-05 -0.00043332577 9.9986792e-05 -0.00044995546 9.9986792e-05
		 -0.00046664476 9.9986792e-05 -0.00048333406 9.9986792e-05 -0.00050002337 9.9986792e-05
		 0.00049999356 8.3327293e-05 0.0004833471 8.3327293e-05 0.00046667084 8.3327293e-05
		 0.00044999272 8.3327293e-05 0.00043331832 8.3327293e-05 0.00041667372 8.3327293e-05
		 0.00039999932 8.3327293e-05 0.00038332492 8.3327293e-05 0.00036664307 8.3327293e-05
		 0.00034999847 8.3327293e-05 0.00033335388 8.3327293e-05 0.00031664968 8.3327293e-05
		 0.00029997528 8.3327293e-05 0.00028333068 8.3327293e-05 0.00026668608 8.3327293e-05
		 0.00024998188 8.3327293e-05 0.00023332238 8.3327293e-05 0.00021666288 8.3327293e-05
		 0.00020000339 8.3327293e-05 0.00018334389 8.3327293e-05 0.00016668439 8.3327293e-05
		 0.00014999509 8.3327293e-05 0.00013333559 8.3327293e-05 0.00011667609 8.3327293e-05
		 9.9986792e-05 8.3327293e-05 8.3327293e-05 8.3327293e-05 6.6667795e-05 8.3327293e-05
		 5.0008297e-05 8.3327293e-05 3.3348799e-05 8.3327293e-05 1.6659498e-05 8.3327293e-05
		 0 8.3327293e-05 -1.6689301e-05 8.3327293e-05 -3.3318996e-05 8.3327293e-05 -5.0008297e-05
		 8.3327293e-05 -6.6697598e-05 8.3327293e-05 -8.3327293e-05 8.3327293e-05;
	setAttr ".uvtk[1500:1749]" -0.00010001659 8.3327293e-05 -0.00011664629 8.3327293e-05
		 -0.00013333559 8.3327293e-05 -0.00015002489 8.3327293e-05 -0.00016665459 8.3327293e-05
		 -0.00018334389 8.3327293e-05 -0.00019997358 8.3327293e-05 -0.00021666288 8.3327293e-05
		 -0.00023335218 8.3327293e-05 -0.00024998188 8.3327293e-05 -0.00026667118 8.3327293e-05
		 -0.00028330088 8.3327293e-05 -0.00029999018 8.3327293e-05 -0.00031667948 8.3327293e-05
		 -0.00033336878 8.3327293e-05 -0.00034999847 8.3327293e-05 -0.00036662817 8.3327293e-05
		 -0.00038331747 8.3327293e-05 -0.00040000677 8.3327293e-05 -0.00041669607 8.3327293e-05
		 -0.00043332577 8.3327293e-05 -0.00044995546 8.3327293e-05 -0.00046664476 8.3327293e-05
		 -0.00048333406 8.3327293e-05 -0.00050002337 8.3327293e-05 0.00049999356 6.6667795e-05
		 0.0004833471 6.6667795e-05 0.00046667084 6.6667795e-05 0.00044999272 6.6667795e-05
		 0.00043331832 6.6667795e-05 0.00041667372 6.6667795e-05 0.00039999932 6.6667795e-05
		 0.00038332492 6.6667795e-05 0.00036664307 6.6667795e-05 0.00034999847 6.6667795e-05
		 0.00033335388 6.6667795e-05 0.00031664968 6.6667795e-05 0.00029997528 6.6667795e-05
		 0.00028333068 6.6667795e-05 0.00026668608 6.6667795e-05 0.00024998188 6.6667795e-05
		 0.00023332238 6.6667795e-05 0.00021666288 6.6667795e-05 0.00020000339 6.6667795e-05
		 0.00018334389 6.6667795e-05 0.00016668439 6.6667795e-05 0.00014999509 6.6667795e-05
		 0.00013333559 6.6667795e-05 0.00011667609 6.6667795e-05 9.9986792e-05 6.6667795e-05
		 8.3327293e-05 6.6667795e-05 6.6667795e-05 6.6667795e-05 5.0008297e-05 6.6667795e-05
		 3.3348799e-05 6.6667795e-05 1.6659498e-05 6.6667795e-05 0 6.6667795e-05 -1.6689301e-05
		 6.6667795e-05 -3.3318996e-05 6.6667795e-05 -5.0008297e-05 6.6667795e-05 -6.6697598e-05
		 6.6667795e-05 -8.3327293e-05 6.6667795e-05 -0.00010001659 6.6667795e-05 -0.00011664629
		 6.6667795e-05 -0.00013333559 6.6667795e-05 -0.00015002489 6.6667795e-05 -0.00016665459
		 6.6667795e-05 -0.00018334389 6.6667795e-05 -0.00019997358 6.6667795e-05 -0.00021666288
		 6.6667795e-05 -0.00023335218 6.6667795e-05 -0.00024998188 6.6667795e-05 -0.00026667118
		 6.6667795e-05 -0.00028330088 6.6667795e-05 -0.00029999018 6.6667795e-05 -0.00031667948
		 6.6667795e-05 -0.00033336878 6.6667795e-05 -0.00034999847 6.6667795e-05 -0.00036662817
		 6.6667795e-05 -0.00038331747 6.6667795e-05 -0.00040000677 6.6667795e-05 -0.00041669607
		 6.6667795e-05 -0.00043332577 6.6667795e-05 -0.00044995546 6.6667795e-05 -0.00046664476
		 6.6667795e-05 -0.00048333406 6.6667795e-05 -0.00050002337 6.6667795e-05 0.00049999356
		 5.0008297e-05 0.0004833471 5.0008297e-05 0.00046667084 5.0008297e-05 0.00044999272
		 5.0008297e-05 0.00043331832 5.0008297e-05 0.00041667372 5.0008297e-05 0.00039999932
		 5.0008297e-05 0.00038332492 5.0008297e-05 0.00036664307 5.0008297e-05 0.00034999847
		 5.0008297e-05 0.00033335388 5.0008297e-05 0.00031664968 5.0008297e-05 0.00029997528
		 5.0008297e-05 0.00028333068 5.0008297e-05 0.00026668608 5.0008297e-05 0.00024998188
		 5.0008297e-05 0.00023332238 5.0008297e-05 0.00021666288 5.0008297e-05 0.00020000339
		 5.0008297e-05 0.00018334389 5.0008297e-05 0.00016668439 5.0008297e-05 0.00014999509
		 5.0008297e-05 0.00013333559 5.0008297e-05 0.00011667609 5.0008297e-05 9.9986792e-05
		 5.0008297e-05 8.3327293e-05 5.0008297e-05 6.6667795e-05 5.0008297e-05 5.0008297e-05
		 5.0008297e-05 3.3348799e-05 5.0008297e-05 1.6659498e-05 5.0008297e-05 0 5.0008297e-05
		 -1.6689301e-05 5.0008297e-05 -3.3318996e-05 5.0008297e-05 -5.0008297e-05 5.0008297e-05
		 -6.6697598e-05 5.0008297e-05 -8.3327293e-05 5.0008297e-05 -0.00010001659 5.0008297e-05
		 -0.00011664629 5.0008297e-05 -0.00013333559 5.0008297e-05 -0.00015002489 5.0008297e-05
		 -0.00016665459 5.0008297e-05 -0.00018334389 5.0008297e-05 -0.00019997358 5.0008297e-05
		 -0.00021666288 5.0008297e-05 -0.00023335218 5.0008297e-05 -0.00024998188 5.0008297e-05
		 -0.00026667118 5.0008297e-05 -0.00028330088 5.0008297e-05 -0.00029999018 5.0008297e-05
		 -0.00031667948 5.0008297e-05 -0.00033336878 5.0008297e-05 -0.00034999847 5.0008297e-05
		 -0.00036662817 5.0008297e-05 -0.00038331747 5.0008297e-05 -0.00040000677 5.0008297e-05
		 -0.00041669607 5.0008297e-05 -0.00043332577 5.0008297e-05 -0.00044995546 5.0008297e-05
		 -0.00046664476 5.0008297e-05 -0.00048333406 5.0008297e-05 -0.00050002337 5.0008297e-05
		 0.00049999356 3.3348799e-05 0.0004833471 3.3348799e-05 0.00046667084 3.3348799e-05
		 0.00044999272 3.3348799e-05 0.00043331832 3.3348799e-05 0.00041667372 3.3348799e-05
		 0.00039999932 3.3348799e-05 0.00038332492 3.3348799e-05 0.00036664307 3.3348799e-05
		 0.00034999847 3.3348799e-05 0.00033335388 3.3348799e-05 0.00031664968 3.3348799e-05
		 0.00029997528 3.3348799e-05 0.00028333068 3.3348799e-05 0.00026668608 3.3348799e-05
		 0.00024998188 3.3348799e-05 0.00023332238 3.3348799e-05 0.00021666288 3.3348799e-05
		 0.00020000339 3.3348799e-05 0.00018334389 3.3348799e-05 0.00016668439 3.3348799e-05
		 0.00014999509 3.3348799e-05 0.00013333559 3.3348799e-05 0.00011667609 3.3348799e-05
		 9.9986792e-05 3.3348799e-05 8.3327293e-05 3.3348799e-05 6.6667795e-05 3.3348799e-05
		 5.0008297e-05 3.3348799e-05 3.3348799e-05 3.3348799e-05 1.6659498e-05 3.3348799e-05
		 0 3.3348799e-05 -1.6689301e-05 3.3348799e-05 -3.3318996e-05 3.3348799e-05 -5.0008297e-05
		 3.3348799e-05 -6.6697598e-05 3.3348799e-05 -8.3327293e-05 3.3348799e-05 -0.00010001659
		 3.3348799e-05 -0.00011664629 3.3348799e-05 -0.00013333559 3.3348799e-05 -0.00015002489
		 3.3348799e-05 -0.00016665459 3.3348799e-05 -0.00018334389 3.3348799e-05 -0.00019997358
		 3.3348799e-05 -0.00021666288 3.3348799e-05 -0.00023335218 3.3348799e-05 -0.00024998188
		 3.3348799e-05 -0.00026667118 3.3348799e-05 -0.00028330088 3.3348799e-05 -0.00029999018
		 3.3348799e-05 -0.00031667948 3.3348799e-05 -0.00033336878 3.3348799e-05 -0.00034999847
		 3.3348799e-05 -0.00036662817 3.3348799e-05 -0.00038331747 3.3348799e-05 -0.00040000677
		 3.3348799e-05 -0.00041669607 3.3348799e-05 -0.00043332577 3.3348799e-05 -0.00044995546
		 3.3348799e-05 -0.00046664476 3.3348799e-05 -0.00048333406 3.3348799e-05 -0.00050002337
		 3.3348799e-05 0.00049999356 1.6659498e-05 0.0004833471 1.6659498e-05 0.00046667084
		 1.6659498e-05 0.00044999272 1.6659498e-05 0.00043331832 1.6659498e-05 0.00041667372
		 1.6659498e-05 0.00039999932 1.6659498e-05 0.00038332492 1.6659498e-05 0.00036664307
		 1.6659498e-05 0.00034999847 1.6659498e-05 0.00033335388 1.6659498e-05 0.00031664968
		 1.6659498e-05 0.00029997528 1.6659498e-05 0.00028333068 1.6659498e-05 0.00026668608
		 1.6659498e-05 0.00024998188 1.6659498e-05 0.00023332238 1.6659498e-05 0.00021666288
		 1.6659498e-05 0.00020000339 1.6659498e-05 0.00018334389 1.6659498e-05 0.00016668439
		 1.6659498e-05 0.00014999509 1.6659498e-05 0.00013333559 1.6659498e-05 0.00011667609
		 1.6659498e-05 9.9986792e-05 1.6659498e-05 8.3327293e-05 1.6659498e-05 6.6667795e-05
		 1.6659498e-05 5.0008297e-05 1.6659498e-05 3.3348799e-05 1.6659498e-05 1.6659498e-05
		 1.6659498e-05 0 1.6659498e-05 -1.6689301e-05 1.6659498e-05 -3.3318996e-05 1.6659498e-05
		 -5.0008297e-05 1.6659498e-05 -6.6697598e-05 1.6659498e-05 -8.3327293e-05 1.6659498e-05
		 -0.00010001659 1.6659498e-05 -0.00011664629 1.6659498e-05 -0.00013333559 1.6659498e-05
		 -0.00015002489 1.6659498e-05 -0.00016665459 1.6659498e-05 -0.00018334389 1.6659498e-05;
	setAttr ".uvtk[1750:1999]" -0.00019997358 1.6659498e-05 -0.00021666288 1.6659498e-05
		 -0.00023335218 1.6659498e-05 -0.00024998188 1.6659498e-05 -0.00026667118 1.6659498e-05
		 -0.00028330088 1.6659498e-05 -0.00029999018 1.6659498e-05 -0.00031667948 1.6659498e-05
		 -0.00033336878 1.6659498e-05 -0.00034999847 1.6659498e-05 -0.00036662817 1.6659498e-05
		 -0.00038331747 1.6659498e-05 -0.00040000677 1.6659498e-05 -0.00041669607 1.6659498e-05
		 -0.00043332577 1.6659498e-05 -0.00044995546 1.6659498e-05 -0.00046664476 1.6659498e-05
		 -0.00048333406 1.6659498e-05 -0.00050002337 1.6659498e-05 0.00049999356 0 0.0004833471
		 0 0.00046667084 0 0.00044999272 0 0.00043331832 0 0.00041667372 0 0.00039999932 0
		 0.00038332492 0 0.00036664307 0 0.00034999847 0 0.00033335388 0 0.00031664968 0 0.00029997528
		 0 0.00028333068 0 0.00026668608 0 0.00024998188 0 0.00023332238 0 0.00021666288 0
		 0.00020000339 0 0.00018334389 0 0.00016668439 0 0.00014999509 0 0.00013333559 0 0.00011667609
		 0 9.9986792e-05 0 8.3327293e-05 0 6.6667795e-05 0 5.0008297e-05 0 3.3348799e-05 0
		 1.6659498e-05 0 0 0 -1.6689301e-05 0 -3.3318996e-05 0 -5.0008297e-05 0 -6.6697598e-05
		 0 -8.3327293e-05 0 -0.00010001659 0 -0.00011664629 0 -0.00013333559 0 -0.00015002489
		 0 -0.00016665459 0 -0.00018334389 0 -0.00019997358 0 -0.00021666288 0 -0.00023335218
		 0 -0.00024998188 0 -0.00026667118 0 -0.00028330088 0 -0.00029999018 0 -0.00031667948
		 0 -0.00033336878 0 -0.00034999847 0 -0.00036662817 0 -0.00038331747 0 -0.00040000677
		 0 -0.00041669607 0 -0.00043332577 0 -0.00044995546 0 -0.00046664476 0 -0.00048333406
		 0 -0.00050002337 0 0.00049999356 -1.6689301e-05 0.0004833471 -1.6689301e-05 0.00046667084
		 -1.6689301e-05 0.00044999272 -1.6689301e-05 0.00043331832 -1.6689301e-05 0.00041667372
		 -1.6689301e-05 0.00039999932 -1.6689301e-05 0.00038332492 -1.6689301e-05 0.00036664307
		 -1.6689301e-05 0.00034999847 -1.6689301e-05 0.00033335388 -1.6689301e-05 0.00031664968
		 -1.6689301e-05 0.00029997528 -1.6689301e-05 0.00028333068 -1.6689301e-05 0.00026668608
		 -1.6689301e-05 0.00024998188 -1.6689301e-05 0.00023332238 -1.6689301e-05 0.00021666288
		 -1.6689301e-05 0.00020000339 -1.6689301e-05 0.00018334389 -1.6689301e-05 0.00016668439
		 -1.6689301e-05 0.00014999509 -1.6689301e-05 0.00013333559 -1.6689301e-05 0.00011667609
		 -1.6689301e-05 9.9986792e-05 -1.6689301e-05 8.3327293e-05 -1.6689301e-05 6.6667795e-05
		 -1.6689301e-05 5.0008297e-05 -1.6689301e-05 3.3348799e-05 -1.6689301e-05 1.6659498e-05
		 -1.6689301e-05 0 -1.6689301e-05 -1.6689301e-05 -1.6689301e-05 -3.3318996e-05 -1.6689301e-05
		 -5.0008297e-05 -1.6689301e-05 -6.6697598e-05 -1.6689301e-05 -8.3327293e-05 -1.6689301e-05
		 -0.00010001659 -1.6689301e-05 -0.00011664629 -1.6689301e-05 -0.00013333559 -1.6689301e-05
		 -0.00015002489 -1.6689301e-05 -0.00016665459 -1.6689301e-05 -0.00018334389 -1.6689301e-05
		 -0.00019997358 -1.6689301e-05 -0.00021666288 -1.6689301e-05 -0.00023335218 -1.6689301e-05
		 -0.00024998188 -1.6689301e-05 -0.00026667118 -1.6689301e-05 -0.00028330088 -1.6689301e-05
		 -0.00029999018 -1.6689301e-05 -0.00031667948 -1.6689301e-05 -0.00033336878 -1.6689301e-05
		 -0.00034999847 -1.6689301e-05 -0.00036662817 -1.6689301e-05 -0.00038331747 -1.6689301e-05
		 -0.00040000677 -1.6689301e-05 -0.00041669607 -1.6689301e-05 -0.00043332577 -1.6689301e-05
		 -0.00044995546 -1.6689301e-05 -0.00046664476 -1.6689301e-05 -0.00048333406 -1.6689301e-05
		 -0.00050002337 -1.6689301e-05 0.00049999356 -3.3318996e-05 0.0004833471 -3.3318996e-05
		 0.00046667084 -3.3318996e-05 0.00044999272 -3.3318996e-05 0.00043331832 -3.3318996e-05
		 0.00041667372 -3.3318996e-05 0.00039999932 -3.3318996e-05 0.00038332492 -3.3318996e-05
		 0.00036664307 -3.3318996e-05 0.00034999847 -3.3318996e-05 0.00033335388 -3.3318996e-05
		 0.00031664968 -3.3318996e-05 0.00029997528 -3.3318996e-05 0.00028333068 -3.3318996e-05
		 0.00026668608 -3.3318996e-05 0.00024998188 -3.3318996e-05 0.00023332238 -3.3318996e-05
		 0.00021666288 -3.3318996e-05 0.00020000339 -3.3318996e-05 0.00018334389 -3.3318996e-05
		 0.00016668439 -3.3318996e-05 0.00014999509 -3.3318996e-05 0.00013333559 -3.3318996e-05
		 0.00011667609 -3.3318996e-05 9.9986792e-05 -3.3318996e-05 8.3327293e-05 -3.3318996e-05
		 6.6667795e-05 -3.3318996e-05 5.0008297e-05 -3.3318996e-05 3.3348799e-05 -3.3318996e-05
		 1.6659498e-05 -3.3318996e-05 0 -3.3318996e-05 -1.6689301e-05 -3.3318996e-05 -3.3318996e-05
		 -3.3318996e-05 -5.0008297e-05 -3.3318996e-05 -6.6697598e-05 -3.3318996e-05 -8.3327293e-05
		 -3.3318996e-05 -0.00010001659 -3.3318996e-05 -0.00011664629 -3.3318996e-05 -0.00013333559
		 -3.3318996e-05 -0.00015002489 -3.3318996e-05 -0.00016665459 -3.3318996e-05 -0.00018334389
		 -3.3318996e-05 -0.00019997358 -3.3318996e-05 -0.00021666288 -3.3318996e-05 -0.00023335218
		 -3.3318996e-05 -0.00024998188 -3.3318996e-05 -0.00026667118 -3.3318996e-05 -0.00028330088
		 -3.3318996e-05 -0.00029999018 -3.3318996e-05 -0.00031667948 -3.3318996e-05 -0.00033336878
		 -3.3318996e-05 -0.00034999847 -3.3318996e-05 -0.00036662817 -3.3318996e-05 -0.00038331747
		 -3.3318996e-05 -0.00040000677 -3.3318996e-05 -0.00041669607 -3.3318996e-05 -0.00043332577
		 -3.3318996e-05 -0.00044995546 -3.3318996e-05 -0.00046664476 -3.3318996e-05 -0.00048333406
		 -3.3318996e-05 -0.00050002337 -3.3318996e-05 0.00049999356 -5.0008297e-05 0.0004833471
		 -5.0008297e-05 0.00046667084 -5.0008297e-05 0.00044999272 -5.0008297e-05 0.00043331832
		 -5.0008297e-05 0.00041667372 -5.0008297e-05 0.00039999932 -5.0008297e-05 0.00038332492
		 -5.0008297e-05 0.00036664307 -5.0008297e-05 0.00034999847 -5.0008297e-05 0.00033335388
		 -5.0008297e-05 0.00031664968 -5.0008297e-05 0.00029997528 -5.0008297e-05 0.00028333068
		 -5.0008297e-05 0.00026668608 -5.0008297e-05 0.00024998188 -5.0008297e-05 0.00023332238
		 -5.0008297e-05 0.00021666288 -5.0008297e-05 0.00020000339 -5.0008297e-05 0.00018334389
		 -5.0008297e-05 0.00016668439 -5.0008297e-05 0.00014999509 -5.0008297e-05 0.00013333559
		 -5.0008297e-05 0.00011667609 -5.0008297e-05 9.9986792e-05 -5.0008297e-05 8.3327293e-05
		 -5.0008297e-05 6.6667795e-05 -5.0008297e-05 5.0008297e-05 -5.0008297e-05 3.3348799e-05
		 -5.0008297e-05 1.6659498e-05 -5.0008297e-05 0 -5.0008297e-05 -1.6689301e-05 -5.0008297e-05
		 -3.3318996e-05 -5.0008297e-05 -5.0008297e-05 -5.0008297e-05 -6.6697598e-05 -5.0008297e-05
		 -8.3327293e-05 -5.0008297e-05 -0.00010001659 -5.0008297e-05 -0.00011664629 -5.0008297e-05
		 -0.00013333559 -5.0008297e-05 -0.00015002489 -5.0008297e-05 -0.00016665459 -5.0008297e-05
		 -0.00018334389 -5.0008297e-05 -0.00019997358 -5.0008297e-05 -0.00021666288 -5.0008297e-05
		 -0.00023335218 -5.0008297e-05 -0.00024998188 -5.0008297e-05 -0.00026667118 -5.0008297e-05
		 -0.00028330088 -5.0008297e-05;
	setAttr ".uvtk[2000:2249]" -0.00029999018 -5.0008297e-05 -0.00031667948 -5.0008297e-05
		 -0.00033336878 -5.0008297e-05 -0.00034999847 -5.0008297e-05 -0.00036662817 -5.0008297e-05
		 -0.00038331747 -5.0008297e-05 -0.00040000677 -5.0008297e-05 -0.00041669607 -5.0008297e-05
		 -0.00043332577 -5.0008297e-05 -0.00044995546 -5.0008297e-05 -0.00046664476 -5.0008297e-05
		 -0.00048333406 -5.0008297e-05 -0.00050002337 -5.0008297e-05 0.00049999356 -6.6697598e-05
		 0.0004833471 -6.6697598e-05 0.00046667084 -6.6697598e-05 0.00044999272 -6.6697598e-05
		 0.00043331832 -6.6697598e-05 0.00041667372 -6.6697598e-05 0.00039999932 -6.6697598e-05
		 0.00038332492 -6.6697598e-05 0.00036664307 -6.6697598e-05 0.00034999847 -6.6697598e-05
		 0.00033335388 -6.6697598e-05 0.00031664968 -6.6697598e-05 0.00029997528 -6.6697598e-05
		 0.00028333068 -6.6697598e-05 0.00026668608 -6.6697598e-05 0.00024998188 -6.6697598e-05
		 0.00023332238 -6.6697598e-05 0.00021666288 -6.6697598e-05 0.00020000339 -6.6697598e-05
		 0.00018334389 -6.6697598e-05 0.00016668439 -6.6697598e-05 0.00014999509 -6.6697598e-05
		 0.00013333559 -6.6697598e-05 0.00011667609 -6.6697598e-05 9.9986792e-05 -6.6697598e-05
		 8.3327293e-05 -6.6697598e-05 6.6667795e-05 -6.6697598e-05 5.0008297e-05 -6.6697598e-05
		 3.3348799e-05 -6.6697598e-05 1.6659498e-05 -6.6697598e-05 0 -6.6697598e-05 -1.6689301e-05
		 -6.6697598e-05 -3.3318996e-05 -6.6697598e-05 -5.0008297e-05 -6.6697598e-05 -6.6697598e-05
		 -6.6697598e-05 -8.3327293e-05 -6.6697598e-05 -0.00010001659 -6.6697598e-05 -0.00011664629
		 -6.6697598e-05 -0.00013333559 -6.6697598e-05 -0.00015002489 -6.6697598e-05 -0.00016665459
		 -6.6697598e-05 -0.00018334389 -6.6697598e-05 -0.00019997358 -6.6697598e-05 -0.00021666288
		 -6.6697598e-05 -0.00023335218 -6.6697598e-05 -0.00024998188 -6.6697598e-05 -0.00026667118
		 -6.6697598e-05 -0.00028330088 -6.6697598e-05 -0.00029999018 -6.6697598e-05 -0.00031667948
		 -6.6697598e-05 -0.00033336878 -6.6697598e-05 -0.00034999847 -6.6697598e-05 -0.00036662817
		 -6.6697598e-05 -0.00038331747 -6.6697598e-05 -0.00040000677 -6.6697598e-05 -0.00041669607
		 -6.6697598e-05 -0.00043332577 -6.6697598e-05 -0.00044995546 -6.6697598e-05 -0.00046664476
		 -6.6697598e-05 -0.00048333406 -6.6697598e-05 -0.00050002337 -6.6697598e-05 0.00049999356
		 -8.3327293e-05 0.0004833471 -8.3327293e-05 0.00046667084 -8.3327293e-05 0.00044999272
		 -8.3327293e-05 0.00043331832 -8.3327293e-05 0.00041667372 -8.3327293e-05 0.00039999932
		 -8.3327293e-05 0.00038332492 -8.3327293e-05 0.00036664307 -8.3327293e-05 0.00034999847
		 -8.3327293e-05 0.00033335388 -8.3327293e-05 0.00031664968 -8.3327293e-05 0.00029997528
		 -8.3327293e-05 0.00028333068 -8.3327293e-05 0.00026668608 -8.3327293e-05 0.00024998188
		 -8.3327293e-05 0.00023332238 -8.3327293e-05 0.00021666288 -8.3327293e-05 0.00020000339
		 -8.3327293e-05 0.00018334389 -8.3327293e-05 0.00016668439 -8.3327293e-05 0.00014999509
		 -8.3327293e-05 0.00013333559 -8.3327293e-05 0.00011667609 -8.3327293e-05 9.9986792e-05
		 -8.3327293e-05 8.3327293e-05 -8.3327293e-05 6.6667795e-05 -8.3327293e-05 5.0008297e-05
		 -8.3327293e-05 3.3348799e-05 -8.3327293e-05 1.6659498e-05 -8.3327293e-05 0 -8.3327293e-05
		 -1.6689301e-05 -8.3327293e-05 -3.3318996e-05 -8.3327293e-05 -5.0008297e-05 -8.3327293e-05
		 -6.6697598e-05 -8.3327293e-05 -8.3327293e-05 -8.3327293e-05 -0.00010001659 -8.3327293e-05
		 -0.00011664629 -8.3327293e-05 -0.00013333559 -8.3327293e-05 -0.00015002489 -8.3327293e-05
		 -0.00016665459 -8.3327293e-05 -0.00018334389 -8.3327293e-05 -0.00019997358 -8.3327293e-05
		 -0.00021666288 -8.3327293e-05 -0.00023335218 -8.3327293e-05 -0.00024998188 -8.3327293e-05
		 -0.00026667118 -8.3327293e-05 -0.00028330088 -8.3327293e-05 -0.00029999018 -8.3327293e-05
		 -0.00031667948 -8.3327293e-05 -0.00033336878 -8.3327293e-05 -0.00034999847 -8.3327293e-05
		 -0.00036662817 -8.3327293e-05 -0.00038331747 -8.3327293e-05 -0.00040000677 -8.3327293e-05
		 -0.00041669607 -8.3327293e-05 -0.00043332577 -8.3327293e-05 -0.00044995546 -8.3327293e-05
		 -0.00046664476 -8.3327293e-05 -0.00048333406 -8.3327293e-05 -0.00050002337 -8.3327293e-05
		 0.00049999356 -0.00010001659 0.0004833471 -0.00010001659 0.00046667084 -0.00010001659
		 0.00044999272 -0.00010001659 0.00043331832 -0.00010001659 0.00041667372 -0.00010001659
		 0.00039999932 -0.00010001659 0.00038332492 -0.00010001659 0.00036664307 -0.00010001659
		 0.00034999847 -0.00010001659 0.00033335388 -0.00010001659 0.00031664968 -0.00010001659
		 0.00029997528 -0.00010001659 0.00028333068 -0.00010001659 0.00026668608 -0.00010001659
		 0.00024998188 -0.00010001659 0.00023332238 -0.00010001659 0.00021666288 -0.00010001659
		 0.00020000339 -0.00010001659 0.00018334389 -0.00010001659 0.00016668439 -0.00010001659
		 0.00014999509 -0.00010001659 0.00013333559 -0.00010001659 0.00011667609 -0.00010001659
		 9.9986792e-05 -0.00010001659 8.3327293e-05 -0.00010001659 6.6667795e-05 -0.00010001659
		 5.0008297e-05 -0.00010001659 3.3348799e-05 -0.00010001659 1.6659498e-05 -0.00010001659
		 0 -0.00010001659 -1.6689301e-05 -0.00010001659 -3.3318996e-05 -0.00010001659 -5.0008297e-05
		 -0.00010001659 -6.6697598e-05 -0.00010001659 -8.3327293e-05 -0.00010001659 -0.00010001659
		 -0.00010001659 -0.00011664629 -0.00010001659 -0.00013333559 -0.00010001659 -0.00015002489
		 -0.00010001659 -0.00016665459 -0.00010001659 -0.00018334389 -0.00010001659 -0.00019997358
		 -0.00010001659 -0.00021666288 -0.00010001659 -0.00023335218 -0.00010001659 -0.00024998188
		 -0.00010001659 -0.00026667118 -0.00010001659 -0.00028330088 -0.00010001659 -0.00029999018
		 -0.00010001659 -0.00031667948 -0.00010001659 -0.00033336878 -0.00010001659 -0.00034999847
		 -0.00010001659 -0.00036662817 -0.00010001659 -0.00038331747 -0.00010001659 -0.00040000677
		 -0.00010001659 -0.00041669607 -0.00010001659 -0.00043332577 -0.00010001659 -0.00044995546
		 -0.00010001659 -0.00046664476 -0.00010001659 -0.00048333406 -0.00010001659 -0.00050002337
		 -0.00010001659 0.00049999356 -0.00011664629 0.0004833471 -0.00011664629 0.00046667084
		 -0.00011664629 0.00044999272 -0.00011664629 0.00043331832 -0.00011664629 0.00041667372
		 -0.00011664629 0.00039999932 -0.00011664629 0.00038332492 -0.00011664629 0.00036664307
		 -0.00011664629 0.00034999847 -0.00011664629 0.00033335388 -0.00011664629 0.00031664968
		 -0.00011664629 0.00029997528 -0.00011664629 0.00028333068 -0.00011664629 0.00026668608
		 -0.00011664629 0.00024998188 -0.00011664629 0.00023332238 -0.00011664629 0.00021666288
		 -0.00011664629 0.00020000339 -0.00011664629 0.00018334389 -0.00011664629 0.00016668439
		 -0.00011664629 0.00014999509 -0.00011664629 0.00013333559 -0.00011664629 0.00011667609
		 -0.00011664629 9.9986792e-05 -0.00011664629 8.3327293e-05 -0.00011664629 6.6667795e-05
		 -0.00011664629 5.0008297e-05 -0.00011664629 3.3348799e-05 -0.00011664629 1.6659498e-05
		 -0.00011664629 0 -0.00011664629 -1.6689301e-05 -0.00011664629 -3.3318996e-05 -0.00011664629
		 -5.0008297e-05 -0.00011664629 -6.6697598e-05 -0.00011664629 -8.3327293e-05 -0.00011664629
		 -0.00010001659 -0.00011664629 -0.00011664629 -0.00011664629 -0.00013333559 -0.00011664629
		 -0.00015002489 -0.00011664629 -0.00016665459 -0.00011664629 -0.00018334389 -0.00011664629
		 -0.00019997358 -0.00011664629 -0.00021666288 -0.00011664629 -0.00023335218 -0.00011664629
		 -0.00024998188 -0.00011664629 -0.00026667118 -0.00011664629 -0.00028330088 -0.00011664629
		 -0.00029999018 -0.00011664629 -0.00031667948 -0.00011664629 -0.00033336878 -0.00011664629
		 -0.00034999847 -0.00011664629 -0.00036662817 -0.00011664629 -0.00038331747 -0.00011664629;
	setAttr ".uvtk[2250:2499]" -0.00040000677 -0.00011664629 -0.00041669607 -0.00011664629
		 -0.00043332577 -0.00011664629 -0.00044995546 -0.00011664629 -0.00046664476 -0.00011664629
		 -0.00048333406 -0.00011664629 -0.00050002337 -0.00011664629 0.00049999356 -0.00013333559
		 0.0004833471 -0.00013333559 0.00046667084 -0.00013333559 0.00044999272 -0.00013333559
		 0.00043331832 -0.00013333559 0.00041667372 -0.00013333559 0.00039999932 -0.00013333559
		 0.00038332492 -0.00013333559 0.00036664307 -0.00013333559 0.00034999847 -0.00013333559
		 0.00033335388 -0.00013333559 0.00031664968 -0.00013333559 0.00029997528 -0.00013333559
		 0.00028333068 -0.00013333559 0.00026668608 -0.00013333559 0.00024998188 -0.00013333559
		 0.00023332238 -0.00013333559 0.00021666288 -0.00013333559 0.00020000339 -0.00013333559
		 0.00018334389 -0.00013333559 0.00016668439 -0.00013333559 0.00014999509 -0.00013333559
		 0.00013333559 -0.00013333559 0.00011667609 -0.00013333559 9.9986792e-05 -0.00013333559
		 8.3327293e-05 -0.00013333559 6.6667795e-05 -0.00013333559 5.0008297e-05 -0.00013333559
		 3.3348799e-05 -0.00013333559 1.6659498e-05 -0.00013333559 0 -0.00013333559 -1.6689301e-05
		 -0.00013333559 -3.3318996e-05 -0.00013333559 -5.0008297e-05 -0.00013333559 -6.6697598e-05
		 -0.00013333559 -8.3327293e-05 -0.00013333559 -0.00010001659 -0.00013333559 -0.00011664629
		 -0.00013333559 -0.00013333559 -0.00013333559 -0.00015002489 -0.00013333559 -0.00016665459
		 -0.00013333559 -0.00018334389 -0.00013333559 -0.00019997358 -0.00013333559 -0.00021666288
		 -0.00013333559 -0.00023335218 -0.00013333559 -0.00024998188 -0.00013333559 -0.00026667118
		 -0.00013333559 -0.00028330088 -0.00013333559 -0.00029999018 -0.00013333559 -0.00031667948
		 -0.00013333559 -0.00033336878 -0.00013333559 -0.00034999847 -0.00013333559 -0.00036662817
		 -0.00013333559 -0.00038331747 -0.00013333559 -0.00040000677 -0.00013333559 -0.00041669607
		 -0.00013333559 -0.00043332577 -0.00013333559 -0.00044995546 -0.00013333559 -0.00046664476
		 -0.00013333559 -0.00048333406 -0.00013333559 -0.00050002337 -0.00013333559 0.00049999356
		 -0.00015002489 0.0004833471 -0.00015002489 0.00046667084 -0.00015002489 0.00044999272
		 -0.00015002489 0.00043331832 -0.00015002489 0.00041667372 -0.00015002489 0.00039999932
		 -0.00015002489 0.00038332492 -0.00015002489 0.00036664307 -0.00015002489 0.00034999847
		 -0.00015002489 0.00033335388 -0.00015002489 0.00031664968 -0.00015002489 0.00029997528
		 -0.00015002489 0.00028333068 -0.00015002489 0.00026668608 -0.00015002489 0.00024998188
		 -0.00015002489 0.00023332238 -0.00015002489 0.00021666288 -0.00015002489 0.00020000339
		 -0.00015002489 0.00018334389 -0.00015002489 0.00016668439 -0.00015002489 0.00014999509
		 -0.00015002489 0.00013333559 -0.00015002489 0.00011667609 -0.00015002489 9.9986792e-05
		 -0.00015002489 8.3327293e-05 -0.00015002489 6.6667795e-05 -0.00015002489 5.0008297e-05
		 -0.00015002489 3.3348799e-05 -0.00015002489 1.6659498e-05 -0.00015002489 0 -0.00015002489
		 -1.6689301e-05 -0.00015002489 -3.3318996e-05 -0.00015002489 -5.0008297e-05 -0.00015002489
		 -6.6697598e-05 -0.00015002489 -8.3327293e-05 -0.00015002489 -0.00010001659 -0.00015002489
		 -0.00011664629 -0.00015002489 -0.00013333559 -0.00015002489 -0.00015002489 -0.00015002489
		 -0.00016665459 -0.00015002489 -0.00018334389 -0.00015002489 -0.00019997358 -0.00015002489
		 -0.00021666288 -0.00015002489 -0.00023335218 -0.00015002489 -0.00024998188 -0.00015002489
		 -0.00026667118 -0.00015002489 -0.00028330088 -0.00015002489 -0.00029999018 -0.00015002489
		 -0.00031667948 -0.00015002489 -0.00033336878 -0.00015002489 -0.00034999847 -0.00015002489
		 -0.00036662817 -0.00015002489 -0.00038331747 -0.00015002489 -0.00040000677 -0.00015002489
		 -0.00041669607 -0.00015002489 -0.00043332577 -0.00015002489 -0.00044995546 -0.00015002489
		 -0.00046664476 -0.00015002489 -0.00048333406 -0.00015002489 -0.00050002337 -0.00015002489
		 0.00049999356 -0.00016665459 0.0004833471 -0.00016665459 0.00046667084 -0.00016665459
		 0.00044999272 -0.00016665459 0.00043331832 -0.00016665459 0.00041667372 -0.00016665459
		 0.00039999932 -0.00016665459 0.00038332492 -0.00016665459 0.00036664307 -0.00016665459
		 0.00034999847 -0.00016665459 0.00033335388 -0.00016665459 0.00031664968 -0.00016665459
		 0.00029997528 -0.00016665459 0.00028333068 -0.00016665459 0.00026668608 -0.00016665459
		 0.00024998188 -0.00016665459 0.00023332238 -0.00016665459 0.00021666288 -0.00016665459
		 0.00020000339 -0.00016665459 0.00018334389 -0.00016665459 0.00016668439 -0.00016665459
		 0.00014999509 -0.00016665459 0.00013333559 -0.00016665459 0.00011667609 -0.00016665459
		 9.9986792e-05 -0.00016665459 8.3327293e-05 -0.00016665459 6.6667795e-05 -0.00016665459
		 5.0008297e-05 -0.00016665459 3.3348799e-05 -0.00016665459 1.6659498e-05 -0.00016665459
		 0 -0.00016665459 -1.6689301e-05 -0.00016665459 -3.3318996e-05 -0.00016665459 -5.0008297e-05
		 -0.00016665459 -6.6697598e-05 -0.00016665459 -8.3327293e-05 -0.00016665459 -0.00010001659
		 -0.00016665459 -0.00011664629 -0.00016665459 -0.00013333559 -0.00016665459 -0.00015002489
		 -0.00016665459 -0.00016665459 -0.00016665459 -0.00018334389 -0.00016665459 -0.00019997358
		 -0.00016665459 -0.00021666288 -0.00016665459 -0.00023335218 -0.00016665459 -0.00024998188
		 -0.00016665459 -0.00026667118 -0.00016665459 -0.00028330088 -0.00016665459 -0.00029999018
		 -0.00016665459 -0.00031667948 -0.00016665459 -0.00033336878 -0.00016665459 -0.00034999847
		 -0.00016665459 -0.00036662817 -0.00016665459 -0.00038331747 -0.00016665459 -0.00040000677
		 -0.00016665459 -0.00041669607 -0.00016665459 -0.00043332577 -0.00016665459 -0.00044995546
		 -0.00016665459 -0.00046664476 -0.00016665459 -0.00048333406 -0.00016665459 -0.00050002337
		 -0.00016665459 0.00049999356 -0.00018334389 0.0004833471 -0.00018334389 0.00046667084
		 -0.00018334389 0.00044999272 -0.00018334389 0.00043331832 -0.00018334389 0.00041667372
		 -0.00018334389 0.00039999932 -0.00018334389 0.00038332492 -0.00018334389 0.00036664307
		 -0.00018334389 0.00034999847 -0.00018334389 0.00033335388 -0.00018334389 0.00031664968
		 -0.00018334389 0.00029997528 -0.00018334389 0.00028333068 -0.00018334389 0.00026668608
		 -0.00018334389 0.00024998188 -0.00018334389 0.00023332238 -0.00018334389 0.00021666288
		 -0.00018334389 0.00020000339 -0.00018334389 0.00018334389 -0.00018334389 0.00016668439
		 -0.00018334389 0.00014999509 -0.00018334389 0.00013333559 -0.00018334389 0.00011667609
		 -0.00018334389 9.9986792e-05 -0.00018334389 8.3327293e-05 -0.00018334389 6.6667795e-05
		 -0.00018334389 5.0008297e-05 -0.00018334389 3.3348799e-05 -0.00018334389 1.6659498e-05
		 -0.00018334389 0 -0.00018334389 -1.6689301e-05 -0.00018334389 -3.3318996e-05 -0.00018334389
		 -5.0008297e-05 -0.00018334389 -6.6697598e-05 -0.00018334389 -8.3327293e-05 -0.00018334389
		 -0.00010001659 -0.00018334389 -0.00011664629 -0.00018334389 -0.00013333559 -0.00018334389
		 -0.00015002489 -0.00018334389 -0.00016665459 -0.00018334389 -0.00018334389 -0.00018334389
		 -0.00019997358 -0.00018334389 -0.00021666288 -0.00018334389 -0.00023335218 -0.00018334389
		 -0.00024998188 -0.00018334389 -0.00026667118 -0.00018334389 -0.00028330088 -0.00018334389
		 -0.00029999018 -0.00018334389 -0.00031667948 -0.00018334389 -0.00033336878 -0.00018334389
		 -0.00034999847 -0.00018334389 -0.00036662817 -0.00018334389 -0.00038331747 -0.00018334389
		 -0.00040000677 -0.00018334389 -0.00041669607 -0.00018334389 -0.00043332577 -0.00018334389
		 -0.00044995546 -0.00018334389 -0.00046664476 -0.00018334389 -0.00048333406 -0.00018334389;
	setAttr ".uvtk[2500:2749]" -0.00050002337 -0.00018334389 0.00049999356 -0.00019997358
		 0.0004833471 -0.00019997358 0.00046667084 -0.00019997358 0.00044999272 -0.00019997358
		 0.00043331832 -0.00019997358 0.00041667372 -0.00019997358 0.00039999932 -0.00019997358
		 0.00038332492 -0.00019997358 0.00036664307 -0.00019997358 0.00034999847 -0.00019997358
		 0.00033335388 -0.00019997358 0.00031664968 -0.00019997358 0.00029997528 -0.00019997358
		 0.00028333068 -0.00019997358 0.00026668608 -0.00019997358 0.00024998188 -0.00019997358
		 0.00023332238 -0.00019997358 0.00021666288 -0.00019997358 0.00020000339 -0.00019997358
		 0.00018334389 -0.00019997358 0.00016668439 -0.00019997358 0.00014999509 -0.00019997358
		 0.00013333559 -0.00019997358 0.00011667609 -0.00019997358 9.9986792e-05 -0.00019997358
		 8.3327293e-05 -0.00019997358 6.6667795e-05 -0.00019997358 5.0008297e-05 -0.00019997358
		 3.3348799e-05 -0.00019997358 1.6659498e-05 -0.00019997358 0 -0.00019997358 -1.6689301e-05
		 -0.00019997358 -3.3318996e-05 -0.00019997358 -5.0008297e-05 -0.00019997358 -6.6697598e-05
		 -0.00019997358 -8.3327293e-05 -0.00019997358 -0.00010001659 -0.00019997358 -0.00011664629
		 -0.00019997358 -0.00013333559 -0.00019997358 -0.00015002489 -0.00019997358 -0.00016665459
		 -0.00019997358 -0.00018334389 -0.00019997358 -0.00019997358 -0.00019997358 -0.00021666288
		 -0.00019997358 -0.00023335218 -0.00019997358 -0.00024998188 -0.00019997358 -0.00026667118
		 -0.00019997358 -0.00028330088 -0.00019997358 -0.00029999018 -0.00019997358 -0.00031667948
		 -0.00019997358 -0.00033336878 -0.00019997358 -0.00034999847 -0.00019997358 -0.00036662817
		 -0.00019997358 -0.00038331747 -0.00019997358 -0.00040000677 -0.00019997358 -0.00041669607
		 -0.00019997358 -0.00043332577 -0.00019997358 -0.00044995546 -0.00019997358 -0.00046664476
		 -0.00019997358 -0.00048333406 -0.00019997358 -0.00050002337 -0.00019997358 0.00049999356
		 -0.00021666288 0.0004833471 -0.00021666288 0.00046667084 -0.00021666288 0.00044999272
		 -0.00021666288 0.00043331832 -0.00021666288 0.00041667372 -0.00021666288 0.00039999932
		 -0.00021666288 0.00038332492 -0.00021666288 0.00036664307 -0.00021666288 0.00034999847
		 -0.00021666288 0.00033335388 -0.00021666288 0.00031664968 -0.00021666288 0.00029997528
		 -0.00021666288 0.00028333068 -0.00021666288 0.00026668608 -0.00021666288 0.00024998188
		 -0.00021666288 0.00023332238 -0.00021666288 0.00021666288 -0.00021666288 0.00020000339
		 -0.00021666288 0.00018334389 -0.00021666288 0.00016668439 -0.00021666288 0.00014999509
		 -0.00021666288 0.00013333559 -0.00021666288 0.00011667609 -0.00021666288 9.9986792e-05
		 -0.00021666288 8.3327293e-05 -0.00021666288 6.6667795e-05 -0.00021666288 5.0008297e-05
		 -0.00021666288 3.3348799e-05 -0.00021666288 1.6659498e-05 -0.00021666288 0 -0.00021666288
		 -1.6689301e-05 -0.00021666288 -3.3318996e-05 -0.00021666288 -5.0008297e-05 -0.00021666288
		 -6.6697598e-05 -0.00021666288 -8.3327293e-05 -0.00021666288 -0.00010001659 -0.00021666288
		 -0.00011664629 -0.00021666288 -0.00013333559 -0.00021666288 -0.00015002489 -0.00021666288
		 -0.00016665459 -0.00021666288 -0.00018334389 -0.00021666288 -0.00019997358 -0.00021666288
		 -0.00021666288 -0.00021666288 -0.00023335218 -0.00021666288 -0.00024998188 -0.00021666288
		 -0.00026667118 -0.00021666288 -0.00028330088 -0.00021666288 -0.00029999018 -0.00021666288
		 -0.00031667948 -0.00021666288 -0.00033336878 -0.00021666288 -0.00034999847 -0.00021666288
		 -0.00036662817 -0.00021666288 -0.00038331747 -0.00021666288 -0.00040000677 -0.00021666288
		 -0.00041669607 -0.00021666288 -0.00043332577 -0.00021666288 -0.00044995546 -0.00021666288
		 -0.00046664476 -0.00021666288 -0.00048333406 -0.00021666288 -0.00050002337 -0.00021666288
		 0.00049999356 -0.00023335218 0.0004833471 -0.00023335218 0.00046667084 -0.00023335218
		 0.00044999272 -0.00023335218 0.00043331832 -0.00023335218 0.00041667372 -0.00023335218
		 0.00039999932 -0.00023335218 0.00038332492 -0.00023335218 0.00036664307 -0.00023335218
		 0.00034999847 -0.00023335218 0.00033335388 -0.00023335218 0.00031664968 -0.00023335218
		 0.00029997528 -0.00023335218 0.00028333068 -0.00023335218 0.00026668608 -0.00023335218
		 0.00024998188 -0.00023335218 0.00023332238 -0.00023335218 0.00021666288 -0.00023335218
		 0.00020000339 -0.00023335218 0.00018334389 -0.00023335218 0.00016668439 -0.00023335218
		 0.00014999509 -0.00023335218 0.00013333559 -0.00023335218 0.00011667609 -0.00023335218
		 9.9986792e-05 -0.00023335218 8.3327293e-05 -0.00023335218 6.6667795e-05 -0.00023335218
		 5.0008297e-05 -0.00023335218 3.3348799e-05 -0.00023335218 1.6659498e-05 -0.00023335218
		 0 -0.00023335218 -1.6689301e-05 -0.00023335218 -3.3318996e-05 -0.00023335218 -5.0008297e-05
		 -0.00023335218 -6.6697598e-05 -0.00023335218 -8.3327293e-05 -0.00023335218 -0.00010001659
		 -0.00023335218 -0.00011664629 -0.00023335218 -0.00013333559 -0.00023335218 -0.00015002489
		 -0.00023335218 -0.00016665459 -0.00023335218 -0.00018334389 -0.00023335218 -0.00019997358
		 -0.00023335218 -0.00021666288 -0.00023335218 -0.00023335218 -0.00023335218 -0.00024998188
		 -0.00023335218 -0.00026667118 -0.00023335218 -0.00028330088 -0.00023335218 -0.00029999018
		 -0.00023335218 -0.00031667948 -0.00023335218 -0.00033336878 -0.00023335218 -0.00034999847
		 -0.00023335218 -0.00036662817 -0.00023335218 -0.00038331747 -0.00023335218 -0.00040000677
		 -0.00023335218 -0.00041669607 -0.00023335218 -0.00043332577 -0.00023335218 -0.00044995546
		 -0.00023335218 -0.00046664476 -0.00023335218 -0.00048333406 -0.00023335218 -0.00050002337
		 -0.00023335218 0.00049999356 -0.00024998188 0.0004833471 -0.00024998188 0.00046667084
		 -0.00024998188 0.00044999272 -0.00024998188 0.00043331832 -0.00024998188 0.00041667372
		 -0.00024998188 0.00039999932 -0.00024998188 0.00038332492 -0.00024998188 0.00036664307
		 -0.00024998188 0.00034999847 -0.00024998188 0.00033335388 -0.00024998188 0.00031664968
		 -0.00024998188 0.00029997528 -0.00024998188 0.00028333068 -0.00024998188 0.00026668608
		 -0.00024998188 0.00024998188 -0.00024998188 0.00023332238 -0.00024998188 0.00021666288
		 -0.00024998188 0.00020000339 -0.00024998188 0.00018334389 -0.00024998188 0.00016668439
		 -0.00024998188 0.00014999509 -0.00024998188 0.00013333559 -0.00024998188 0.00011667609
		 -0.00024998188 9.9986792e-05 -0.00024998188 8.3327293e-05 -0.00024998188 6.6667795e-05
		 -0.00024998188 5.0008297e-05 -0.00024998188 3.3348799e-05 -0.00024998188 1.6659498e-05
		 -0.00024998188 0 -0.00024998188 -1.6689301e-05 -0.00024998188 -3.3318996e-05 -0.00024998188
		 -5.0008297e-05 -0.00024998188 -6.6697598e-05 -0.00024998188 -8.3327293e-05 -0.00024998188
		 -0.00010001659 -0.00024998188 -0.00011664629 -0.00024998188 -0.00013333559 -0.00024998188
		 -0.00015002489 -0.00024998188 -0.00016665459 -0.00024998188 -0.00018334389 -0.00024998188
		 -0.00019997358 -0.00024998188 -0.00021666288 -0.00024998188 -0.00023335218 -0.00024998188
		 -0.00024998188 -0.00024998188 -0.00026667118 -0.00024998188 -0.00028330088 -0.00024998188
		 -0.00029999018 -0.00024998188 -0.00031667948 -0.00024998188 -0.00033336878 -0.00024998188
		 -0.00034999847 -0.00024998188 -0.00036662817 -0.00024998188 -0.00038331747 -0.00024998188
		 -0.00040000677 -0.00024998188 -0.00041669607 -0.00024998188 -0.00043332577 -0.00024998188
		 -0.00044995546 -0.00024998188 -0.00046664476 -0.00024998188 -0.00048333406 -0.00024998188
		 -0.00050002337 -0.00024998188 0.00049999356 -0.00026667118 0.0004833471 -0.00026667118
		 0.00046667084 -0.00026667118 0.00044999272 -0.00026667118 0.00043331832 -0.00026667118;
	setAttr ".uvtk[2750:2999]" 0.00041667372 -0.00026667118 0.00039999932 -0.00026667118
		 0.00038332492 -0.00026667118 0.00036664307 -0.00026667118 0.00034999847 -0.00026667118
		 0.00033335388 -0.00026667118 0.00031664968 -0.00026667118 0.00029997528 -0.00026667118
		 0.00028333068 -0.00026667118 0.00026668608 -0.00026667118 0.00024998188 -0.00026667118
		 0.00023332238 -0.00026667118 0.00021666288 -0.00026667118 0.00020000339 -0.00026667118
		 0.00018334389 -0.00026667118 0.00016668439 -0.00026667118 0.00014999509 -0.00026667118
		 0.00013333559 -0.00026667118 0.00011667609 -0.00026667118 9.9986792e-05 -0.00026667118
		 8.3327293e-05 -0.00026667118 6.6667795e-05 -0.00026667118 5.0008297e-05 -0.00026667118
		 3.3348799e-05 -0.00026667118 1.6659498e-05 -0.00026667118 0 -0.00026667118 -1.6689301e-05
		 -0.00026667118 -3.3318996e-05 -0.00026667118 -5.0008297e-05 -0.00026667118 -6.6697598e-05
		 -0.00026667118 -8.3327293e-05 -0.00026667118 -0.00010001659 -0.00026667118 -0.00011664629
		 -0.00026667118 -0.00013333559 -0.00026667118 -0.00015002489 -0.00026667118 -0.00016665459
		 -0.00026667118 -0.00018334389 -0.00026667118 -0.00019997358 -0.00026667118 -0.00021666288
		 -0.00026667118 -0.00023335218 -0.00026667118 -0.00024998188 -0.00026667118 -0.00026667118
		 -0.00026667118 -0.00028330088 -0.00026667118 -0.00029999018 -0.00026667118 -0.00031667948
		 -0.00026667118 -0.00033336878 -0.00026667118 -0.00034999847 -0.00026667118 -0.00036662817
		 -0.00026667118 -0.00038331747 -0.00026667118 -0.00040000677 -0.00026667118 -0.00041669607
		 -0.00026667118 -0.00043332577 -0.00026667118 -0.00044995546 -0.00026667118 -0.00046664476
		 -0.00026667118 -0.00048333406 -0.00026667118 -0.00050002337 -0.00026667118 0.00049999356
		 -0.00028330088 0.0004833471 -0.00028330088 0.00046667084 -0.00028330088 0.00044999272
		 -0.00028330088 0.00043331832 -0.00028330088 0.00041667372 -0.00028330088 0.00039999932
		 -0.00028330088 0.00038332492 -0.00028330088 0.00036664307 -0.00028330088 0.00034999847
		 -0.00028330088 0.00033335388 -0.00028330088 0.00031664968 -0.00028330088 0.00029997528
		 -0.00028330088 0.00028333068 -0.00028330088 0.00026668608 -0.00028330088 0.00024998188
		 -0.00028330088 0.00023332238 -0.00028330088 0.00021666288 -0.00028330088 0.00020000339
		 -0.00028330088 0.00018334389 -0.00028330088 0.00016668439 -0.00028330088 0.00014999509
		 -0.00028330088 0.00013333559 -0.00028330088 0.00011667609 -0.00028330088 9.9986792e-05
		 -0.00028330088 8.3327293e-05 -0.00028330088 6.6667795e-05 -0.00028330088 5.0008297e-05
		 -0.00028330088 3.3348799e-05 -0.00028330088 1.6659498e-05 -0.00028330088 0 -0.00028330088
		 -1.6689301e-05 -0.00028330088 -3.3318996e-05 -0.00028330088 -5.0008297e-05 -0.00028330088
		 -6.6697598e-05 -0.00028330088 -8.3327293e-05 -0.00028330088 -0.00010001659 -0.00028330088
		 -0.00011664629 -0.00028330088 -0.00013333559 -0.00028330088 -0.00015002489 -0.00028330088
		 -0.00016665459 -0.00028330088 -0.00018334389 -0.00028330088 -0.00019997358 -0.00028330088
		 -0.00021666288 -0.00028330088 -0.00023335218 -0.00028330088 -0.00024998188 -0.00028330088
		 -0.00026667118 -0.00028330088 -0.00028330088 -0.00028330088 -0.00029999018 -0.00028330088
		 -0.00031667948 -0.00028330088 -0.00033336878 -0.00028330088 -0.00034999847 -0.00028330088
		 -0.00036662817 -0.00028330088 -0.00038331747 -0.00028330088 -0.00040000677 -0.00028330088
		 -0.00041669607 -0.00028330088 -0.00043332577 -0.00028330088 -0.00044995546 -0.00028330088
		 -0.00046664476 -0.00028330088 -0.00048333406 -0.00028330088 -0.00050002337 -0.00028330088
		 0.00049999356 -0.00029999018 0.0004833471 -0.00029999018 0.00046667084 -0.00029999018
		 0.00044999272 -0.00029999018 0.00043331832 -0.00029999018 0.00041667372 -0.00029999018
		 0.00039999932 -0.00029999018 0.00038332492 -0.00029999018 0.00036664307 -0.00029999018
		 0.00034999847 -0.00029999018 0.00033335388 -0.00029999018 0.00031664968 -0.00029999018
		 0.00029997528 -0.00029999018 0.00028333068 -0.00029999018 0.00026668608 -0.00029999018
		 0.00024998188 -0.00029999018 0.00023332238 -0.00029999018 0.00021666288 -0.00029999018
		 0.00020000339 -0.00029999018 0.00018334389 -0.00029999018 0.00016668439 -0.00029999018
		 0.00014999509 -0.00029999018 0.00013333559 -0.00029999018 0.00011667609 -0.00029999018
		 9.9986792e-05 -0.00029999018 8.3327293e-05 -0.00029999018 6.6667795e-05 -0.00029999018
		 5.0008297e-05 -0.00029999018 3.3348799e-05 -0.00029999018 1.6659498e-05 -0.00029999018
		 0 -0.00029999018 -1.6689301e-05 -0.00029999018 -3.3318996e-05 -0.00029999018 -5.0008297e-05
		 -0.00029999018 -6.6697598e-05 -0.00029999018 -8.3327293e-05 -0.00029999018 -0.00010001659
		 -0.00029999018 -0.00011664629 -0.00029999018 -0.00013333559 -0.00029999018 -0.00015002489
		 -0.00029999018 -0.00016665459 -0.00029999018 -0.00018334389 -0.00029999018 -0.00019997358
		 -0.00029999018 -0.00021666288 -0.00029999018 -0.00023335218 -0.00029999018 -0.00024998188
		 -0.00029999018 -0.00026667118 -0.00029999018 -0.00028330088 -0.00029999018 -0.00029999018
		 -0.00029999018 -0.00031667948 -0.00029999018 -0.00033336878 -0.00029999018 -0.00034999847
		 -0.00029999018 -0.00036662817 -0.00029999018 -0.00038331747 -0.00029999018 -0.00040000677
		 -0.00029999018 -0.00041669607 -0.00029999018 -0.00043332577 -0.00029999018 -0.00044995546
		 -0.00029999018 -0.00046664476 -0.00029999018 -0.00048333406 -0.00029999018 -0.00050002337
		 -0.00029999018 0.00049999356 -0.00031667948 0.0004833471 -0.00031667948 0.00046667084
		 -0.00031667948 0.00044999272 -0.00031667948 0.00043331832 -0.00031667948 0.00041667372
		 -0.00031667948 0.00039999932 -0.00031667948 0.00038332492 -0.00031667948 0.00036664307
		 -0.00031667948 0.00034999847 -0.00031667948 0.00033335388 -0.00031667948 0.00031664968
		 -0.00031667948 0.00029997528 -0.00031667948 0.00028333068 -0.00031667948 0.00026668608
		 -0.00031667948 0.00024998188 -0.00031667948 0.00023332238 -0.00031667948 0.00021666288
		 -0.00031667948 0.00020000339 -0.00031667948 0.00018334389 -0.00031667948 0.00016668439
		 -0.00031667948 0.00014999509 -0.00031667948 0.00013333559 -0.00031667948 0.00011667609
		 -0.00031667948 9.9986792e-05 -0.00031667948 8.3327293e-05 -0.00031667948 6.6667795e-05
		 -0.00031667948 5.0008297e-05 -0.00031667948 3.3348799e-05 -0.00031667948 1.6659498e-05
		 -0.00031667948 0 -0.00031667948 -1.6689301e-05 -0.00031667948 -3.3318996e-05 -0.00031667948
		 -5.0008297e-05 -0.00031667948 -6.6697598e-05 -0.00031667948 -8.3327293e-05 -0.00031667948
		 -0.00010001659 -0.00031667948 -0.00011664629 -0.00031667948 -0.00013333559 -0.00031667948
		 -0.00015002489 -0.00031667948 -0.00016665459 -0.00031667948 -0.00018334389 -0.00031667948
		 -0.00019997358 -0.00031667948 -0.00021666288 -0.00031667948 -0.00023335218 -0.00031667948
		 -0.00024998188 -0.00031667948 -0.00026667118 -0.00031667948 -0.00028330088 -0.00031667948
		 -0.00029999018 -0.00031667948 -0.00031667948 -0.00031667948 -0.00033336878 -0.00031667948
		 -0.00034999847 -0.00031667948 -0.00036662817 -0.00031667948 -0.00038331747 -0.00031667948
		 -0.00040000677 -0.00031667948 -0.00041669607 -0.00031667948 -0.00043332577 -0.00031667948
		 -0.00044995546 -0.00031667948 -0.00046664476 -0.00031667948 -0.00048333406 -0.00031667948
		 -0.00050002337 -0.00031667948 0.00049999356 -0.00033336878 0.0004833471 -0.00033336878
		 0.00046667084 -0.00033336878 0.00044999272 -0.00033336878 0.00043331832 -0.00033336878
		 0.00041667372 -0.00033336878 0.00039999932 -0.00033336878 0.00038332492 -0.00033336878
		 0.00036664307 -0.00033336878 0.00034999847 -0.00033336878 0.00033335388 -0.00033336878;
	setAttr ".uvtk[3000:3249]" 0.00031664968 -0.00033336878 0.00029997528 -0.00033336878
		 0.00028333068 -0.00033336878 0.00026668608 -0.00033336878 0.00024998188 -0.00033336878
		 0.00023332238 -0.00033336878 0.00021666288 -0.00033336878 0.00020000339 -0.00033336878
		 0.00018334389 -0.00033336878 0.00016668439 -0.00033336878 0.00014999509 -0.00033336878
		 0.00013333559 -0.00033336878 0.00011667609 -0.00033336878 9.9986792e-05 -0.00033336878
		 8.3327293e-05 -0.00033336878 6.6667795e-05 -0.00033336878 5.0008297e-05 -0.00033336878
		 3.3348799e-05 -0.00033336878 1.6659498e-05 -0.00033336878 0 -0.00033336878 -1.6689301e-05
		 -0.00033336878 -3.3318996e-05 -0.00033336878 -5.0008297e-05 -0.00033336878 -6.6697598e-05
		 -0.00033336878 -8.3327293e-05 -0.00033336878 -0.00010001659 -0.00033336878 -0.00011664629
		 -0.00033336878 -0.00013333559 -0.00033336878 -0.00015002489 -0.00033336878 -0.00016665459
		 -0.00033336878 -0.00018334389 -0.00033336878 -0.00019997358 -0.00033336878 -0.00021666288
		 -0.00033336878 -0.00023335218 -0.00033336878 -0.00024998188 -0.00033336878 -0.00026667118
		 -0.00033336878 -0.00028330088 -0.00033336878 -0.00029999018 -0.00033336878 -0.00031667948
		 -0.00033336878 -0.00033336878 -0.00033336878 -0.00034999847 -0.00033336878 -0.00036662817
		 -0.00033336878 -0.00038331747 -0.00033336878 -0.00040000677 -0.00033336878 -0.00041669607
		 -0.00033336878 -0.00043332577 -0.00033336878 -0.00044995546 -0.00033336878 -0.00046664476
		 -0.00033336878 -0.00048333406 -0.00033336878 -0.00050002337 -0.00033336878 0.00049999356
		 -0.00034999847 0.0004833471 -0.00034999847 0.00046667084 -0.00034999847 0.00044999272
		 -0.00034999847 0.00043331832 -0.00034999847 0.00041667372 -0.00034999847 0.00039999932
		 -0.00034999847 0.00038332492 -0.00034999847 0.00036664307 -0.00034999847 0.00034999847
		 -0.00034999847 0.00033335388 -0.00034999847 0.00031664968 -0.00034999847 0.00029997528
		 -0.00034999847 0.00028333068 -0.00034999847 0.00026668608 -0.00034999847 0.00024998188
		 -0.00034999847 0.00023332238 -0.00034999847 0.00021666288 -0.00034999847 0.00020000339
		 -0.00034999847 0.00018334389 -0.00034999847 0.00016668439 -0.00034999847 0.00014999509
		 -0.00034999847 0.00013333559 -0.00034999847 0.00011667609 -0.00034999847 9.9986792e-05
		 -0.00034999847 8.3327293e-05 -0.00034999847 6.6667795e-05 -0.00034999847 5.0008297e-05
		 -0.00034999847 3.3348799e-05 -0.00034999847 1.6659498e-05 -0.00034999847 0 -0.00034999847
		 -1.6689301e-05 -0.00034999847 -3.3318996e-05 -0.00034999847 -5.0008297e-05 -0.00034999847
		 -6.6697598e-05 -0.00034999847 -8.3327293e-05 -0.00034999847 -0.00010001659 -0.00034999847
		 -0.00011664629 -0.00034999847 -0.00013333559 -0.00034999847 -0.00015002489 -0.00034999847
		 -0.00016665459 -0.00034999847 -0.00018334389 -0.00034999847 -0.00019997358 -0.00034999847
		 -0.00021666288 -0.00034999847 -0.00023335218 -0.00034999847 -0.00024998188 -0.00034999847
		 -0.00026667118 -0.00034999847 -0.00028330088 -0.00034999847 -0.00029999018 -0.00034999847
		 -0.00031667948 -0.00034999847 -0.00033336878 -0.00034999847 -0.00034999847 -0.00034999847
		 -0.00036662817 -0.00034999847 -0.00038331747 -0.00034999847 -0.00040000677 -0.00034999847
		 -0.00041669607 -0.00034999847 -0.00043332577 -0.00034999847 -0.00044995546 -0.00034999847
		 -0.00046664476 -0.00034999847 -0.00048333406 -0.00034999847 -0.00050002337 -0.00034999847
		 0.00049999356 -0.00036662817 0.0004833471 -0.00036662817 0.00046667084 -0.00036662817
		 0.00044999272 -0.00036662817 0.00043331832 -0.00036662817 0.00041667372 -0.00036662817
		 0.00039999932 -0.00036662817 0.00038332492 -0.00036662817 0.00036664307 -0.00036662817
		 0.00034999847 -0.00036662817 0.00033335388 -0.00036662817 0.00031664968 -0.00036662817
		 0.00029997528 -0.00036662817 0.00028333068 -0.00036662817 0.00026668608 -0.00036662817
		 0.00024998188 -0.00036662817 0.00023332238 -0.00036662817 0.00021666288 -0.00036662817
		 0.00020000339 -0.00036662817 0.00018334389 -0.00036662817 0.00016668439 -0.00036662817
		 0.00014999509 -0.00036662817 0.00013333559 -0.00036662817 0.00011667609 -0.00036662817
		 9.9986792e-05 -0.00036662817 8.3327293e-05 -0.00036662817 6.6667795e-05 -0.00036662817
		 5.0008297e-05 -0.00036662817 3.3348799e-05 -0.00036662817 1.6659498e-05 -0.00036662817
		 0 -0.00036662817 -1.6689301e-05 -0.00036662817 -3.3318996e-05 -0.00036662817 -5.0008297e-05
		 -0.00036662817 -6.6697598e-05 -0.00036662817 -8.3327293e-05 -0.00036662817 -0.00010001659
		 -0.00036662817 -0.00011664629 -0.00036662817 -0.00013333559 -0.00036662817 -0.00015002489
		 -0.00036662817 -0.00016665459 -0.00036662817 -0.00018334389 -0.00036662817 -0.00019997358
		 -0.00036662817 -0.00021666288 -0.00036662817 -0.00023335218 -0.00036662817 -0.00024998188
		 -0.00036662817 -0.00026667118 -0.00036662817 -0.00028330088 -0.00036662817 -0.00029999018
		 -0.00036662817 -0.00031667948 -0.00036662817 -0.00033336878 -0.00036662817 -0.00034999847
		 -0.00036662817 -0.00036662817 -0.00036662817 -0.00038331747 -0.00036662817 -0.00040000677
		 -0.00036662817 -0.00041669607 -0.00036662817 -0.00043332577 -0.00036662817 -0.00044995546
		 -0.00036662817 -0.00046664476 -0.00036662817 -0.00048333406 -0.00036662817 -0.00050002337
		 -0.00036662817 0.00049999356 -0.00038331747 0.0004833471 -0.00038331747 0.00046667084
		 -0.00038331747 0.00044999272 -0.00038331747 0.00043331832 -0.00038331747 0.00041667372
		 -0.00038331747 0.00039999932 -0.00038331747 0.00038332492 -0.00038331747 0.00036664307
		 -0.00038331747 0.00034999847 -0.00038331747 0.00033335388 -0.00038331747 0.00031664968
		 -0.00038331747 0.00029997528 -0.00038331747 0.00028333068 -0.00038331747 0.00026668608
		 -0.00038331747 0.00024998188 -0.00038331747 0.00023332238 -0.00038331747 0.00021666288
		 -0.00038331747 0.00020000339 -0.00038331747 0.00018334389 -0.00038331747 0.00016668439
		 -0.00038331747 0.00014999509 -0.00038331747 0.00013333559 -0.00038331747 0.00011667609
		 -0.00038331747 9.9986792e-05 -0.00038331747 8.3327293e-05 -0.00038331747 6.6667795e-05
		 -0.00038331747 5.0008297e-05 -0.00038331747 3.3348799e-05 -0.00038331747 1.6659498e-05
		 -0.00038331747 0 -0.00038331747 -1.6689301e-05 -0.00038331747 -3.3318996e-05 -0.00038331747
		 -5.0008297e-05 -0.00038331747 -6.6697598e-05 -0.00038331747 -8.3327293e-05 -0.00038331747
		 -0.00010001659 -0.00038331747 -0.00011664629 -0.00038331747 -0.00013333559 -0.00038331747
		 -0.00015002489 -0.00038331747 -0.00016665459 -0.00038331747 -0.00018334389 -0.00038331747
		 -0.00019997358 -0.00038331747 -0.00021666288 -0.00038331747 -0.00023335218 -0.00038331747
		 -0.00024998188 -0.00038331747 -0.00026667118 -0.00038331747 -0.00028330088 -0.00038331747
		 -0.00029999018 -0.00038331747 -0.00031667948 -0.00038331747 -0.00033336878 -0.00038331747
		 -0.00034999847 -0.00038331747 -0.00036662817 -0.00038331747 -0.00038331747 -0.00038331747
		 -0.00040000677 -0.00038331747 -0.00041669607 -0.00038331747 -0.00043332577 -0.00038331747
		 -0.00044995546 -0.00038331747 -0.00046664476 -0.00038331747 -0.00048333406 -0.00038331747
		 -0.00050002337 -0.00038331747 0.00049999356 -0.00040000677 0.0004833471 -0.00040000677
		 0.00046667084 -0.00040000677 0.00044999272 -0.00040000677 0.00043331832 -0.00040000677
		 0.00041667372 -0.00040000677 0.00039999932 -0.00040000677 0.00038332492 -0.00040000677
		 0.00036664307 -0.00040000677 0.00034999847 -0.00040000677 0.00033335388 -0.00040000677
		 0.00031664968 -0.00040000677 0.00029997528 -0.00040000677 0.00028333068 -0.00040000677
		 0.00026668608 -0.00040000677 0.00024998188 -0.00040000677 0.00023332238 -0.00040000677;
	setAttr ".uvtk[3250:3499]" 0.00021666288 -0.00040000677 0.00020000339 -0.00040000677
		 0.00018334389 -0.00040000677 0.00016668439 -0.00040000677 0.00014999509 -0.00040000677
		 0.00013333559 -0.00040000677 0.00011667609 -0.00040000677 9.9986792e-05 -0.00040000677
		 8.3327293e-05 -0.00040000677 6.6667795e-05 -0.00040000677 5.0008297e-05 -0.00040000677
		 3.3348799e-05 -0.00040000677 1.6659498e-05 -0.00040000677 0 -0.00040000677 -1.6689301e-05
		 -0.00040000677 -3.3318996e-05 -0.00040000677 -5.0008297e-05 -0.00040000677 -6.6697598e-05
		 -0.00040000677 -8.3327293e-05 -0.00040000677 -0.00010001659 -0.00040000677 -0.00011664629
		 -0.00040000677 -0.00013333559 -0.00040000677 -0.00015002489 -0.00040000677 -0.00016665459
		 -0.00040000677 -0.00018334389 -0.00040000677 -0.00019997358 -0.00040000677 -0.00021666288
		 -0.00040000677 -0.00023335218 -0.00040000677 -0.00024998188 -0.00040000677 -0.00026667118
		 -0.00040000677 -0.00028330088 -0.00040000677 -0.00029999018 -0.00040000677 -0.00031667948
		 -0.00040000677 -0.00033336878 -0.00040000677 -0.00034999847 -0.00040000677 -0.00036662817
		 -0.00040000677 -0.00038331747 -0.00040000677 -0.00040000677 -0.00040000677 -0.00041669607
		 -0.00040000677 -0.00043332577 -0.00040000677 -0.00044995546 -0.00040000677 -0.00046664476
		 -0.00040000677 -0.00048333406 -0.00040000677 -0.00050002337 -0.00040000677 0.00049999356
		 -0.00041669607 0.0004833471 -0.00041669607 0.00046667084 -0.00041669607 0.00044999272
		 -0.00041669607 0.00043331832 -0.00041669607 0.00041667372 -0.00041669607 0.00039999932
		 -0.00041669607 0.00038332492 -0.00041669607 0.00036664307 -0.00041669607 0.00034999847
		 -0.00041669607 0.00033335388 -0.00041669607 0.00031664968 -0.00041669607 0.00029997528
		 -0.00041669607 0.00028333068 -0.00041669607 0.00026668608 -0.00041669607 0.00024998188
		 -0.00041669607 0.00023332238 -0.00041669607 0.00021666288 -0.00041669607 0.00020000339
		 -0.00041669607 0.00018334389 -0.00041669607 0.00016668439 -0.00041669607 0.00014999509
		 -0.00041669607 0.00013333559 -0.00041669607 0.00011667609 -0.00041669607 9.9986792e-05
		 -0.00041669607 8.3327293e-05 -0.00041669607 6.6667795e-05 -0.00041669607 5.0008297e-05
		 -0.00041669607 3.3348799e-05 -0.00041669607 1.6659498e-05 -0.00041669607 0 -0.00041669607
		 -1.6689301e-05 -0.00041669607 -3.3318996e-05 -0.00041669607 -5.0008297e-05 -0.00041669607
		 -6.6697598e-05 -0.00041669607 -8.3327293e-05 -0.00041669607 -0.00010001659 -0.00041669607
		 -0.00011664629 -0.00041669607 -0.00013333559 -0.00041669607 -0.00015002489 -0.00041669607
		 -0.00016665459 -0.00041669607 -0.00018334389 -0.00041669607 -0.00019997358 -0.00041669607
		 -0.00021666288 -0.00041669607 -0.00023335218 -0.00041669607 -0.00024998188 -0.00041669607
		 -0.00026667118 -0.00041669607 -0.00028330088 -0.00041669607 -0.00029999018 -0.00041669607
		 -0.00031667948 -0.00041669607 -0.00033336878 -0.00041669607 -0.00034999847 -0.00041669607
		 -0.00036662817 -0.00041669607 -0.00038331747 -0.00041669607 -0.00040000677 -0.00041669607
		 -0.00041669607 -0.00041669607 -0.00043332577 -0.00041669607 -0.00044995546 -0.00041669607
		 -0.00046664476 -0.00041669607 -0.00048333406 -0.00041669607 -0.00050002337 -0.00041669607
		 0.00049999356 -0.00043332577 0.0004833471 -0.00043332577 0.00046667084 -0.00043332577
		 0.00044999272 -0.00043332577 0.00043331832 -0.00043332577 0.00041667372 -0.00043332577
		 0.00039999932 -0.00043332577 0.00038332492 -0.00043332577 0.00036664307 -0.00043332577
		 0.00034999847 -0.00043332577 0.00033335388 -0.00043332577 0.00031664968 -0.00043332577
		 0.00029997528 -0.00043332577 0.00028333068 -0.00043332577 0.00026668608 -0.00043332577
		 0.00024998188 -0.00043332577 0.00023332238 -0.00043332577 0.00021666288 -0.00043332577
		 0.00020000339 -0.00043332577 0.00018334389 -0.00043332577 0.00016668439 -0.00043332577
		 0.00014999509 -0.00043332577 0.00013333559 -0.00043332577 0.00011667609 -0.00043332577
		 9.9986792e-05 -0.00043332577 8.3327293e-05 -0.00043332577 6.6667795e-05 -0.00043332577
		 5.0008297e-05 -0.00043332577 3.3348799e-05 -0.00043332577 1.6659498e-05 -0.00043332577
		 0 -0.00043332577 -1.6689301e-05 -0.00043332577 -3.3318996e-05 -0.00043332577 -5.0008297e-05
		 -0.00043332577 -6.6697598e-05 -0.00043332577 -8.3327293e-05 -0.00043332577 -0.00010001659
		 -0.00043332577 -0.00011664629 -0.00043332577 -0.00013333559 -0.00043332577 -0.00015002489
		 -0.00043332577 -0.00016665459 -0.00043332577 -0.00018334389 -0.00043332577 -0.00019997358
		 -0.00043332577 -0.00021666288 -0.00043332577 -0.00023335218 -0.00043332577 -0.00024998188
		 -0.00043332577 -0.00026667118 -0.00043332577 -0.00028330088 -0.00043332577 -0.00029999018
		 -0.00043332577 -0.00031667948 -0.00043332577 -0.00033336878 -0.00043332577 -0.00034999847
		 -0.00043332577 -0.00036662817 -0.00043332577 -0.00038331747 -0.00043332577 -0.00040000677
		 -0.00043332577 -0.00041669607 -0.00043332577 -0.00043332577 -0.00043332577 -0.00044995546
		 -0.00043332577 -0.00046664476 -0.00043332577 -0.00048333406 -0.00043332577 -0.00050002337
		 -0.00043332577 0.00049999356 -0.00044995546 0.0004833471 -0.00044995546 0.00046667084
		 -0.00044995546 0.00044999272 -0.00044995546 0.00043331832 -0.00044995546 0.00041667372
		 -0.00044995546 0.00039999932 -0.00044995546 0.00038332492 -0.00044995546 0.00036664307
		 -0.00044995546 0.00034999847 -0.00044995546 0.00033335388 -0.00044995546 0.00031664968
		 -0.00044995546 0.00029997528 -0.00044995546 0.00028333068 -0.00044995546 0.00026668608
		 -0.00044995546 0.00024998188 -0.00044995546 0.00023332238 -0.00044995546 0.00021666288
		 -0.00044995546 0.00020000339 -0.00044995546 0.00018334389 -0.00044995546 0.00016668439
		 -0.00044995546 0.00014999509 -0.00044995546 0.00013333559 -0.00044995546 0.00011667609
		 -0.00044995546 9.9986792e-05 -0.00044995546 8.3327293e-05 -0.00044995546 6.6667795e-05
		 -0.00044995546 5.0008297e-05 -0.00044995546 3.3348799e-05 -0.00044995546 1.6659498e-05
		 -0.00044995546 0 -0.00044995546 -1.6689301e-05 -0.00044995546 -3.3318996e-05 -0.00044995546
		 -5.0008297e-05 -0.00044995546 -6.6697598e-05 -0.00044995546 -8.3327293e-05 -0.00044995546
		 -0.00010001659 -0.00044995546 -0.00011664629 -0.00044995546 -0.00013333559 -0.00044995546
		 -0.00015002489 -0.00044995546 -0.00016665459 -0.00044995546 -0.00018334389 -0.00044995546
		 -0.00019997358 -0.00044995546 -0.00021666288 -0.00044995546 -0.00023335218 -0.00044995546
		 -0.00024998188 -0.00044995546 -0.00026667118 -0.00044995546 -0.00028330088 -0.00044995546
		 -0.00029999018 -0.00044995546 -0.00031667948 -0.00044995546 -0.00033336878 -0.00044995546
		 -0.00034999847 -0.00044995546 -0.00036662817 -0.00044995546 -0.00038331747 -0.00044995546
		 -0.00040000677 -0.00044995546 -0.00041669607 -0.00044995546 -0.00043332577 -0.00044995546
		 -0.00044995546 -0.00044995546 -0.00046664476 -0.00044995546 -0.00048333406 -0.00044995546
		 -0.00050002337 -0.00044995546 0.00049999356 -0.00046664476 0.0004833471 -0.00046664476
		 0.00046667084 -0.00046664476 0.00044999272 -0.00046664476 0.00043331832 -0.00046664476
		 0.00041667372 -0.00046664476 0.00039999932 -0.00046664476 0.00038332492 -0.00046664476
		 0.00036664307 -0.00046664476 0.00034999847 -0.00046664476 0.00033335388 -0.00046664476
		 0.00031664968 -0.00046664476 0.00029997528 -0.00046664476 0.00028333068 -0.00046664476
		 0.00026668608 -0.00046664476 0.00024998188 -0.00046664476 0.00023332238 -0.00046664476
		 0.00021666288 -0.00046664476 0.00020000339 -0.00046664476 0.00018334389 -0.00046664476
		 0.00016668439 -0.00046664476 0.00014999509 -0.00046664476 0.00013333559 -0.00046664476;
	setAttr ".uvtk[3500:3718]" 0.00011667609 -0.00046664476 9.9986792e-05 -0.00046664476
		 8.3327293e-05 -0.00046664476 6.6667795e-05 -0.00046664476 5.0008297e-05 -0.00046664476
		 3.3348799e-05 -0.00046664476 1.6659498e-05 -0.00046664476 0 -0.00046664476 -1.6689301e-05
		 -0.00046664476 -3.3318996e-05 -0.00046664476 -5.0008297e-05 -0.00046664476 -6.6697598e-05
		 -0.00046664476 -8.3327293e-05 -0.00046664476 -0.00010001659 -0.00046664476 -0.00011664629
		 -0.00046664476 -0.00013333559 -0.00046664476 -0.00015002489 -0.00046664476 -0.00016665459
		 -0.00046664476 -0.00018334389 -0.00046664476 -0.00019997358 -0.00046664476 -0.00021666288
		 -0.00046664476 -0.00023335218 -0.00046664476 -0.00024998188 -0.00046664476 -0.00026667118
		 -0.00046664476 -0.00028330088 -0.00046664476 -0.00029999018 -0.00046664476 -0.00031667948
		 -0.00046664476 -0.00033336878 -0.00046664476 -0.00034999847 -0.00046664476 -0.00036662817
		 -0.00046664476 -0.00038331747 -0.00046664476 -0.00040000677 -0.00046664476 -0.00041669607
		 -0.00046664476 -0.00043332577 -0.00046664476 -0.00044995546 -0.00046664476 -0.00046664476
		 -0.00046664476 -0.00048333406 -0.00046664476 -0.00050002337 -0.00046664476 0.00049999356
		 -0.00048333406 0.0004833471 -0.00048333406 0.00046667084 -0.00048333406 0.00044999272
		 -0.00048333406 0.00043331832 -0.00048333406 0.00041667372 -0.00048333406 0.00039999932
		 -0.00048333406 0.00038332492 -0.00048333406 0.00036664307 -0.00048333406 0.00034999847
		 -0.00048333406 0.00033335388 -0.00048333406 0.00031664968 -0.00048333406 0.00029997528
		 -0.00048333406 0.00028333068 -0.00048333406 0.00026668608 -0.00048333406 0.00024998188
		 -0.00048333406 0.00023332238 -0.00048333406 0.00021666288 -0.00048333406 0.00020000339
		 -0.00048333406 0.00018334389 -0.00048333406 0.00016668439 -0.00048333406 0.00014999509
		 -0.00048333406 0.00013333559 -0.00048333406 0.00011667609 -0.00048333406 9.9986792e-05
		 -0.00048333406 8.3327293e-05 -0.00048333406 6.6667795e-05 -0.00048333406 5.0008297e-05
		 -0.00048333406 3.3348799e-05 -0.00048333406 1.6659498e-05 -0.00048333406 0 -0.00048333406
		 -1.6689301e-05 -0.00048333406 -3.3318996e-05 -0.00048333406 -5.0008297e-05 -0.00048333406
		 -6.6697598e-05 -0.00048333406 -8.3327293e-05 -0.00048333406 -0.00010001659 -0.00048333406
		 -0.00011664629 -0.00048333406 -0.00013333559 -0.00048333406 -0.00015002489 -0.00048333406
		 -0.00016665459 -0.00048333406 -0.00018334389 -0.00048333406 -0.00019997358 -0.00048333406
		 -0.00021666288 -0.00048333406 -0.00023335218 -0.00048333406 -0.00024998188 -0.00048333406
		 -0.00026667118 -0.00048333406 -0.00028330088 -0.00048333406 -0.00029999018 -0.00048333406
		 -0.00031667948 -0.00048333406 -0.00033336878 -0.00048333406 -0.00034999847 -0.00048333406
		 -0.00036662817 -0.00048333406 -0.00038331747 -0.00048333406 -0.00040000677 -0.00048333406
		 -0.00041669607 -0.00048333406 -0.00043332577 -0.00048333406 -0.00044995546 -0.00048333406
		 -0.00046664476 -0.00048333406 -0.00048333406 -0.00048333406 -0.00050002337 -0.00048333406
		 0.00049164053 0.00049999356 0.00047499314 0.00049999356 0.00045831874 0.00049999356
		 0.00044167042 0.00049999356 0.00042496622 0.00049999356 0.00040832162 0.00049999356
		 0.00039166957 0.00049999356 0.00037498772 0.00049999356 0.00035834312 0.00049999356
		 0.00034166873 0.00049999356 0.00032499433 0.00049999356 0.00030831993 0.00049999356
		 0.00029169023 0.00049999356 0.00027498603 0.00049999356 0.00025831163 0.00049999356
		 0.00024166703 0.00049999356 0.00022497773 0.00049999356 0.00020831823 0.00049999356
		 0.00019165874 0.00049999356 0.00017499924 0.00049999356 0.00015833974 0.00049999356
		 0.00014168024 0.00049999356 0.00012499094 0.00049999356 0.00010833144 0.00049999356
		 9.1671944e-05 0.00049999356 7.4982643e-05 0.00049999356 5.8323145e-05 0.00049999356
		 4.1663647e-05 0.00049999356 2.5004148e-05 0.00049999356 8.3446503e-06 0.00049999356
		 -8.3446503e-06 0.00049999356 -2.4974346e-05 0.00049999356 -4.1663647e-05 0.00049999356
		 -5.8352947e-05 0.00049999356 -7.4982643e-05 0.00049999356 -9.1671944e-05 0.00049999356
		 -0.00010836124 0.00049999356 -0.00012499094 0.00049999356 -0.00014168024 0.00049999356
		 -0.00015830994 0.00049999356 -0.00017499924 0.00049999356 -0.00019168854 0.00049999356
		 -0.00020831823 0.00049999356 -0.00022500753 0.00049999356 -0.00024163723 0.00049999356
		 -0.00025832653 0.00049999356 -0.00027495623 0.00049999356 -0.00029164553 0.00049999356
		 -0.00030833483 0.00049999356 -0.00032496452 0.00049999356 -0.00034165382 0.00049999356
		 -0.00035834312 0.00049999356 -0.00037497282 0.00049999356 -0.00039166212 0.00049999356
		 -0.00040835142 0.00049999356 -0.00042498112 0.00049999356 -0.00044167042 0.00049999356
		 -0.00045835972 0.00049999356 -0.00047498941 0.00049999356 -0.00049167871 0.00049999356
		 0.00049164053 -0.00049996376 0.00047499314 -0.00049996376 0.00045831874 -0.00049996376
		 0.00044167042 -0.00049996376 0.00042496622 -0.00049996376 0.00040832162 -0.00049996376
		 0.00039166957 -0.00049996376 0.00037498772 -0.00049996376 0.00035834312 -0.00049996376
		 0.00034166873 -0.00049996376 0.00032499433 -0.00049996376 0.00030831993 -0.00049996376
		 0.00029169023 -0.00049996376 0.00027498603 -0.00049996376 0.00025831163 -0.00049996376
		 0.00024166703 -0.00049996376 0.00022497773 -0.00049996376 0.00020831823 -0.00049996376
		 0.00019165874 -0.00049996376 0.00017499924 -0.00049996376 0.00015833974 -0.00049996376
		 0.00014168024 -0.00049996376 0.00012499094 -0.00049996376 0.00010833144 -0.00049996376
		 9.1671944e-05 -0.00049996376 7.4982643e-05 -0.00049996376 5.8323145e-05 -0.00049996376
		 4.1663647e-05 -0.00049996376 2.5004148e-05 -0.00049996376 8.3446503e-06 -0.00049996376
		 -8.3446503e-06 -0.00049996376 -2.4974346e-05 -0.00049996376 -4.1663647e-05 -0.00049996376
		 -5.8352947e-05 -0.00049996376 -7.4982643e-05 -0.00049996376 -9.1671944e-05 -0.00049996376
		 -0.00010836124 -0.00049996376 -0.00012499094 -0.00049996376 -0.00014168024 -0.00049996376
		 -0.00015830994 -0.00049996376 -0.00017499924 -0.00049996376 -0.00019168854 -0.00049996376
		 -0.00020831823 -0.00049996376 -0.00022500753 -0.00049996376 -0.00024163723 -0.00049996376
		 -0.00025832653 -0.00049996376 -0.00027495623 -0.00049996376 -0.00029164553 -0.00049996376
		 -0.00030833483 -0.00049996376 -0.00032496452 -0.00049996376 -0.00034165382 -0.00049996376
		 -0.00035834312 -0.00049996376 -0.00037497282 -0.00049996376 -0.00039166212 -0.00049996376
		 -0.00040835142 -0.00049996376 -0.00042498112 -0.00049996376 -0.00044167042 -0.00049996376
		 -0.00045835972 -0.00049996376 -0.00047498941 -0.00049996376 -0.00049167871 -0.00049996376;
createNode transformGeometry -n "sphere1:transformGeometry1";
	rename -uid "58FE1A08-4FF0-AD82-2240-C7A5850483BA";
	setAttr ".txf" -type "matrix" 0.10452846326765342 0 -0.9945218953682734 0 0 1 0 0
		 0.9945218953682734 0 0.10452846326765342 0 0 0 0 1;
createNode polyPlane -n "sphere1:polyPlane1";
	rename -uid "EF3AF504-4568-6D26-9879-159EC0BD41CD";
	setAttr ".sw" 30;
	setAttr ".sh" 30;
	setAttr ".cuv" 2;
createNode transformGeometry -n "sphere1:transformGeometry2";
	rename -uid "0828BBE3-48CC-CFF6-1212-1EAF71481ADC";
	setAttr ".txf" -type "matrix" 1 0 0 0 0 0 1 0 0 -1 0 0 0 0 0 1;
createNode polyNormalizeUV -n "polyNormalizeUV1";
	rename -uid "91932D6E-4FF5-AFF6-81E1-F894D084659A";
	setAttr ".uopa" yes;
	setAttr ".ics" -type "componentList" 1 "f[0:3599]";
	setAttr ".ix" -type "matrix" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
	setAttr ".pa" no;
	setAttr ".cot" yes;
createNode polyNormalizeUV -n "polyNormalizeUV2";
	rename -uid "DE10A78B-4BF8-8B1D-02EA-30A378316607";
	setAttr ".uopa" yes;
	setAttr ".ics" -type "componentList" 1 "f[0:3599]";
	setAttr ".ix" -type "matrix" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
	setAttr ".pa" no;
createNode polyNormalizeUV -n "polyNormalizeUV3";
	rename -uid "3E5CC00C-4326-15BF-44D4-0D86E959292D";
	setAttr ".uopa" yes;
	setAttr ".ics" -type "componentList" 1 "f[0:899]";
	setAttr ".ix" -type "matrix" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
	setAttr ".pa" no;
createNode polyNormalizeUV -n "polyNormalizeUV4";
	rename -uid "3385BB46-4988-F58F-7344-90A2F03E53A3";
	setAttr ".uopa" yes;
	setAttr ".ics" -type "componentList" 1 "f[0:3599]";
	setAttr ".ix" -type "matrix" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
	setAttr ".pa" no;
createNode polyNormalizeUV -n "polyNormalizeUV5";
	rename -uid "F0624547-42C6-33BE-06E8-158B8F0C03A0";
	setAttr ".uopa" yes;
	setAttr ".ics" -type "componentList" 1 "f[0:899]";
	setAttr ".ix" -type "matrix" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
	setAttr ".pa" no;
select -ne :time1;
	setAttr ".o" 1;
	setAttr ".unw" 1;
select -ne :hardwareRenderingGlobals;
	setAttr ".otfna" -type "stringArray" 22 "NURBS Curves" "NURBS Surfaces" "Polygons" "Subdiv Surface" "Particles" "Particle Instance" "Fluids" "Strokes" "Image Planes" "UI" "Lights" "Cameras" "Locators" "Joints" "IK Handles" "Deformers" "Motion Trails" "Components" "Hair Systems" "Follicles" "Misc. UI" "Ornaments"  ;
	setAttr ".otfva" -type "Int32Array" 22 0 1 1 1 1 1
		 1 1 1 0 0 0 0 0 0 0 0 0
		 0 0 0 0 ;
	setAttr ".fprt" yes;
select -ne :renderPartition;
	setAttr -s 3 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 6 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :defaultRenderUtilityList1;
select -ne :defaultRenderingList1;
select -ne :defaultTextureList1;
select -ne :initialShadingGroup;
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :defaultRenderGlobals;
	addAttr -ci true -h true -sn "dss" -ln "defaultSurfaceShader" -dt "string";
	setAttr ".dss" -type "string" "lambert1";
select -ne :defaultResolution;
	setAttr ".pa" 1;
select -ne :hardwareRenderGlobals;
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
connectAttr "polyNormalizeUV4.out" "sphere1:pSphereShape1.i";
connectAttr "sphere1:polyTweakUV1.uvtk[0]" "sphere1:pSphereShape1.uvst[0].uvtw";
connectAttr "polyNormalizeUV5.out" "sphere1:pPlaneShape1.i";
relationship "link" ":lightLinker1" ":initialShadingGroup.message" ":defaultLightSet.message";
relationship "link" ":lightLinker1" ":initialParticleSE.message" ":defaultLightSet.message";
relationship "link" ":lightLinker1" "sphere1:surfaceShader1SG.message" ":defaultLightSet.message";
relationship "shadowLink" ":lightLinker1" ":initialShadingGroup.message" ":defaultLightSet.message";
relationship "shadowLink" ":lightLinker1" ":initialParticleSE.message" ":defaultLightSet.message";
relationship "shadowLink" ":lightLinker1" "sphere1:surfaceShader1SG.message" ":defaultLightSet.message";
connectAttr "layerManager.dli[0]" "defaultLayer.id";
connectAttr "renderLayerManager.rlmi[0]" "defaultRenderLayer.rlid";
connectAttr "sphere1:file1.oc" "sphere1:surfaceShader1.oc";
connectAttr "sphere1:surfaceShader1.oc" "sphere1:surfaceShader1SG.ss";
connectAttr "sphere1:pSphereShape1.iog" "sphere1:surfaceShader1SG.dsm" -na;
connectAttr "sphere1:pPlaneShape1.iog" "sphere1:surfaceShader1SG.dsm" -na;
connectAttr "sphere1:surfaceShader1SG.msg" "sphere1:materialInfo1.sg";
connectAttr "sphere1:surfaceShader1.msg" "sphere1:materialInfo1.m";
connectAttr "sphere1:file1.msg" "sphere1:materialInfo1.t" -na;
connectAttr ":defaultColorMgtGlobals.cme" "sphere1:file1.cme";
connectAttr ":defaultColorMgtGlobals.cfe" "sphere1:file1.cmcf";
connectAttr ":defaultColorMgtGlobals.cfp" "sphere1:file1.cmcp";
connectAttr ":defaultColorMgtGlobals.wsn" "sphere1:file1.ws";
connectAttr "sphere1:place2dTexture1.c" "sphere1:file1.c";
connectAttr "sphere1:place2dTexture1.tf" "sphere1:file1.tf";
connectAttr "sphere1:place2dTexture1.rf" "sphere1:file1.rf";
connectAttr "sphere1:place2dTexture1.mu" "sphere1:file1.mu";
connectAttr "sphere1:place2dTexture1.mv" "sphere1:file1.mv";
connectAttr "sphere1:place2dTexture1.s" "sphere1:file1.s";
connectAttr "sphere1:place2dTexture1.wu" "sphere1:file1.wu";
connectAttr "sphere1:place2dTexture1.wv" "sphere1:file1.wv";
connectAttr "sphere1:place2dTexture1.re" "sphere1:file1.re";
connectAttr "sphere1:place2dTexture1.of" "sphere1:file1.of";
connectAttr "sphere1:place2dTexture1.r" "sphere1:file1.ro";
connectAttr "sphere1:place2dTexture1.n" "sphere1:file1.n";
connectAttr "sphere1:place2dTexture1.vt1" "sphere1:file1.vt1";
connectAttr "sphere1:place2dTexture1.vt2" "sphere1:file1.vt2";
connectAttr "sphere1:place2dTexture1.vt3" "sphere1:file1.vt3";
connectAttr "sphere1:place2dTexture1.vc1" "sphere1:file1.vc1";
connectAttr "sphere1:place2dTexture1.o" "sphere1:file1.uv";
connectAttr "sphere1:place2dTexture1.ofs" "sphere1:file1.fs";
connectAttr "sphere1:polySphere1.out" "sphere1:polyTweakUV1.ip";
connectAttr "sphere1:polyTweakUV1.out" "sphere1:transformGeometry1.ig";
connectAttr "sphere1:polyPlane1.out" "sphere1:transformGeometry2.ig";
connectAttr "sphere1:transformGeometry1.og" "polyNormalizeUV1.ip";
connectAttr "sphere1:pSphereShape1.wm" "polyNormalizeUV1.mp";
connectAttr "polyNormalizeUV1.out" "polyNormalizeUV2.ip";
connectAttr "sphere1:pSphereShape1.wm" "polyNormalizeUV2.mp";
connectAttr "sphere1:transformGeometry2.og" "polyNormalizeUV3.ip";
connectAttr "sphere1:pPlaneShape1.wm" "polyNormalizeUV3.mp";
connectAttr "polyNormalizeUV2.out" "polyNormalizeUV4.ip";
connectAttr "sphere1:pSphereShape1.wm" "polyNormalizeUV4.mp";
connectAttr "polyNormalizeUV3.out" "polyNormalizeUV5.ip";
connectAttr "sphere1:pPlaneShape1.wm" "polyNormalizeUV5.mp";
connectAttr "sphere1:surfaceShader1SG.pa" ":renderPartition.st" -na;
connectAttr "sphere1:surfaceShader1.msg" ":defaultShaderList1.s" -na;
connectAttr "sphere1:place2dTexture1.msg" ":defaultRenderUtilityList1.u" -na;
connectAttr "defaultRenderLayer.msg" ":defaultRenderingList1.r" -na;
connectAttr "sphere1:file1.msg" ":defaultTextureList1.tx" -na;
// End of sphere_unit_test_geom.ma
