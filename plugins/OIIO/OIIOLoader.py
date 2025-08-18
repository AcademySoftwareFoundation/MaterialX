"""
MaterialX ImageLoader implementation using OpenImageIO package.

This module provides a MaterialX-compatible ImageLoader implementation using OpenImageIO (OIIO).

- Dependencies: OpenImageIO PyPi package (version 3.0.6.1) 
- API Docs: https://openimageio.readthedocs.io/en/v3.0.6.1/)
"""
import MaterialX as mx
import MaterialX.PyMaterialXRender as mx_render
import OpenImageIO as oiio
import numpy as np
import ctypes
import os
import logging
import argparse

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("OIIOLoad")

have_matplot = False
try:
    import matplotlib.pyplot as plt
    have_matplot = True
except ImportError:
    logger.warning("matplotlib module not found. Image display will be disabled.")

class OiioImageLoader(mx_render.ImageLoader):
    """
    A MaterialX ImageLoader implementation that uses OpenImageIO to read image files.
    
    Inherits from MaterialX.ImageLoader and implements the required interface methods.
    Supports common image formats like PNG, JPEG, TIFF, EXR, HDR, etc.
    """
    
    def __init__(self):
        """Initialize the OiioImageLoader and set supported extensions."""
        super().__init__()
        
        # Set all extensions supported by OpenImageIO. e.g.
        # openexr:exr,sxr,mxr;tiff:tif,tiff,tx,env,sm,vsm;jpeg:jpg,jpe,jpeg,jif,jfif,jfi;bmp:bmp,dib;cineon:cin;dds:dds;dpx:dpx;fits:fits;hdr:hdr,rgbe;ico:ico;iff:iff,z;null:null,nul;png:png;pnm:ppm,pgm,pbm,pnm,pfm;psd:psd,pdd,psb;rla:rla;sgi:sgi,rgb,rgba,bw,int,inta;softimage:pic;targa:tga,tpic;term:term;webp:webp;zfile:zfile        
        self._extensions = set()
        oiio_extensions = oiio.get_string_attribute("extension_list")
        # Split string by ";"
        for group in oiio_extensions.split(";"):
            # Each group is like "openexr:exr,sxr,mxr"
            if ":" in group:
                _, exts = group.split(":", 1)
                self._extensions.update(ext.strip() for ext in exts.split(","))
            else:
                self._extensions.update(ext.strip() for ext in group.split(","))
        logger.debug(f"Cache supported extensions: {self._extensions}")

        self.last_loaded_path = None
        self.last_spec = None

    def supportedExtensions(self):
        logger.info(f"Supported OIIO supported extensions: {self._extensions}")
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
        logger.info(f"Load using OIIO loader: {file_path_str}")
        
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
            logger.debug(f"Create buffer with width: {spec.width}, height: {spec.height}, channels: {spec.nchannels}")

            # Read the image data using the correct OIIO Python API (returns a bytes object)
            logger.debug(f"Reading image data from '{file_path_str}' with spec: {spec}")
            data = img_input.read_image(0, 0, 0, spec.nchannels, spec.format)
            if len(data) > 0:
                logger.debug(f"Done Reading image data from '{file_path_str}' with spec: {spec}")
            else:
                logger.error(f"Could not read image data.")
                return None
            
            if have_matplot:
                flat = data.reshape(spec.height, spec.width, spec.nchannels)
                norm = np.clip(flat, 0.0, 1.0)
                rgb = norm[..., :3] if spec.nchannels >= 3 else np.repeat(norm[..., :1], 3, axis=-1)
                plt.imshow(rgb.astype(np.float32))
                plt.axis('off')
                plt.show()

            # Copy the data into the MaterialX image resource buffer
            resource_buffer_ptr = mx_image.getResourceBuffer()
            # Calculate the size in bytes
            bytes_per_channel = spec.format.size()
            total_bytes = spec.width * spec.height * spec.nchannels * bytes_per_channel
            logger.info(f"Total bytes read in: {total_bytes} (width: {spec.width}, height: {spec.height}, channels: {spec.nchannels}, format: {spec.format})")
            try:
                ctypes.memmove(resource_buffer_ptr, (ctypes.c_char * total_bytes).from_buffer_copy(data), total_bytes)
            except Exception as e:
                logger.error(f"Failed to update image resource buffer: {e}")

            img_input.close()

            return mx_image

        except Exception as e:
            print(f"Error loading image from '{file_path_str}': {str(e)}")
            return None
        
        return None   

    def saveImage(self, filePath, image, verticalFlip=False):
        filename = filePath.asString()
        width = image.getWidth()
        height = image.getHeight()
        channels = image.getChannelCount()
        mx_basetype = image.getBaseType()
        oiio_format = self._materialx_to_oiio_type(mx_basetype)
        logger.info(f"mx_basetype: {mx_basetype}, oiio_format: {oiio_format}, base_stride: {image.getBaseStride()}")
        if oiio_format is None:
            logger.error(f"Error: Unsupported MaterialX base type for OIIO: {mx_basetype}")
            return False
        
        buffer = image.getResourceBuffer()
        
        np_type = self._materialx_type_to_np_type(mx_basetype)
        pixels = np.zeros((height, width, channels), dtype=np_type)
        # Copy from buffer to pixels
        try:
            # Calculate total bytes
            base_stride = image.getBaseStride()
            total_bytes = width * height * channels * base_stride
            buf_type = (ctypes.c_char * total_bytes)
            buf = buf_type.from_address(buffer)
            np_buffer = np.frombuffer(buf, dtype=np_type).reshape((height, width, channels))
            np.copyto(pixels, np_buffer)
        except Exception as e:
            logger.error(f"Error copying buffer to pixels: {e}")
            return False
        
        out = oiio.ImageOutput.create(filename)
        if out:
            if np_type is None:
                logger.error(f"Error: Unsupported NumPy type for OIIO: {mx_basetype}")
                return False
            spec = oiio.ImageSpec(width, height, channels, np_type)
            out.open(filename, spec)
            out.write_image(pixels)
            logger.info(f"Image saved to {filename} with width: {width}, height: {height}, channels: {channels}, base type: {mx_basetype}")
            out.close()
            return True        
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
        logger.debug(f"OIIO to MaterialX type mapping: {return_val} from {oiio_basetype}")
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
        return_val = type_mapping.get(mx_basetype, None)
        print(f"MaterialX type mapping: {mx_basetype} to {return_val}")
        return return_val

    def _materialx_type_to_np_type(self, mx_basetype):
        """Map MaterialX base type to NumPy dtype."""
        type_mapping = {
            mx_render.BaseType.UINT8: 'uint8',
            mx_render.BaseType.UINT16: 'uint16',
            mx_render.BaseType.INT8: 'int8',
            mx_render.BaseType.INT16: 'int16',
            mx_render.BaseType.HALF: 'half',
            mx_render.BaseType.FLOAT: 'float',
        }
        return type_mapping.get(mx_basetype, None)


