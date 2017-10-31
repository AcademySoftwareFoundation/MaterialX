# MaterialX

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/materialx/MaterialX/blob/master/LICENSE.txt)
[![Travis Build Status](https://travis-ci.org/materialx/MaterialX.svg?branch=master)](https://travis-ci.org/materialx/MaterialX)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/13103i35tqr8mb81?svg=true)](https://ci.appveyor.com/project/jstone-lucasfilm/materialx)

MaterialX is an open standard for transfer of rich material and look-development content between applications and renderers.  Originated at Lucasfilm in 2012, MaterialX has been used by Industrial Light & Magic (ILM) in feature films such as _Star Wars: The Force Awakens_ and real-time experiences such as _Trials on Tatooine_, and it remains the central material format for new ILM productions.

### Quick Start for Developers

- Download the latest version of the [CMake](https://cmake.org/) build system.
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.

### Supported Platforms

The MaterialX codebase requires a compiler with support for C++11, and can be built with any of the following:

- Microsoft Visual Studio 2015 or newer
- GCC 4.8 or newer
- Clang 3.3 or newer

The Python bindings for MaterialX are based on [PyBind11](https://github.com/pybind/pybind11), and currently support Python versions 2.6, 2.7, and 3.x.

### Repository

The MaterialX repository consists of the following folders:

    documents - The MaterialX specification, developer guide, and example files.
    source    - A cross-platform C++ library for MaterialX with Python bindings.
                The MaterialXCore module supports the core MaterialX elements
                and graph traversal, while the MaterialXFormat module supports
                XML serialization.
    python    - Support modules for MaterialX Python.

## ADSK Contribution Workflow

### Setup remote names
As setup the following remotes should be set up: (git remote -v)

- Public fork:
```
 public_fork  https://github.com/autodesk-forks/MaterialX.git (fetch)
 public_fork  https://github.com/autodesk-forks/MaterialX.git (push)
```

- Private fork
```
private_fork    https://git.autodesk.com/autodesk-forks/MaterialX (fetch)
private_fork    https://git.autodesk.com/autodesk-forks/MaterialX (push)
```

- ILM master
```
upstream        https://github.com/materialx/MaterialX (fetch)
upstream        /// ATTENTION ///   Are you sure you want to push to public github.com? To override this warning run: 'git adsk enable-public-push github.com/materialx/MaterialX (push)
```

Remotes can be added using this syntax:
```
git remote add <desired-name> <remote-location.git>
git fetch fetch
```
### When working on a ADSK contribution make sure to update your branch to the latest ILM master
```
git pull upstream master
```

Note that master on both the private and public fork cannot be modified so the pull should be to a local branch.
If doing your work in the private fork, and it makes sense update adsk/master and branch from there.
Otherwise create a new branch in the public fork.

### Synchonrizing code between public and private forks
If working in a private branch then the changes can be cherry-picked to the pubiic branch and vice versa:
```
git cherry-pick <commit-hash>
```

### Code reviews
All code will need to be reviewed by ILM so all changes must eventually be put into a public fork branch.
It may be best to just pass the diff to ILM first before creaing a pull request. Once a pull request is done,
then ILM's CI system will be invoked. Comment the review as necessary and naturally fix an errors found during CI.

### Testing
There are yaml files which define the build process. Make sure to run those build steps instead of / or just
building and testing locally. See 
```
.appyveyor.yml 
```
for Windows, and 
```
.travis.yml 
```
for Linux and Mac. 

The CI build will run build and test on all the platforms specified.

### Coding standards
Make sure to follow the Pixar/ILM coding standards.
