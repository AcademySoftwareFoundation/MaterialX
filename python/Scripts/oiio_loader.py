"""
Sample MaterialX ImageLoader implementation using OpenImageIO package.

This module provides a MaterialX-compatible ImageLoader implementation using OpenImageIO (OIIO).
The test will test loading an image, save it out, and optionally previewing it.

Steps:
    1. Create an OIIOLoader which is derived from the ImageLoader interface class.
    2. Create a new ImageHandler and register the loader with it.
    3. Request to acquire an image using the ImageHandler. An EXR image is requested.
    4. OIIOLoader will return supported extensions and match the requested image format.
    5. As such the OIIOLoader will be requested to load in the EXR image, convert the 
    data and return a MaterialX Image.
    6. Try to acquire the image again. This should returnt the cached MaterialX Image.
    7. Save the image back to disk in the original format.

    The image can optionally be previewed after load before save.

- Python Dependencies: 
    - OpenImageIO (version 3.0.6.1) 
        - API Docs can be found here: https://openimageio.readthedocs.io/en/v3.0.6.1/)
    - numpy : For numerical operations on image data
    - matplotlib : If image preview is desired.
"""
import ctypes
import os
import argparse

import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("OIIOLoad")

try:
    import MaterialX as mx
    import MaterialX.PyMaterialXRender as mx_render
except ImportError:
    logger.error("Required modules not found. Please install MaterialX.")
    raise
try:
    import OpenImageIO as oiio
    import numpy as np
except ImportError:
    logger.error("Required modules not found. Please install OpenImageIO and numpy.")
    raise

have_matplot = False
try:
    import matplotlib.pyplot as plt
    have_matplot = True
except ImportError:
    logger.warning("matplotlib module not found. Image preview display is disabled.")

