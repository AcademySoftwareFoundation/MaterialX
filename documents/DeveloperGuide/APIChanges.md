## API Signature Changes and Deprecation

Any changes to the C++, Python or Javascript API interfaces can only be made during a minor revision update. In the examples below if the desire is to just remove an interface then it should just be marked as deprecated as it will not map to any new method.

Deprecated methods can be removed in the next minor revision update (?)

The following examples show class method signature changes for C++, Python and Javascript APIs. The examples show how to deprecate a method and add a new method with the desired signature. Non-class function changes can be handled in a similar way. 

### C++ API

For C++, the user should change the appropriate header file by adding in a new function or change the signature of an existing function. The existing function is added a deprecated function which calls into the new function if possible.

**Example 1**: New and deprecated functions have the same name
```cpp
class MyClass
{
  public:
    /// New method with same name as deprecated method
    void myMethod(int a, int b);

    // Deprecated method -- without documentation information 
    [[deprecated("This method is deprecated, use myMethod(a,b) instead")]]
    void myMethod(int a, int b, int c);
};
```
with the implementation looking something like this:
```cpp
void MyClass::myMethod(int a, int b)
{
    // Implementation
}
void MyClass::myMethod(int a, int b, int c)
{
    // Call into new method if possible
    myMethod(a, b);
}
```

**Example 2**: New and deprecated functions have different names
```cpp
class MyClass
{
  public:
    /// New method with different name than deprecated method
    void myNewMethod(int a, int b);

    // Deprecated method -- without documentation information 
    [[deprecated("This method is deprecated, use myNewMethod(a,b) instead")]]
    void myMethod(int a, int b, int c);
};
```
with the implementation looking something like this:
```cpp
void MyClass::myNewMethod(int a, int b)
{
    // Implementation
}
void MyClass::myMethod(int a, int b, int c)
{
    // Call into new method if possible
    myNewMethod(a, b);
}
```


### Python API

For Python, the user should change the appropriate Python file by adding in a new function or change the signature of an existing function.

- If the new function has a different name then the existing function then the deprecated function can be added into the `main.py` file in
`python/MaterialX`. The deprecated function will call into the new function if possible.
- If they have the same name then the deprecated function should be added via the `pybind11` wrapper code along with the new wrapper code for the C++ function

**Example 1**: New and deprecated functions have the same name

Update the appropriate `pybind11` wrapper class:
```cpp
class PyBindMyClass : public MyClass
{    
  public:
    // Add deprecated method to class
    void deprecatedMyMethod(int a, int b, int c) 
    {
        PyErr_WarnEx(PyExc_DeprecationWarning,
            "This method is deprecated, use myMethod(a, b) instead.", 1);        
        myMethod(a, b);
    }
};

void bindPyMyClass(py::module& mod)
{
    py::class_<MyClass, MyClassPtr>(mod, "MyClass")
        .def("myMethod", &MyClass::myMethod),
        .def("myMethod", ((mx::MyClass::*)(int a, int b, int c)) & PyBindMyClass::deprecatedMyMethod)        
}
```

**Example 2**: New and deprecated functions have different names

Update the appropriate `pybind11` wrapper class:
```cpp
void bindPyMyClass(py::module& mod)
{
    py::class_<MyClass, MyClassPtr>(mod, "MyClass")
        .def("myMethod", &MyClass::myNewMethod)
}
```

Update `python/MaterialX/main.py`:
```python

def _myMethod(self, a, b, c):
    warnings.warn("This function is deprecated use myNewMethod(a,b) instead.", DeprecationWarning, stacklevel = 2)
    return self.myNewMethod(a, b)

# Add deprecated function to class
MyClass.myMethod = _myMethod
```


### Javascript API

For Javascript, the user should change the appropriate Javascript file by adding in a new function or change the signature of an existing function.

The new function should be added to the appropriate file in the `javascript` directory. The deprecated function should be modified to call into the new function if possible.

There is no explicit support for deprecation in Javascript. The 
implementation can provide a warning message when the deprecated function is called.

**Example 1**: New and deprecated functions have the same name

```cpp
    // Add new method to class using appropriate utility wrappers
    BIND_MEMBER_FUNC("myMethod", mx::MyClass, myMethod, 2,2, int, int)

    // Provide an explicit call to the deprecated method
    .function("myMethod", ems::optional_override([](mx::MyClass& self, 
        int a, int b, int c)
        {
            const std::string message = "This method is deprecated, use myMethod(int, int) instead.";
            EM_ASM_({
                console.warn(UTF8ToString($0));
            }, message.c_str());
            return self.myMethod(a, b);
        }))
``` 

**Example 2**: New and deprecated functions have different names

```cpp
    // Add new method to class using appropriate utility wrappers
    BIND_MEMBER_FUNC("myNewMethod", mx::MyClass, myMethod, 2,2, int, int)

    // Provide an explicit call to the deprecated method
    .function("myMethod", ems::optional_override([](mx::MyClass& self, 
        int a, int b, int c)
        {
            const std::string message = "This method is deprecated, use myMethod(int, int) instead.";
            EM_ASM_({
                console.warn(UTF8ToString($0));
            }, message.c_str());
            return self.myNewMethod(a, b);
        }))
``` 

