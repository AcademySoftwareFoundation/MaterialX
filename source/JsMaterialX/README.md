# Javascript Support

A Javascript package is created from the following modules.

- [JsMaterialXCore](JsMaterialXCore): Contains all of the core classes and util functions.
- [JsMaterialXFormat](JsMaterialXFormat): Contains the `readFromXmlString` function to read a MaterialX string.

## Generating JS/WASM

### Prerequisites

Make sure to clone the [emsdk repository](https://github.com/emscripten-core/emsdk), install and enable the latest emsdk environment.
```sh
# Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
cd emsdk

# Download and install the latest SDK tools.
./emsdk install latest

# Make the "latest" SDK "active" for the current user. (writes ~/.emscripten file)
./emsdk activate latest
```

For more information follow the steps described in the [emscripten documentation](https://emscripten.org/docs/getting_started/downloads.html).
The Emscripten toolchain is documented [here](https://emscripten.org/docs/building_from_source/toolchain_what_is_needed.html).

### Build
In the root of directory of this repository run the following:

#### CMake
The JavasScript library can be built using cmake and make.

1. Create the `build` folder from in the *root* of the repository.
```sh
mkdir -p ./build
cd ./build
```

2. Run cmake and make
When building the JavaScript output the user can specify the emsdk path with the `MATERIALX_EMSDK_PATH` option.
This option can be omitted if the `emsdk/emsdk_env.sh` script was run before hand.

```sh
# This will generate the release library
cmake .. -DMATERIALX_BUILD_JS=ON -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_EMSDK_PATH=/mnt/c/GitHub/PUBLIC/emsdk
cmake --build .
```

For Windows use [Ninja](https://ninja-build.org/) as the cmake generator.
```sh
cmake .. -DMATERIALX_BUILD_JS=ON -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_EMSDK_PATH=C:\GitHub\PUBLIC\emsdk -G "Ninja"
cmake --build .
```


#### Docker
It is also possible to build the project with [docker](https://docs.docker.com/) here are the required steps:

1. Get the emscripten docker image
```sh
docker run -dit --name emscripten -v {path_to_MaterialX}:/src trzeci/emscripten:1.39.7-upstream bash
```

2. Build the JavaScript bindings.
```sh
docker exec -it emscripten sh -c "cd build && cmake .. -DMATERIALX_BUILD_JS=ON -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_EMSDK_PATH=/emsdk_portable/ && cmake --build . --target install"
```

### Output
After building the project the `JsMaterialX.wasm` and `JsMaterialX.js` files can be found in `./_build/source/JsMaterialX/`.

### Testing
The JavaScript tests are located in `./test` folder and are defined with the `.spec.js` suffix.
Most of these tests were copied over from the Python [main.py tests](../../python/MaterialXTest/main.py).

#### Setup
These tests require node.js. This is a part of the emscripten environment. So make sure to call `emsdk_env` before running the steps described below.

1. Install the npm packages.
```sh
cd ./test && npm install
```

2. Run the tests
```sh
npm run test
```


