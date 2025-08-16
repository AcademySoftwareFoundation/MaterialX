# MaterialX Utilities Collection

This package `mtlxutils` contains a collection of the utilities explained and used for Python tutorials that can be found [here](https://kwokcb.github.io/MaterialX_Learn/documents/jupyter_example.html)

The minimum MaterialX package version is `1.38.8`.

## "Core" Utilities

* `mxbase` : Utility for minimum version checking.
* `mxfile` : File utilities including: creating "working" documents, loading in default libraries, library write filter predicate, and write to file / string wrappers. Used by various notebooks. Logic described in the [Basics notebook](https://github.com/kwokcb/MaterialX_Learn/tree/main/pymaterialx/mtlx_basics_notebook.ipynb)
* `mxnodegraph` : Node and nodegraph creation as explained in the [Nodegraph notebook](https://github.com/kwokcb/MaterialX_Learn/tree/main/pymaterialx/mtlx_graphs_notebook.ipynb)
* `mxtraversal` : Nodegraph introspection and sample usage to create `Mermaid` graphs as explained in the [Connectivity Notebook](https://github.com/kwokcb/MaterialX_Learn/tree/main/pymaterialx/mtlx_connectivity_notebook.ipynb) 
* `mxshadergen` : Utilities for shader generation as explained in the [Shader Generation Notebook](https://github.com/kwokcb/MaterialX_Learn/blob/main/pymaterialx/mtlx_shadergen_notebook.ipynb) 
* `mxrenderer` : Utilities for rendering with a default GLSL renderer as covered in the [Rendering Notebook](https://github.com/kwokcb/MaterialX_Learn/blob/main/pymaterialx/mtlx_render_notebook.ipynb) 


## "Supplemental" Utilities

* `mxusd` : MaterialX / Usd graphing mapping utilities as explained in the [MaterialX / Usd notebook](https://github.com/kwokcb/MaterialX_Learn/tree/main/pymaterialx/mtlx_usd_notebook.ipynb). Note that 
the Usd / MaterialX conversion logic is not included here as `UsdMtlx` from the the official Usd release should be used.


