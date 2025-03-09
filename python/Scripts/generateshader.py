#!/usr/bin/env python
'''
Generate shader code for each renderable element in a MaterialX document or folder.
The currently supported target languages are GLSL, ESSL, MSL, OSL, and MDL.
'''

import sys, os, argparse, subprocess

import MaterialX as mx
import MaterialX.PyMaterialXGenGlsl as mx_gen_glsl
import MaterialX.PyMaterialXGenMdl as mx_gen_mdl
import MaterialX.PyMaterialXGenMsl as mx_gen_msl
import MaterialX.PyMaterialXGenOsl as mx_gen_osl
import MaterialX.PyMaterialXGenShader as mx_gen_shader

def validateCode(sourceCodeFile, codevalidator, codevalidatorArgs):
    if codevalidator:
        cmd = codevalidator.split()
        cmd.append(sourceCodeFile)
        if codevalidatorArgs:
            cmd.append(codevalidatorArgs)
        cmd_flatten ='----- Run Validator: '
        for c in cmd:
            cmd_flatten += c + ' '
        print(cmd_flatten)
        try:
            output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            return output.decode(encoding='utf-8')
        except subprocess.CalledProcessError as out:                                                                                                   
            return (out.output.decode(encoding='utf-8'))
    return ""

def getMaterialXFiles(rootPath):
    filelist = []
    if os.path.isdir(rootPath): 
        for subdir, dirs, files in os.walk(rootPath):
            for file in files:
                if file.endswith('mtlx'):
                    filelist.append(os.path.join(subdir, file)) 
    else:
        filelist.append( rootPath )

    return filelist

