"""
MaterialX ImageLoader implementation using OpenImageIO package.

This module provides a MaterialX-compatible ImageLoader implementation using OpenImageIO (OIIO)
for reading various image formats commonly used in graphics and VFX workflows.

Classes:
    OiioImageLoader: MaterialX ImageLoader implementation using OpenImageIO
    ImageLoaderHelper: Helper class providing NumPy-based image operations

The OiioImageLoader class properly inherits from MaterialX.ImageLoader and implements
the required interface methods (loadImage, saveImage) to work within the MaterialX
rendering pipeline.

Requirements:
    - MaterialX Python bindings
    - OpenImageIO Python package
    - NumPy (for helper functionality)

Usage:
    # Create MaterialX-compatible image loader
    loader = OiioImageLoader()
    
    # Use with MaterialX ImageHandler
    image_handler = MaterialX.ImageHandler.create(loader)
    
    # Or use helper for NumPy operations
    helper = ImageLoaderHelper(loader)
    numpy_array = helper.load_image("path/to/image.exr")
"""

import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import OpenImageIO as oiio
import numpy as np
import ctypes
from typing import Optional, Tuple, Union
import os


class OiioImageLoader(mx_render.ImageLoader):
    """
    A MaterialX ImageLoader implementation that uses OpenImageIO to read image files.
    
    Inherits from MaterialX.ImageLoader and implements the required interface methods.
    Supports common image formats like PNG, JPEG, TIFF, EXR, HDR, etc.
    """
    
    def __init__(self):
        """Initialize the OiioImageLoader and set supported extensions."""
        super().__init__()
        
        # Set all extensions supported by OpenImageIO
        # Add all standard extensions. This is a string set which just has the extension names list (no structure)
        self._extensions = set()
        self._extensions.update([
            "exr", "sxr", "mxr", "tif", "tiff", "tx", "env", "sm", "vsm",
            "jpg", "jpe", "jpeg", "jif", "jfif", "jfi", "bmp", "dib",
            "cin", "dds", "dpx", "fits", "hdr", "rgbe", "ico", "iff", "z",
            "null", "nul", "png", "pnm", "ppm", "pgm", "pbm", "pfm", "psd",
            "pdd", "psb", "rla", "sgi", "rgb", "rgba", "bw", "int", "inta",
            "pic", "tga", "tpic", "term", "webp", "zfile"
        ])

        # Get list from oiio itself: e.g.
        # openexr:exr,sxr,mxr;tiff:tif,tiff,tx,env,sm,vsm;jpeg:jpg,jpe,jpeg,jif,jfif,jfi;bmp:bmp,dib;cineon:cin;dds:dds;dpx:dpx;fits:fits;hdr:hdr,rgbe;ico:ico;iff:iff,z;null:null,nul;png:png;pnm:ppm,pgm,pbm,pnm,pfm;psd:psd,pdd,psb;rla:rla;sgi:sgi,rgb,rgba,bw,int,inta;softimage:pic;targa:tga,tpic;term:term;webp:webp;zfile:zfile        
        #oiio_extensions = oiio.get_string_attribute("extension_list")
        #print(f"Supported OIIO extensions: {oiio_extensions}")
        
        self.last_loaded_path = None
        self.last_spec = None

    def supportedExtensions(self):
        print(f"Check supported extensions: {self._extensions}")
        return self._extensions

    def loadImage(self, filePath):
        """
        Load an image from the file system (MaterialX interface method).
        
        Args:
            filePath (MaterialX.FilePath): Path to the image file
            
        Returns:
            MaterialX.ImagePtr: MaterialX Image object or None if loading fails
        """
        file_path_str = filePath.asString()
        print(f"------ Call oiio loader {file_path_str}")
        
        if not os.path.exists(file_path_str):
            print(f"Error: File '{file_path_str}' does not exist")
            return None
        
        try:
            # Open the image file
            img_input = oiio.ImageInput.open(file_path_str)
            if not img_input:
                print(f"Error: Could not open '{file_path_str}' - {oiio.geterror()}")
                return None
            
            # Get image specifications
            spec = img_input.spec()
            self.last_spec = spec
            self.last_loaded_path = file_path_str
            
            # Determine MaterialX base type from OIIO format
            base_type = self._oiio_to_materialx_type(spec.format.basetype)
            if base_type is None:
                img_input.close()
                print(f"Error: Unsupported image format for '{file_path_str}'")
                return None
            
            # Create MaterialX image
            mx_image = mx_render.Image.create(spec.width, spec.height, spec.nchannels, base_type)
            mx_image.createResourceBuffer()
            
            # Read the image data using the correct OIIO Python API
            data = img_input.read_image(0, 0, 0, spec.nchannels, spec.format)
            img_input.close()

            if data is None:
                print(f"Error: Could not read image data from '{file_path_str}'")
                return None

            # Read the image data directly into the MaterialX image buffer (like C++ version)
            success = img_input.read_image(0, 0, 0, spec.nchannels, spec.format, mx_image.getResourceBuffer())
            try:
                img_input = oiio.ImageInput.open(file_path_str)
                if not img_input:
                    print(f"Error: Could not open '{file_path_str}' - {oiio.geterror()}")
                    return None

                spec = img_input.spec()
                base_type = self._oiio_to_materialx_type(spec.format.basetype)
                if base_type is None:
                    img_input.close()
                    print(f"Error: Unsupported image format for '{file_path_str}'")
                    return None

                mx_image = mx_render.Image.create(spec.width, spec.height, spec.nchannels, base_type)
                mx_image.createResourceBuffer()

                success = img_input.read_image(0, 0, 0, spec.nchannels, spec.format, mx_image.getResourceBuffer())
                img_input.close()
                if not success:
                    print(f"Error: Could not read image data from '{file_path_str}'")
                    return None

                return mx_image
            except Exception as e:
                print(f"Error loading image '{file_path_str}': {str(e)}")
                return None
                return False
            
            # Create OIIO image spec
            spec = oiio.ImageSpec(width, height, channels, oiio_format)
            
            # Create output
            img_output = oiio.ImageOutput.create(file_path_str)
            if not img_output:
                print(f"Error: Could not create output for '{file_path_str}'")
                return False
            
            # Open for writing
            if not img_output.open(file_path_str, spec):
                print(f"Error: Could not open '{file_path_str}' for writing")
                return False
            
            # Write image data
            resource_buffer = image.getResourceBuffer()
            if verticalFlip:
                # Calculate scanline size and write with negative stride for vertical flip
                scanline_size = width * channels * image.getBaseStride()
                # Calculate pointer to last scanline
                last_line_ptr = resource_buffer + (height - 1) * scanline_size
                success = img_output.write_image(
                    oiio_format,
                    last_line_ptr,
                    oiio.AutoStride,  # x stride
                    -scanline_size,   # negative y stride for flip
                    oiio.AutoStride   # z stride
                )
            else:
                success = img_output.write_image(oiio_format, resource_buffer)
            
            img_output.close()
            
            if success:
                print(f"Successfully saved image to '{file_path_str}'")
                return True
            else:
                print(f"Error writing image data to '{file_path_str}'")
                return False
                
        except Exception as e:
            print(f"Error saving image to '{file_path_str}': {str(e)}")
            return False
    
    def _oiio_to_materialx_type(self, oiio_basetype):
        """Convert OIIO base type to MaterialX Image base type."""
        type_mapping = {
            oiio.UINT8: mx_render.BaseType.UINT8,
            oiio.INT8: mx_render.BaseType.INT8,
            oiio.UINT16: mx_render.BaseType.UINT16,              
            oiio.INT16: mx_render.BaseType.INT16,
            oiio.HALF: mx_render.BaseType.HALF,
            oiio.FLOAT: mx_render.BaseType.FLOAT
        }
        return_val = type_mapping.get(oiio_basetype, None)
        print(f"OIIO to MaterialX type mapping: {return_val} from {oiio_basetype}")
        return return_val

    def _materialx_to_oiio_type(self, mx_basetype):
        """Convert MaterialX Image base type to OIIO type."""
        type_mapping = {
            mx_render.BaseType.UINT8: oiio.UINT8,
            mx_render.BaseType.UINT16: oiio.UINT16,
            mx_render.BaseType.INT8: oiio.INT8,
            mx_render.BaseType.INT16: oiio.INT16,
            mx_render.BaseType.HALF: oiio.HALF,
            mx_render.BaseType.FLOAT: oiio.FLOAT,
        }
        return type_mapping.get(mx_basetype, None)




def main():
    """
    Example usage of the OiioImageLoader class with MaterialX ImageHandler.
    """
    # Instantiate your Python-side loader
    loader = OiioImageLoader()
    # Create a handler and add your loader
    handler = mx_render.ImageHandler.create(loader)
    #handler.addLoader(loader)

    # Example: Load and save an image (replace with actual image path)
    test_image_path = "test.exr"  # Replace with actual path
    mx_filepath = mx.FilePath(test_image_path)

    print("MaterialX ImageHandler Example Usage")
    print("=" * 45)

    # Load image using handler API
    mx_image = handler.acquireImage(mx_filepath)
    if mx_image:
        print(f"Image loaded via handler:")
        print(f"  Dimensions: {mx_image.getWidth()}x{mx_image.getHeight()}")
        print(f"  Channels: {mx_image.getChannelCount()}")
        print(f"  Base type: {mx_image.getBaseType()}")

        # Save image using handler API (to a new file)
        out_path = mx.FilePath("saved_image.png")
        if handler.saveImage(out_path, mx_image):
            print(f"Image saved to {out_path.asString()}")
        else:
            print("Failed to save image.")
    else:
        print("Failed to load image via handler.")

if __name__ == "__main__":
    main()
