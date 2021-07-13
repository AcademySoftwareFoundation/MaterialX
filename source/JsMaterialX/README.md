# MaterialX JavaScript Bindings

This folder contains the JavaScript bindings for the MaterialX library. The bindings are generated using the emscripten SDK.

## Generating the Bindings

### Prerequisites

The emscripten SDK is required to generate the JavaScript bindings. There are several ways of using the SDK on different platforms. In general, we recommend to install the SDK directly, following the instructions below. Alternative options are explained [below](#alternative-options-to-use-the-emscripten-sdk).

To install the SDK directly, follow the instructions of the [emscripten SDK installation Guide](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended). Make sure to install prerequisites depending on your platform and read usage hints first (e.g. differences between Unix / Windows scripts).

Note that following the instructions will set some environment variables that are required to use the SDK. These variables are only set temporarily for the current terminal, though. Setting the environment variables in other terminals can be achieved by running
```sh
source ./emsdk_env.sh
```
inside of the `emsdk` folder (check the documentation for the Windows equivalent). In case of the MaterialX project, it is not required to have these environment variables set. You can also use a CMake build flag instead, as described in the [build instructions](#build-steps) below.

Setting the environment variables permanently is also possible, either by adding a `--permanent` flag to the `activate` command, or by sourcing the `emsdk_env` script every time a shell is launched, e.g. by adding the `source` call to `~/.bash_profile` or an equivalent file. Note however, that the environment variables set by the emscripten SDK might override existing system settings, like the default Python, Java or NodeJs version, so setting them permanently might not be desired on all systems.

### Alternative options to use the emscripten SDK
If installing the emscripten SDK directly isn't desired, e.g. because you prefer to keep development environments cleanly separated, it can be provided in different ways.

On Windows, using WSL2 (e.g. with an Ubuntu image) is a viable alternative to isolate the build environment from your main system. Simply set up the build environment in that Linux container.

Another alternative is to use a [Docker](https://docs.docker.com/) image. With Docker installed, use the [emscripten Docker image](https://hub.docker.com/r/emscripten/emsdk) as described in the [Docker build instructions](#docker) below.

### Build Steps
Run the following commands in the root folder of this repository.

Create a build folder in the *root* of the repository and navigate to that folder:
```sh
mkdir ./build
cd ./build
```

If you are using the emsdk directly on Windows, note that the emscripten SDK doesn't work with Microsoft's Visual Studio build tools. You need to use an alternative CMake generator like [MinGW](http://mingw-w64.org/doku.php) Makefiles or [Ninja](https://ninja-build.org/). We recommend to use Ninja (unless you already have MinGW installed), since it's pretty lightweight and a pure build system, instead of a full compiler suite.

Generate the build files with CMake. When building the JavaScript bindings, you can optionally specify the emsdk path with the `MATERIALX_EMSDK_PATH` option. This option can be omitted if the `emsdk/emsdk_env.sh` script was run beforehand.
```sh
cmake .. -DMATERIALX_BUILD_JS=ON -DMATERIALX_BUILD_RENDER=OFF -DMATERIALX_BUILD_TESTS=OFF -DMATERIALX_BUILD_GEN_GLSL=OFF -DMATERIALX_BUILD_GEN_OSL=OFF -DMATERIALX_BUILD_GEN_MDL=OFF -DMATERIALX_BUILD_GEN_OGSXML=OFF -DMATERIALX_BUILD_GEN_OGSFX=OFF -DMATERIALX_BUILD_GEN_ARNOLD=OFF -DMATERIALX_BUILD_VIEWER=OFF -DMATERIALX_EMSDK_PATH=/path/to/emsdk
```
On Windows, remember to set the CMake generator via the `-G` flag , e.g. `-G "Ninja"`.

To build the project, run
```sh
cmake --build . --target install -j8
```
Change the value of the `-j` option to the number of threads you want to build with.
#### Docker
In order to build using the Docker image, execute the following command in a terminal (in the root of this repository, after creating the build folder), using the CMake commands introduced above.
```sh
docker run --rm -v {path_to_MaterialX_repo}:/src emscripten/emsdk sh -c "cd build && <cmake generator command> && <cmake build command>"
```

For follow-up builds (i.e. after changing the source code), remove the `<cmake generator command>` step from the above call.

### Output
After building the project the `JsMaterialXCore.wasm`, `JsMaterialXCore.js`, `JsMaterialXGenShader.wasm`, `JsMaterialXGenShader.js` and `JsMaterialXGenShader.data` files can be found in the global install directory of this project.

## Testing
The JavaScript tests are located in the `test` folder and use the `.spec.js` suffix.

#### Setup
These tests require `node.js`, which is shipped with the emscripten environment. Make sure to `source` the `emsdk/emsdk_env.sh` script before running the steps described below, if you don't have NodeJs installed on your system already (running the command is not required otherwise).

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
<script src="./JsMaterialXCore.js" type="text/javascript"></script>
<script type="text/javascript">
    Module().then((mx) => {
        mx.createDocument();
        ...
    });
</script>
```

In NodeJs, simply `require` the module like this:
```javascript
const Module = require('./JsMaterialXCore.js');

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

#### Loading MaterialX files
The bindings expose the `readFromXmlString` and `readFromXmlFile` functions. Their usage is similar to C++ and should work in browsers and NodeJs. Note that these functions are asynchronous in JavaScript, so you need to either `await` them or place depending code in a `.then()` block.

By default, the functions will resolve referenced (i.e. included) documents. This can be disabled by setting the `readOptions.readXIncludes` to `false`:
```javascript
const readOptions = new mx.XmlReadOptions();
readOptions.readXIncludes = false;
await readFromXmlFile(doc, filename, searchPath, readOptions); // will only read the top-level file, no includes
```
Note that the `readXIncludesFunction` option that exists on the C++ read options is not supported in JavaScript.

The `searchPath` is a semicolon-separated list of absolute or relative paths. Relative paths will be evaluated with regards to the current working directory. In case of using absolute search paths in web browsers (i.e. urls), note that urls like `mydomain.com/path` or `localhost/path` might be considered relative paths. To ensure they're used as absolute paths, make them fully formed urls, i.e. have a protocol prefix like `https://`.

#### Writing MaterialX files
Documents can be written to strings via the `writeToXmlString` method, and to files with the `writeToXmlFile` method. In NodeJs, the latter will write the file to the path provided to the method (relative paths will be evaluated with respect to the current working directory). In the browser, the written file will be downloaded automatically, so only the file name matters.

Note that the `XmlWriteOptions.elementPredicate` option is not supported in JavaScript.

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

### Custom JavaScript Code
Some bindings cannot be direct mappings to a C++ function. In particular when operations are asynchronous in JavaScript (e.g. loading files), it's easier to provide custom JavaScript implementations for the affected functions. This is how `readFromXmlString` and `readFromXmlFile` are implemented, for example. Such JavaScript code can be provided using the post-JS feature of emscripten. There should be one `post.js` file per MaterialX module, if that module requires any custom JS code. Note that these files need to be added to `CMakeLists.txt` in the `JsMaterialX` source folder. We recommend to provide custom code that dependes on the WebAssembly module like this:
```javascript
onModuleReady(function () {
    <your code here>
});
```
This will register your code after the module has been initialized. The wasm module will be available as `Module`.
Since the module itself is ES5 code, we recommend to write custom code in ES5 as well, even though ES6 should work as well in most cases.

In order to avoid conflicting definitions in post.js files, we recommend to wrap custom code in an [IIFE](https://developer.mozilla.org/en-US/docs/Glossary/IIFE) (Immediately Invoked Function Expression).

### Testing strategy
Testing every binding doesn't seem desirable, since most of them will directly map to the C++ implementation, which should already be tested in the C++ tests. Instead, we only test common workflows (e.g. iterating/parsing a document), bindings with custom implementations, and our custom binding mechanisms. The latter involves custom marshalling, e.g. of the vector <-> array conversion, or support for optional parameters. Additionally, all features that might behave different on the web, compared to desktop, should be tested as well.

The C++ and [Python binding tests](../../python/MaterialXTest/main.py) follow a different approach than the JS unit tests, by testing larger workflows instead of single features. In order to cover at least as much functionality in JS as in Python, the tests have been ported to JS. However, JS tests are organized in the same file structure as the bindings, so these workflow tests have been added to the file where they fit in best (e.g. the `Traverse Graph` test is in `traversal.spec.js`). This is equivalent to how tests are organized in C++. 

## CI
Emscripten builds and test runs are specified in `.github/workflows/build_wasm.yml`.

