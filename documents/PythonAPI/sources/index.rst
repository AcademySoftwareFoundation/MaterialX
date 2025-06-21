MaterialX Python API Documentation
==================================

The `MaterialX Python API`_ provides Python bindings for the
`MaterialX C++ API <https://materialx.org/docs/api/>`_ .

MaterialX is an open standard for representing rich material and look-development
content in computer graphics, enabling its platform-independent description and
exchange across applications and renderers. Launched at `Industrial Light & Magic
<https://www.ilm.com/>`_ in 2012, MaterialX has been a key technology in their
feature films and real-time experiences since *Star Wars: The Force Awakens*
and *Millennium Falcon: Smugglers Run*. The project was released as open source
in 2017, with companies including Sony Pictures Imageworks, Pixar, Autodesk,
Adobe, and SideFX contributing to its ongoing development. In 2021, MaterialX
became the seventh hosted project of the `Academy Software Foundation
<https://www.aswf.io/>`_ .

.. toctree::
    :maxdepth: 1

    MainPage.md
    GraphEditor.md
    Viewer.md
    ShaderGeneration.md


MaterialX Python API
--------------------

The MaterialX Python API consists of two parts:

- A set of `MaterialX Python Modules`_ that are implemented as Python C
  extensions that correspond to MaterialX C++ libraries.
- A Python package named `MaterialX` that wraps the MaterialX Python modules to
  provide a more pythonic interface, in particular for working with the
  `Element Classes <generated/PyMaterialXCore.html#element-classes>`_ and
  `Value Classes <generated/PyMaterialXCore.html#value-classes>`_ of
  `PyMaterialXCore`.

The `MaterialX` Python package is typically imported aliased as `mx`:

.. code:: python

   import MaterialX as mx

All functions and classes from the `PyMaterialXCore` and `PyMaterialXFormat`
modules are available in the top-level `MaterialX` namespace, and, by aliasing,
the `mx` namespace.

For example, the `PyMaterialXCore.Matrix44` class is typically used as
`mx.Matrix44`:

.. code:: python

    >>> import MaterialX as mx
    >>> mx.Matrix44
    <class 'MaterialX.PyMaterialXCore.Matrix44'>

You can use the *Quick search* box in the sidebar on the left to quickly find
documentation for a particular module, function, class, method, or attribute of
interest, for example `getUdimCoordinates <search.html?q=getUdimCoordinates>`_
or `MeshPartition <search.html?q=MeshPartition>`_.


MaterialX Python Modules
------------------------

.. autosummary::
    :toctree: generated
    :caption: MaterialX Python Modules:

    PyMaterialXCore
    PyMaterialXFormat
    PyMaterialXGenShader
    PyMaterialXGenGlsl
    PyMaterialXGenOsl
    PyMaterialXGenMdl
    PyMaterialXGenMsl
    PyMaterialXRender
    PyMaterialXRenderGlsl
    PyMaterialXRenderOsl
    PyMaterialXRenderMsl

.. toctree::

    genindex