def main():
    parser = argparse.ArgumentParser(description='Generate shader code for each renderable element in a MaterialX document or folder.')
    parser.add_argument('--path', dest='paths', action='append', nargs='+', help='An additional absolute search path location (e.g. "/projects/MaterialX")')
    parser.add_argument('--library', dest='libraries', action='append', nargs='+', help='An additional relative path to a custom data library folder (e.g. "libraries/custom")')
    parser.add_argument('--target', dest='target', default='glsl', help='Target shader generator to use (e.g. "glsl, osl, mdl, essl, vulkan"). Default is glsl.')
    parser.add_argument('--outputPath', dest='outputPath', help='File path to output shaders to. If not specified, is the location of the input document is used.')
    parser.add_argument('--validator', dest='validator', nargs='?', const=' ', type=str, help='Name of executable to perform source code validation.')
    parser.add_argument('--validatorArgs', dest='validatorArgs', nargs='?', const=' ', type=str, help='Optional arguments for code validator.')
    parser.add_argument('--vulkanGlsl', dest='vulkanCompliantGlsl', default=False, type=bool, help='Set to True to generate Vulkan-compliant GLSL when using the genglsl target.')
    parser.add_argument('--shaderInterfaceType', dest='shaderInterfaceType', default=0, type=int, help='Set the type of shader interface to be generated')
    parser.add_argument(dest='inputFilename', help='Path to input document or folder containing input documents.')
    opts = parser.parse_args()

    # Load standard and custom data libraries.
    stdlib = mx.createDocument()
    searchPath = mx.getDefaultDataSearchPath()
    libraryFolders = []
    if opts.paths:
        for pathList in opts.paths:
            for path in pathList:
                searchPath.append(path)
    if opts.libraries:
        for libraryList in opts.libraries:
            for library in libraryList:
                libraryFolders.append(library)
    libraryFolders.extend(mx.getDefaultDataLibraryFolders())
    mx.loadLibraries(libraryFolders, searchPath, stdlib)

    # Generate shaders for each input document.
    for inputFilename in getMaterialXFiles(opts.inputFilename):        
        doc = mx.createDocument()
        try:
            mx.readFromXmlFile(doc, inputFilename)
            doc.setDataLibrary(stdlib)
        except mx.ExceptionFileMissing as err:
            print('Generation failed: "', err, '"')
            sys.exit(-1)

        print('---------- Generate code for file: ', inputFilename, '--------------------')

        valid, msg = doc.validate()
        if not valid:
            print('Validation warnings for input document:')
            print(msg)

        gentarget = 'glsl'
        if opts.target:
            gentarget = opts.target
        if gentarget == 'osl':
            shadergen = mx_gen_osl.OslShaderGenerator.create()
        elif gentarget == 'mdl':
            shadergen = mx_gen_mdl.MdlShaderGenerator.create()
        elif gentarget == 'essl':
            shadergen = mx_gen_glsl.EsslShaderGenerator.create()
        elif gentarget == 'vulkan':
            shadergen = mx_gen_glsl.VkShaderGenerator.create()
        elif gentarget == 'msl':
            shadergen = mx_gen_msl.MslShaderGenerator.create()
        else:
            shadergen = mx_gen_glsl.GlslShaderGenerator.create()
                
        codeSearchPath = searchPath
        codeSearchPath.append(os.path.dirname(inputFilename))
        context = mx_gen_shader.GenContext(shadergen)
        context.registerSourceCodeSearchPath(codeSearchPath)
        shadergen.registerTypeDefs(doc);

        # If we're generating Vulkan-compliant GLSL then set the binding context
        if opts.vulkanCompliantGlsl:
            bindingContext = mx_gen_glsl.GlslResourceBindingContext.create(0,0)
            context.pushUserData('udbinding', bindingContext)

        genoptions = context.getOptions() 
        if opts.shaderInterfaceType == 0 or opts.shaderInterfaceType == 1:
            genoptions.shaderInterfaceType = mx_gen_shader.ShaderInterfaceType(opts.shaderInterfaceType)
        else:
            genoptions.shaderInterfaceType = mx_gen_shader.ShaderInterfaceType.SHADER_INTERFACE_COMPLETE

        print('- Set up CMS ...')
        cms = mx_gen_shader.DefaultColorManagementSystem.create(shadergen.getTarget())  
        cms.loadLibrary(doc)
        shadergen.setColorManagementSystem(cms)  

        print('- Set up Units ...')
        unitsystem = mx_gen_shader.UnitSystem.create(shadergen.getTarget())
        registry = mx.UnitConverterRegistry.create()
        distanceTypeDef = doc.getUnitTypeDef('distance')
        registry.addUnitConverter(distanceTypeDef, mx.LinearUnitConverter.create(distanceTypeDef))
        angleTypeDef = doc.getUnitTypeDef('angle')
        registry.addUnitConverter(angleTypeDef, mx.LinearUnitConverter.create(angleTypeDef))
        unitsystem.loadLibrary(stdlib)
        unitsystem.setUnitConverterRegistry(registry)
        shadergen.setUnitSystem(unitsystem)
        genoptions.targetDistanceUnit = 'meter'

        pathPrefix = ''
        if opts.outputPath and os.path.exists(opts.outputPath):
            pathPrefix = opts.outputPath + os.path.sep
        else:
            pathPrefix = os.path.dirname(os.path.abspath(inputFilename))
        print('- Shader output path: ' + pathPrefix)

        failedShaders = ""
        for elem in mx_gen_shader.findRenderableElements(doc):
            elemName = elem.getName()
            print('-- Generate code for element: ' + elemName)
            elemName = mx.createValidName(elemName)
            shader = shadergen.generate(elemName, elem, context)        
            if shader:
                # Use extension of .vert and .frag as it's type is
                # recognized by glslangValidator
                if gentarget in ['glsl', 'essl', 'vulkan', 'msl']:
                    pixelSource = shader.getSourceCode(mx_gen_shader.PIXEL_STAGE)
                    filename = pathPrefix + "/" + shader.getName() + "." + gentarget + ".frag"
                    print('--- Wrote pixel shader to: ' + filename)
                    file = open(filename, 'w+')
                    file.write(pixelSource)
                    file.close()
                    errors = validateCode(filename, opts.validator, opts.validatorArgs)                

                    vertexSource = shader.getSourceCode(mx_gen_shader.VERTEX_STAGE)
                    filename = pathPrefix + "/" + shader.getName() + "." + gentarget + ".vert"
                    print('--- Wrote vertex shader to: ' + filename)
                    file = open(filename, 'w+')
                    file.write(vertexSource)
                    file.close()
                    errors += validateCode(filename, opts.validator, opts.validatorArgs)

                else:
                    pixelSource = shader.getSourceCode(mx_gen_shader.PIXEL_STAGE)
                    filename = pathPrefix + "/" + shader.getName() + "." + gentarget
                    print('--- Wrote pixel shader to: ' + filename)
                    file = open(filename, 'w+')
                    file.write(pixelSource)
                    file.close()
                    errors = validateCode(filename, opts.validator, opts.validatorArgs)

                if errors != "":
                    print("--- Validation failed for element: ", elemName)
                    print("----------------------------")
                    print('--- Error log: ', errors)
                    print("----------------------------")
                    failedShaders += (elemName + ' ')
                else:
                    print("--- Validation passed for element:", elemName)

            else:
                print("--- Validation failed for element:", elemName)
                failedShaders += (elemName + ' ')

        if failedShaders != "":
            sys.exit(-1)

if __name__ == '__main__':
    main()