def main():
    """
    Example usage of the OiioImageLoader class with MaterialX ImageHandler.
    """
    parser = argparse.ArgumentParser(description="MaterialX OIIO Image Handler")
    parser.add_argument("path", help="Path to the image file")
    args = parser.parse_args()  

    test_image_path = args.path
    if not os.path.exists(test_image_path):
        logger.error(f"Image file not found: {test_image_path}")
        return      

    # Instantiate your Python-side loader
    loader = OiioImageLoader()
    # Create a handler and add your loader
    handler = mx_render.ImageHandler.create(loader)
    #handler.addLoader(loader)

    # Example: Load and save an image (replace with actual image path)
    #test_image_path = "test.exr"  # Replace with actual path
    mx_filepath = mx.FilePath(test_image_path)

    # Load image using handler API
    logger.info(f"Loading image from path: {mx_filepath.asString()}")
    mx_image = handler.acquireImage(mx_filepath)
    if mx_image:
        logger.info(f"MaterialX Image loaded via Image Handler:")
        logger.info(f"  Dimensions: {mx_image.getWidth()}x{mx_image.getHeight()}")
        logger.info(f"  Channels: {mx_image.getChannelCount()}")
        logger.info(f"  Base type: {mx_image.getBaseType()}")

        # Save image using handler API (to a new file)
        logger.info('** Save Image **')
        out_path = mx.FilePath("saved_" + os.path.basename(test_image_path))
        if handler.saveImage(out_path, mx_image):
            logger.info(f"Image saved to {out_path.asString()}")
        else:
            logger.error("Failed to save image.")
    else:
        logger.error("Failed to load image via handler.")

if __name__ == "__main__":
    main()
