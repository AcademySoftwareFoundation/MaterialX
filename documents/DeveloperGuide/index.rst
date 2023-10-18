MaterialX Python API Documentation
==================================

The MaterialX Python API provides Python bindings for the
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

General information about MaterialX, announcements, events, third-party support,
and contributing to MaterialX is available at
`MaterialX.org <https://materialx.org/>`_ .

.. toctree::
    :maxdepth: 1

    MainPage.md
    GraphEditor.md
    Viewer.md
    ShaderGeneration.md


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


Import Order Dependencies
-------------------------

Note that the order in which the MaterialX Python modules are imported matters, as there are dependencies between them:

- :py:mod:`PyMaterialXCore` -- typically always needed
    - :py:mod:`PyMaterialXFormat` -- needed for file I/O
    - :py:mod:`PyMaterialXRender` -- needed for core rendering
        - :py:mod:`PyMaterialXRenderGlsl` -- render using OpenGL Shading Language
        - :py:mod:`PyMaterialXRenderMsl` -- render using Metal Shading Language
        - :py:mod:`PyMaterialXRenderOsl` -- render using Open Shading Language
- :py:mod:`PyMaterialXGenShader` -- needed for core shader generation
    - :py:mod:`PyMaterialXGenGlsl` -- generating shaders using OpenGL Shading Language
    - :py:mod:`PyMaterialXGenMdl` -- generating shaders using Material Definition Language
    - :py:mod:`PyMaterialXGenMsl` -- generating shaders using Metal Shading Language
    - :py:mod:`PyMaterialXGenOsl` -- generating shaders using Open Shading Language

That is to say, for example:

- :py:mod:`PyMaterialXCore` needs to be imported before :py:mod:`PyMaterialXFormat` and :py:mod:`PyMaterialXRender`
- :py:mod:`PyMaterialXGenShader` needs to be imported before :py:mod:`PyMaterialXGenMsl`
