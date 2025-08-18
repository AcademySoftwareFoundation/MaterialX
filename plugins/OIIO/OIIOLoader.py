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
            print(f"create buffer with width: {spec.width}, height: {spec.height}, channels: {spec.nchannels}")

            # Read the image data using the correct OIIO Python API (returns a bytes object)
            print(f"Reading image data from '{file_path_str}' with spec: {spec}")
            data = img_input.read_image(0, 0, 0, spec.nchannels, spec.format)
            if len(data) > 0:
                print(f"Done Reading image data from '{file_path_str}' with spec: {spec}")

            if data is None:
                print(f"Error: Could not read image data from '{file_path_str}'")
                return None
            else:
                print(f"Successfully read image data from '{file_path_str}'")

            # Copy the data into the MaterialX image resource buffer
            resource_buffer_ptr = mx_image.getResourceBuffer()
            # Calculate the size in bytes
            print("Before COPY")
            bytes_per_channel = spec.format.size()
            print("Before COPY 2")
            total_bytes = spec.width * spec.height * spec.nchannels * bytes_per_channel
            print("Before COPY 3")
            print(f"Total bytes to copy: {total_bytes} (width: {spec.width}, height: {spec.height}, channels: {spec.nchannels})")
            # Debug: print type and value of resource_buffer
            print("resource_buffer type:", type(resource_buffer_ptr))
            print("resource_buffer value:", resource_buffer_ptr)
            # Attempt memmove as before (may need to adjust after seeing output)
            try:
                ctypes.memmove(resource_buffer_ptr, (ctypes.c_char * total_bytes).from_buffer_copy(data), total_bytes)
            except Exception as e:
                print("memmove error:", e)

            img_input.close()

            print(f"Image loaded via LOADER:")
            print(f"  Dimensions: {mx_image.getWidth()}x{mx_image.getHeight()}")
            print(f"  Channels: {mx_image.getChannelCount()}")
            print(f"  Base type: {mx_image.getBaseType()}")
            return mx_image            

        except Exception as e:
            print(f"Error loading image from '{file_path_str}': {str(e)}")
            return None
        
        return None

    def saveImage(self, filePath, image, verticalFlip=False):
        """
        Save a MaterialX image to disk using OpenImageIO.
        Args:
            filePath (MaterialX.FilePath): Path to save the image
            image (MaterialX.Image): The MaterialX image object
            verticalFlip (bool): Whether to vertically flip the image on save
        Returns:
            bool: True if successful, False otherwise
        """
        file_path_str = filePath.asString()
        width = image.getWidth()
        height = image.getHeight()
        channels = image.getChannelCount()
        mx_basetype = image.getBaseType()
        oiio_format = self._materialx_to_oiio_type(mx_basetype)
        if oiio_format is None:
            print(f"Error: Unsupported MaterialX base type for OIIO: {mx_basetype}")
            return False

        try:
            spec = oiio.ImageSpec(width, height, channels, oiio_format)
            img_output = oiio.ImageOutput.create(file_path_str)
            if not img_output:
                print(f"Error: Could not create output for '{file_path_str}'")
                return False
            if not img_output.open(file_path_str, spec):
                print(f"Error: Could not open '{file_path_str}' for writing")
                return False

            # Use the pointer version for buffer
            resource_buffer_ptr = image.getResourceBufferPtr()
            total_bytes = width * height * channels * image.getBaseStride()
            buf_type = ctypes.c_char * total_bytes
            buffer = buf_type.from_address(resource_buffer_ptr)

            if verticalFlip:
                scanline_size = width * channels * image.getBaseStride()
                # Calculate pointer to last scanline
                last_line_addr = resource_buffer_ptr + (height - 1) * scanline_size
                last_line_ptr = ctypes.c_void_p(last_line_addr)
                # Create a buffer for the flipped image (OIIO expects a buffer, not just a pointer)
                # We'll copy and flip the data in Python for simplicity
                flipped = bytearray(total_bytes)
                for y in range(height):
                    src_offset = (height - 1 - y) * scanline_size
                    dst_offset = y * scanline_size
                    flipped[dst_offset:dst_offset+scanline_size] = buffer[src_offset:src_offset+scanline_size]
                flipped_buffer = (ctypes.c_char * total_bytes).from_buffer(flipped)
                success = img_output.write_image(flipped_buffer)
            else:
                success = img_output.write_image(buffer)

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
        print('---------------------------------------')
        out_path = mx.FilePath("saved_image.exr")
        if handler.saveImage(out_path, mx_image):
            print(f"Image saved to {out_path.asString()}")
        else:
            print("Failed to save image.")
    else:
        print("Failed to load image via handler.")

if __name__ == "__main__":
    main()
