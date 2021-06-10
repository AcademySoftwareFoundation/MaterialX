## MaterialX JavaScript Bindings

## JavaScript Support

A JavaScript package is created from the following modules.

- [JsMaterialXCore](JsMaterialXCore): Contains all of the core classes and util functions.
- [JsMaterialXFormat](JsMaterialXFormat): Contains the `readFromXmlString` function to read a MaterialX string.

## Generating the Bindings

### Prerequisites

The emscripten SDK is required to generate the JavaScript bindings. On Linux and MacOS, the SDK can be installed directly, following the instructions below. On Windows, we recommend to use a Docker image instead (See instructions below. Using the SDK directly might still work, but hasn't been tested). Alternatively, WSL2 can be used on Windows (with an Ubuntu image). In this case, the same steps apply on all platforms.

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

For more information, follow the steps described in the [emscripten documentation](https://emscripten.org/docs/getting_started/downloads.html).
The emscripten toolchain is documented [here](https://emscripten.org/docs/building_from_source/toolchain_what_is_needed.html).

### Build
In the root of directory of this repository run the following:

#### Docker (recommended on Windows machines, not required otherwise)
It is recommended to build the project with [docker](https://docs.docker.com/) here are the required steps:

  1. For Windows make sure to use Linux containers and that File Sharing is set up to allow local directories on Windows to be shared with Linux containers. 
  
     For example, if the path to MaterialX is ```"c:\git\MaterialXrepo"``` then the ```"c"``` drive should be set. (See https://docs.docker.com/docker-for-windows/#file-sharing for more details)

  2. Get the `emscripten` docker image
     ```sh
     docker run -dit --name emscripten -v {path_to_MaterialX}:/src trzeci/emscripten:1.39.7-upstream bash
     ```

  3. Build the JavaScript bindings.
     ```sh
     docker exec -it emscripten sh -c "cd build && cmake .. -DMATERIALX_BUILD_JS=ON -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_EMSDK_PATH=/emsdk_portable/ && cmake --build . --target install"
     ```

#### CMake
The JavaScript library can be built using cmake and make.

1. Create a build folder from in the *root* of the repository.
```sh
mkdir -p ./build
cd ./build
```

2. Run cmake and make
When building the JavaScript output the user can specify the emsdk path with the `MATERIALX_EMSDK_PATH` option.
This option can be omitted if the `emsdk/emsdk_env.sh` script was run beforehand.

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


### Output
After building the project the `JsMaterialX.wasm` and `JsMaterialX.js` files can be found in the global install directory of this project.

### Install
To install the results into the test directory run
```sh
cmake --build --target install
```
from the build directory.

## Testing
The JavaScript tests are located in the `test` folder and are defined with the `.spec.js` suffix.

#### Setup
These tests require `node.js`, which is shipped with the emscripten environment. Make sure to `source` the `emsdk/emsdk_env.sh` script before running the steps described below.

1. From the test directory, install the npm packages.
```sh
npm install
```

2. Run the tests from the test directory.
```sh
npm run test
```

## Using the Bindings
### Consuming the Module
<!-- TODO: Change to official export name -->
The JavaScript bindings can be consumed via a script tag in the browser:
```html
<script src="./JsMaterialX.js" type="text/javascript"></script>
<script type="text/javascript">
    Module().then((mx) => {
        mx.createDocument();
        ...
    });
</script>
```

In NodeJs, simply `require` the module like this:
```javascript
const Module = require('./JsMaterialX.js');

Module().then(mx => {
    mx.createDocument();
    ...
});
```

### JavaScript API
In general, the JavaScript API is the same as the C++ API. Sometimes, it doesn't make sense to bind certain functions or even classes, for example when there is already an alternative in native JS. Other special cases are explained in the following sections.

#### Data Type Conversions
Data types are automatically converted from C++ types to the corresponding JavaScript types, to provide a more natural interface on the JavaScript side. For example, a string that is returned from a C++ function as an `std::string` won't have the C++ string interface, but will be a JavaScript string instead. While this is usually straight-forward, it has some implications when it comes to containers.

C++ vectors will be converted to JS arrays (and vice versa). Other C++ containers/collections are converted to either JS arrays or objects as well. While this provides a more natural interface on the JS side, it comes with the side-effect that modifications to such containers on the JS side will not be reflected in C++. For example, pushing an element to a JS array that was returned in place of a C++ vector will not update the vector in C++. Elements within the array can be modified and their updates will be reflected in C++, though (this does only apply to class types, not primitives or strings, of course).

#### Template Functions
Functions that handle generic types in C++ via templates are mapped to JavaScript by creating multiple bindings, one for each type. For example, the `InterfaceElement::setInputValue` function is mapped as `setInputValueString`, `setInputValueBoolean`, `setInputValueInteger` and so on.

#### Value Getters
Some functions suggest to return values (e.g. `ValueElement::getValue`), but return `Value` pointers instead. Getting the actual value requires to call `getData` on that pointer, given that the pointer is valid. In order to align with the Python bindings and simplify the consumption of values in JavaScript, `getValue` and related functions return the value directly, or `null` if no value is set. In case that the underlying `Value` pointer is of interest, the original behaviour is available through a function that is prefixed with `_`, e.g. `_getValue`.

#### Iterators
MaterialX comes with a number of iterators (e.g. `TreeIterator`, `GraphIterator`). These iterators implement the iterable (and iterator) protocol in JS, and can therefore be used in `for ... of` loops.

#### Exception Handling
When a C++ function throws an exception, this exception will also be thrown by the corresponding JS function. However, you will only get a pointer (i.e. a number in JS) to the C++ exception object in a `try ... catch ...` block, due to some exception handling limitations of emscripten. The helper method `getExceptionMessage` can be used to extract the exception message from that pointer:

```javascript
const doc = mx.createDocument();
doc.addNode('category', 'node1');

try {
    doc.addNode('category', 'node1');
} catch (errPtr) {
    // typeof errPtr === 'number' yields 'true'
    console.log(mx.getExceptionMessage(errPtr)); // Prints 'Child name is not unique: node1'
}
```

## Maintaining the Bindings
This section provides some background on binding creation for contributors. In general, we recommed to look at existing bindings for examples.

### What to Bind?
In general, we aim for 100% coverage of the MaterialX API, at least for the Core and Format packages. However, there are functions and even classes where creating bindings wouldn't make much sense. The `splitString` utility function is such an example, because the JavaScript string class does already have a `split` method. The `FilePath` and `FileSearchPath` classes of the Format package are simply represented as strings on the JavaScript side, even though they provide complex APIs in C++. This is because most of their APIs do not apply to browsers, since they are specific to file system operations. In NodeJs, they would present a competing implementation of the core `fs` module, and therefore be redundant (even though they might be convenient in some cases).

The examples above illustrate that it does not always make sense to create bindings, if there is no easy way to map them to both browsers and NodeJs, or if there is already an alternative in native JS. The overhead, both in maintenance and bundle size, wouldn't pay off.

### Emscripten's optional_override
Emscripten's `optional_override` allows to provide custom binding implementations in-place and enables function overloading by parameter count, which is otherwise not supported in JavaScript. Contributors need to be careful when using it, though, since there is a small pitfall.

If a function binding has multiple overloads defined via `optional_override` to support optional parameters, this binding must only be defined once on the base class (i.e. the class that defines the function initially). Virtual functions that are overriden in deriving classes must not be bound again when creating bindings for these derived classes. Doing so can lead to the wrong function (i.e. base class' vs derived class' implementation) being called at runtime.

### Optional Parameters
Many C++ functions have optional parameters. Unfortunately, emscripten does not automatically deal with optional parameters. Binding these functions the 'normal' way will require users to provide all parameters in JavaScript, including optional ones. We provide helper macros to cicumvent this issue. Different flavors of the `BIND_*_FUNC` macros defined in `Helpers.h` can be used to conveniently bind functions with optional parameters. See uses of these macros in the existing bindings for examples.

NOTE: Since these macros use `optional_override` internally, the restrictions explained above go for them as well. Only define bindings for virtual functions once on the base class with these macros.

### Template Functions
Generic functions that deal with multiple types cannot be bound directly to JavaScript. Instead, we create multiple bindings, one per type. The binding name follows the pattern `functionName<Type>`. For convenience, we usually provide a custom macro that takes the type and constructs the corresponding binding. See the existing bindings for examples.

### Array <-> Vector conversion
As explained in the user documentation, types are automatically converted between C++ and JavaScript. While there are multiple examples for custom marshalling of types (e.g. `std::pair<int, int>` to array, or `FileSearchPath` to string), the most common use case is the conversion of C++ vectors to JS arrays, and vice versa. This conversion can automatically be achieved by including the `VectorHelper.h` header in each binding file that covers functions which either accept or return vectors in C++.

### Testing strategy
Testing every binding doesn't seem desirable, since most of them will directly map to the C++ implementation, which should already be tested in the C++ tests. Instead, we only test common workflows (e.g. iterating/parsing a document), bindings with custom implementations, and our custom binding mechanisms. The latter involves custom marshalling, e.g. of the vector <-> array conversion, or support for optional parameters. Additionally, all features that might behave different on the web, compared to desktop, should be tested as well.

<!--
TODO: Tests and other best practices
-->

## CI
Emscripten builds and test runs are specified in `.github/workflows/build_wasm.yml`.