class OiioImageLoader(mx_render.ImageLoader):
    """
    A MaterialX ImageLoader implementation that uses OpenImageIO to read image files.
    
    Inherits from MaterialX.ImageLoader and implements the required interface methods.
    Supports common image formats like PNG, JPEG, TIFF, EXR, HDR, etc.
    """
    
    def __init__(self):
        """
        Initialize the OiioImageLoader and set supported extensions."""
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

        self.preview = False
        self.identifier = "OpenImageIO Custom Image Loader"
        self.color_space = {}

    def supportedExtensions(self):
        """
        Derived method to return a set of supported image file extensions.
        """
        logger.info(f"OIIO supported extensions: {self._extensions}")
        return self._extensions

    def set_preview(self, value):
        """
        Set whether to preview images when loading and saving

        @param value: Boolean indicating whether to enable preview
        """
        self.preview = value

    def get_identifier(self):
        return "OIIO Custom Loader"

    def previewImage(self, title, data, width, height, nchannels, color_space):
        """
        Utility method to preview an image using matplotlib.
        Handles normalization and dtype for correct display.

        @param title: Title for the preview window
        @param data: Image data array
        @param width: Image width
        @param height: Image height
        @param nchannels: Number of image channels
        @param color_space: Color space of the image
        """
        if not self.preview:
            return

        if have_matplot:
            # If the image is float16 (half), convert to float32
            if data.dtype == np.float16:
                data = data.astype(np.float32)

            flat = data.reshape(height, width, nchannels)
            # Always display as RGB (first 3 channels or repeat if less)
            if nchannels >= 3:
                rgb = flat[..., :3]
            else:
                rgb = np.repeat(flat[..., :1], 3, axis=-1)

            # Determine if normalization is needed
            if np.issubdtype(flat.dtype, np.floating):
                # If float, normalize to [0, 1] for display
                rgb_disp = np.clip(rgb, 0.0, 1.0)
            elif np.issubdtype(flat.dtype, np.integer):
                # If integer, assume 8 or 16 bit, scale if needed
                if flat.dtype == np.uint8:
                    rgb_disp = rgb  # matplotlib expects [0,255] for uint8
                elif flat.dtype == np.uint16:
                    # Scale 16-bit to 8-bit for display
                    rgb_disp = (rgb / 257).astype(np.uint8)
                else:
                    # For other integer types, try to scale to [0,255]
                    rgb_disp = np.clip(rgb, 0, 255).astype(np.uint8)
            else:
                rgb_disp = rgb

            # Set title bar text for the preview window
            fig, ax = plt.subplots()
            ax.imshow(rgb_disp)
            ax.axis("off")
            #fig.patch.set_facecolor("black")
            fig.canvas.manager.set_window_title(title)
            info = f"Dimensions:({width}x{height}), {nchannels} channels, type={data.dtype}, colorspace={color_space}"
            fig.suptitle(title, fontsize=12)
            plt.title(info, fontsize=9) 
            plt.show()

    def loadImage(self, filePath):
        """
        Load an image from the file system (MaterialX interface method).

        @param filePath (MaterialX.FilePath): Path to the image file
        @returns MaterialX.ImagePtr: MaterialX Image object or None if loading fails
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
            color_space = spec.getattribute("oiio:ColorSpace")  
            logger.info(f"ColorSpace: {color_space}")
            self.color_space[file_path_str] = color_space

            # Check channel count
            channels = spec.nchannels
            if channels > 4:
                channels = 4
            
            # Determine MaterialX base type from OIIO format
            base_type = self._oiio_to_materialx_type(spec.format.basetype)
            if base_type is None:
                img_input.close()
                print(f"Error: Unsupported image format for '{file_path_str}'")
                return None
            
            # Create MaterialX image
            mx_image = mx_render.Image.create(spec.width, spec.height, channels, base_type)
            mx_image.createResourceBuffer()
            logger.debug(f"Create buffer with width: {spec.width}, height: {spec.height}, channels: {spec.nchannels} -> {channels}")

            # Read the image data using the correct OIIO Python API (returns a bytes object)
            logger.debug(f"Reading image data from '{file_path_str}' with spec: {spec}")
            data = img_input.read_image(0, 0, 0, channels, spec.format)
            if len(data) > 0:
                logger.debug(f"Done Reading image data from '{file_path_str}' with spec: {spec}")
            else:
                logger.error(f"Could not read image data.")
                return None

            self.previewImage("Loaded MaterialX Image", data, spec.width, spec.height, channels, color_space)

            # Steps:
            # - Copy the OIIO data into the MaterialX image resource buffer            
            resource_buffer_ptr = mx_image.getResourceBuffer()
            bytes_per_channel = spec.format.size()
            total_bytes = spec.width * spec.height * channels * bytes_per_channel
            logger.info(f"Total bytes read in: {total_bytes} (width: {spec.width}, height: {spec.height}, channels: {channels}, format: {spec.format})")
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
        """
        @brief Saves an image to disk using OpenImageIO (OIIO).

        @param filePath The file path where the image will be saved. Expected to have an asString() method.
        @param image The MaterialX image object to save.
        @param verticalFlip Whether to vertically flip the image before saving. (Currently unused.)
        @return True if the image was saved successfully, False otherwise.
        """
        filename = filePath.asString()
        width = image.getWidth()
        height = image.getHeight()

        # Clamp to RGBA
        src_channels = image.getChannelCount()
        channels = min(src_channels, 4)
        if src_channels > 4:
            logger.warning(f"Image has {src_channels} channels. Saving only first {channels} (RGBA).")

        mx_basetype = image.getBaseType()
        oiio_format = self._materialx_to_oiio_type(mx_basetype)
        logger.info(f"mx_basetype: {mx_basetype}, oiio_format: {oiio_format}, base_stride: {image.getBaseStride()}")
        if oiio_format is None:
            logger.error(f"Unsupported MaterialX base type for OIIO: {mx_basetype}")
            return False

        buffer_addr = image.getResourceBuffer()
        np_type = self._materialx_type_to_np_type(mx_basetype)
        if np_type is None:
            logger.error(f"No NumPy dtype mapping for base type: {mx_basetype}")
            return False

        try:
            # Steps: 
            # - Maps the MaterialX base type to OIIO and NumPy types.
            # - Allocates a NumPy array for the pixel data.
            # - Copies the raw buffer from the image into the NumPy array.
            # - Optionally previews the image for debugging.
            # - Creates an OIIO ImageOutput and writes the image to disk.
            #
            base_stride = image.getBaseStride()  # bytes per channel element
            total_bytes = width * height * src_channels * base_stride

            buf_type = (ctypes.c_char * total_bytes)
            buf = buf_type.from_address(buffer_addr)

            np_buffer = np.frombuffer(buf, dtype=np_type)

            # Validate total elements before reshape to catch mismatches early
            expected_elems = width * height * src_channels
            if np_buffer.size != expected_elems:
                logger.error(f"Buffer element count mismatch: got {np_buffer.size}, expected {expected_elems}.")
                return False

            np_buffer = np_buffer.reshape((height, width, src_channels))

            # Keep only up to RGBA
            pixels = np_buffer[..., :channels].copy()

            if verticalFlip:
                logger.info("Applying vertical flip before saving image.")
                pixels = np.flipud(pixels)

            logger.info("Previewing image after load into Image and reload for save...")
            # Remove "saved_" prefix if present
            search_name = filename.replace("saved_", "")
            color_space = "Unknown"
            for key in self.color_space:
                value = self.color_space[key]
                path = os.path.basename(key)
                if path in search_name:
                    color_space = value
            logger.info(f"colorspace lookup for: {search_name}. list: {color_space}")
            self.previewImage("OpenImageIO Output Image", pixels, width, height, channels, color_space)

        except Exception as e:
            logger.error(f"Error copying buffer to pixels: {e}")
            return False

        out = oiio.ImageOutput.create(filename)
        if not out:
            logger.error("Failed to create OIIO ImageOutput.")
            return False

        try:
            spec = oiio.ImageSpec(width, height, channels, oiio_format)
            out.open(filename, spec)
            out.write_image(pixels)
            logger.info(f"Image saved to {filename} (w={width}, h={height}, c={channels}, type={mx_basetype})")
            out.close()
            return True
        except Exception as e:
            logger.error(f"Failed to write image: {e}")
            try:
                out.close()
            finally:
                pass
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
        logger.debug(f"MaterialX type mapping: {mx_basetype} to {return_val}")
        return return_val

    def _materialx_type_to_np_type(self, mx_basetype):
        """Map MaterialX base type to NumPy dtype with explicit widths."""
        type_mapping = {
            mx_render.BaseType.UINT8:  np.uint8,
            mx_render.BaseType.UINT16: np.uint16,
            mx_render.BaseType.INT8:   np.int8,
            mx_render.BaseType.INT16:  np.int16,
            mx_render.BaseType.HALF:   np.float16,
            mx_render.BaseType.FLOAT:  np.float32,
        }
        return type_mapping.get(mx_basetype, None)


def test_load_save():
    """
    Example usage of the OiioImageLoader class with MaterialX ImageHandler.
    """
    parser = argparse.ArgumentParser(description="MaterialX OIIO Image Handler")
    parser.add_argument("path", help="Path to the image file")
    parser.add_argument("--flip", action="store_true", help="Flip the image vertically")
    parser.add_argument("--preview", action="store_true", help="Preview the image before saving")
    args = parser.parse_args()

    test_image_path = args.path
    if not os.path.exists(test_image_path):
        logger.error(f"Image file not found: {test_image_path}")
        return      

    # Create MaterialX handler with custom OIIO image loader
    loader = OiioImageLoader()
    loader.set_preview(args.preview)
    handler = mx_render.ImageHandler.create(loader)
    logger.info(f"Created image handler with loader ({loader.get_identifier()}): {handler is not None}")
    handler.addLoader(loader)

    mx_filepath = mx.FilePath(test_image_path)

    # Load image using handler API
    logger.info('-'*45)
    logger.info(f"Loading image from path: {mx_filepath.asString()}")
    mx_image = handler.acquireImage(mx_filepath)
    if mx_image:
        # Q: How to check for failed image load as you
        # get back a 1x1 pixel image.
        if mx_image.getWidth() == 1 and mx_image.getHeight() == 1:
            logger.warning("Failed to load image. Got 1x1 pixel image returned")
            return
        logger.info(f"MaterialX Image loaded via Image Handler:")
        logger.info(f"  Dimensions: {mx_image.getWidth()}x{mx_image.getHeight()}")
        logger.info(f"  Channels: {mx_image.getChannelCount()}")
        logger.info(f"  Base type: {mx_image.getBaseType()}")

        # Save image using handler API (to a new file)
        logger.info('-'*45)

        # Retrieve cached image
        mx_image = handler.acquireImage(mx_filepath)
        if mx_image:
            out_path = mx.FilePath("saved_" + os.path.basename(test_image_path))
            if handler.saveImage(out_path, mx_image, verticalFlip=args.flip):
                logger.info(f"MaterialX Image saved to {out_path.asString()}")
            else:
                logger.error("Failed to save image.")
        else:
            logger.error("Failed to acquire image for saving.")
    else:
        logger.error("Failed to load image.")

if __name__ == "__main__":    
    test_load_save()
