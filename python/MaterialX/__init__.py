# Python 3.8+ on Windows: DLL search paths for dependent
# shared libraries
# Refs.:
# - https://github.com/python/cpython/issues/80266
# - https://docs.python.org/3.8/library/os.html#os.add_dll_directory
import os
import sys
if sys.platform == "win32" and sys.version_info >= (3, 8):
    import importlib.metadata
    try:
        importlib.metadata.version('MaterialX')
    except importlib.metadata.PackageNotFoundError:
        # On a non-pip installation, this file is in %INSTALLDIR%\python\MaterialX
        # We need to add %INSTALLDIR%\bin to the DLL path.
        mxdir = os.path.dirname(__file__)
        pydir = os.path.split(mxdir)[0]
        installdir = os.path.split(pydir)[0]
        bindir = os.path.join(installdir, "bin")
        if os.path.exists(bindir):
            os.add_dll_directory(bindir)

from .main import *
from .colorspace import *

try:
    from .legacy import *
except ImportError:
    pass

__version__ = getVersionString()
