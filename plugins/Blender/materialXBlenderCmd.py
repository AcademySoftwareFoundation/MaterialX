import argparse
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import logging
import os, sys

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('Blenderlugin')

# Import blender packages
have_blender = False
try:
    import bpy
    import materialXBlender as mxblender
    logger.info(f"{__name__} module loaded successfully.")
    have_blender = True
except ImportError:
    raise ImportError(f"{__name__} not loaded successfully.")


def haveVersion(major, minor, patch):
    '''
    Check if the current vesion matches a given version
    ''' 
    imajor, iminor, ipatch = mx.getVersionIntegers()
    if major >= imajor:
        if  major > imajor:
            return True            
        if iminor >= minor:
            if iminor > minor:
                return True 
            if  ipatch >= patch:
                return True
    return False


class BlenderLoader(mx_render.DocumentLoaderPlugin):
    _plugin_name = "BlenderLoader"
    _ui_name = "Import Blender File..."

    def name(self):
        return self._plugin_name

    def uiName(self):
        return self._ui_name

    def supportedExtensions(self):
        return [".blend"]

    def run(self, path):
        export_settings = dict()
        export_settings['file_path'] = path 
        export_settings['selected_objects'] = False

        export_settings['write_geometry'] = False
        export_settings['geometry_format'] = 'GLB'
        export_settings['seperate_files'] = False
        export_settings['write_mesh_material'] = 'EXPORT' 
        export_settings['export_vertex_color'] = False
        export_settings['export_tangents'] = True
        export_settings['export_normals'] = True
        export_settings['export_uv'] = True
        export_settings['export_animation'] = False
        
        export_settings['seperate_materials'] = False
        export_settings['diagram'] = False
        export_settings['write_materials_to_file'] = False

        # Open the blender file
        bpy.ops.wm.open_mainfile(filepath=path)
        converter = mxblender.BlenderMaterialXExporter()
        docs = converter.execute(export_settings)
        if len(docs) > 0:
            logger.info(f"Successfully exported {len(docs)} documents from Blender.")    
            # Print out all document keys
            for key in docs.keys():
                logger.info(f" - Created: {key}")
            # Get first item from docs dict()
            doc = docs[list(docs.keys())[0]]
            return doc
        else:   
            logger.error("No documents were exported from Blender.")
        return None

def main():

    parser = argparse.ArgumentParser(description='Extract MaterialX materials from a Blender file. Optionally export meshes as GLTF files')
    parser.add_argument(dest='inputFileName', help='Root name of image files to examine.')
    parser.add_argument('--writeGeom', dest='writeGeom', default=False, type=bool, help='Set to True to export meshes in GLTF format. Default is False.')
    parser.add_argument('--writeGeomMaterials', dest='writeGeomMaterials', default=False, type=bool, help='Set to True to write materials as part of mesh export. Default is False.')
    parser.add_argument('--separateGeomFile', dest='separateGeomFile', default=False, type=bool, help='Set to True to write meshes to separate GLTF files. Default is False.')
    parser.add_argument('--separateMtlxFile', dest='separateMtlxFile', default=False, type=bool, help='Set to True to write each material to a separate MaterialX file. Default is False.')
    parser.add_argument('--writeMtlxGraph', dest='writeMtlxGraph', default=False, type=bool, help='Set to True to export Mermaid graph for materials. Default is False.')
    parser.add_argument('--outputPath', dest='outputPath', default="./", help='File path to output shaders to. If not specified, is the location of the input document is used.')

    print('Versions:' + ' Blender: ' + bpy.app.version_string + ' MaterialX: ' + mx.getVersionString())

    opts = parser.parse_args()

    haveDesiredVersion = haveVersion(1,38,9)
    if not haveDesiredVersion:
        print("Minimum required version not met. Have version: ", mx.__version__)
        exit(-1)

    inputFileName = opts.inputFileName  
    if not os.path.exists(inputFileName):
        print('Input file does not exist. Exiting')
        exit(-1)
    bpy.ops.wm.open_mainfile(filepath=inputFileName)

    outputFileName = mx.FilePath(inputFileName)
    outputFileName.removeExtension()
    outputFileName.addExtension('mtlx')
    outputFileName = outputFileName.getBaseName()
    outputFilePath = mx.FilePath(opts.outputPath)
    pathExists = os.path.exists(outputFilePath.asString())
    if not pathExists:
        print('Created folder: ', outputFilePath.asString())
        os.makedirs(outputFilePath.asString())
    outputFilePath = outputFilePath / outputFileName

    export_settings = dict()
    export_settings['file_path'] = outputFilePath.asString() 
    export_settings['selected_objects'] = False

    export_settings['write_geometry'] = opts.writeGeom
    export_settings['geometry_format'] = 'GLB'
    export_settings['seperate_files'] = opts.separateGeomFile
    print('opts.writeGeomMaterials', opts.writeGeomMaterials)
    export_settings['write_mesh_material'] = 'EXPORT' if opts.writeGeomMaterials else 'NONE' 
    export_settings['export_vertex_color'] = False
    export_settings['export_tangents'] = True
    export_settings['export_normals'] = True
    export_settings['export_uv'] = True
    export_settings['export_animation'] = False
    
    export_settings['seperate_materials'] = opts.separateMtlxFile
    export_settings['diagram'] = opts.writeMtlxGraph
    export_settings['write_materials_to_file'] = True

    #librarySearchPath = mx.FilePath(os.path.abspath(__file__))
    #librarySearchPath = librarySearchPath.getParentPath()
    #print('> MaterialX library search path: ', librarySearchPath.asString())
    #export_settings['library_search_path'] = librarySearchPath.asString()

    converter = mxblender.BlenderMaterialXExporter()
    return converter.execute(export_settings)

if __name__ == '__main__':
    main()
else:
    logger.info("Successfully loaded Blender plugin module.")
    loader = BlenderLoader()
    manager = mx_render.getPluginManager()
    manager.registerPlugin(loader)    

