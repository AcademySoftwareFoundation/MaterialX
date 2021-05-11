#ifndef JSMATERIALX_HELPERS_H
#define JSMATERIALX_HELPERS_H

#include <vector>

template <class myClass>
std::vector<myClass> arrayToVec(myClass *arr, int size)
{
    std::vector<myClass> dest(arr, arr + size);
    return dest;
}


// Binding helpers

/**
 * Use this macro to conveniently create bindings for class member functions with optional parameters.
 * @param JSNAME The name of the function in JavaScript, as a double-quoted string (e.g. "addNodeGraph").
 * @param CLASSNAME The name (and scope) of the class that the member functions belongs to (e.g. mx::Document).
 * @param FUNCNAME The name of the function to bind (e.g. addNodeGraph).
 * @param MINARGS The minimal number of parameters that need to provided when calling the function (a.k.a # of required parameters).
 * @param MAXARGS The total number of parameters that the function takes, including optional ones.
 * @param ... The types of all parameters, as a comma-separated list (e.g. const std::string&, float, bool)
 */
#define BIND_FUNC(JSNAME, CLASSNAME, FUNCNAME, MINARGS, MAXARGS, ...) \
BIND_FUNC_ ##MINARGS ## _ ##MAXARGS(JSNAME, CLASSNAME, FUNCNAME, __VA_ARGS__)

#define BIND_FUNC_8(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2, TYPE3 p3, TYPE4 p4, TYPE5 p5, TYPE6 p6, TYPE7 p7, TYPE8 p8) { \
    return self.FUNCNAME(p1, p2, p3, p4, p5, p6, p7, p8); \
}))

// 9 Macros for MAXARGS = 8
#define BIND_FUNC_8_8(...) \
BIND_FUNC_8(__VA_ARGS__)

#define BIND_FUNC_7_8(...) \
BIND_FUNC_8_8(__VA_ARGS__) \
BIND_FUNC_7(__VA_ARGS__)

#define BIND_FUNC_6_8(...) \
BIND_FUNC_7_8(__VA_ARGS__) \
BIND_FUNC_6(__VA_ARGS__)

#define BIND_FUNC_5_8(...) \
BIND_FUNC_6_8(__VA_ARGS__) \
BIND_FUNC_5(__VA_ARGS__)

#define BIND_FUNC_4_8(...) \
BIND_FUNC_5_8(__VA_ARGS__) \
BIND_FUNC_4(__VA_ARGS__)

#define BIND_FUNC_3_8(...) \
BIND_FUNC_4_8(__VA_ARGS__) \
BIND_FUNC_3(__VA_ARGS__)

#define BIND_FUNC_2_8(...) \
BIND_FUNC_3_8(__VA_ARGS__) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_8(...) \
BIND_FUNC_2_8(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_8(...) \
BIND_FUNC_1_8(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_7(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2, TYPE3 p3, TYPE4 p4, TYPE5 p5, TYPE6 p6, TYPE7 p7) { \
    return self.FUNCNAME(p1, p2, p3, p4, p5, p6, p7); \
}))

// 8 Macros for MAXARGS = 7
#define BIND_FUNC_7_7(...) \
BIND_FUNC_7(__VA_ARGS__)

#define BIND_FUNC_6_7(...) \
BIND_FUNC_7_7(__VA_ARGS__) \
BIND_FUNC_6(__VA_ARGS__)

#define BIND_FUNC_5_7(...) \
BIND_FUNC_6_7(__VA_ARGS__) \
BIND_FUNC_5(__VA_ARGS__)

#define BIND_FUNC_4_7(...) \
BIND_FUNC_5_7(__VA_ARGS__) \
BIND_FUNC_4(__VA_ARGS__)

#define BIND_FUNC_3_7(...) \
BIND_FUNC_4_7(__VA_ARGS__) \
BIND_FUNC_3(__VA_ARGS__)

#define BIND_FUNC_2_7(...) \
BIND_FUNC_3_7(__VA_ARGS__) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_7(...) \
BIND_FUNC_2_7(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_7(...) \
BIND_FUNC_1_7(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_6(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2, TYPE3 p3, TYPE4 p4, TYPE5 p5, TYPE6 p6) { \
    return self.FUNCNAME(p1, p2, p3, p4, p5, p6); \
}))

// 7 Macros for MAXARGS = 6
#define BIND_FUNC_6_6(...) \
BIND_FUNC_6(__VA_ARGS__)

#define BIND_FUNC_5_6(...) \
BIND_FUNC_6_6(__VA_ARGS__) \
BIND_FUNC_5(__VA_ARGS__)

#define BIND_FUNC_4_6(...) \
BIND_FUNC_5_6(__VA_ARGS__) \
BIND_FUNC_4(__VA_ARGS__)

#define BIND_FUNC_3_6(...) \
BIND_FUNC_4_6(__VA_ARGS__) \
BIND_FUNC_3(__VA_ARGS__)

#define BIND_FUNC_2_6(...) \
BIND_FUNC_3_6(__VA_ARGS__) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_6(...) \
BIND_FUNC_2_6(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_6(...) \
BIND_FUNC_1_6(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_5(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2, TYPE3 p3, TYPE4 p4, TYPE5 p5) { \
    return self.FUNCNAME(p1, p2, p3, p4, p5); \
}))

// 6 Macros for MAXARGS = 5
#define BIND_FUNC_5_5(...) \
BIND_FUNC_5(__VA_ARGS__)

#define BIND_FUNC_4_5(...) \
BIND_FUNC_5_5(__VA_ARGS__) \
BIND_FUNC_4(__VA_ARGS__)

#define BIND_FUNC_3_5(...) \
BIND_FUNC_4_5(__VA_ARGS__) \
BIND_FUNC_3(__VA_ARGS__)

#define BIND_FUNC_2_5(...) \
BIND_FUNC_3_5(__VA_ARGS__) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_5(...) \
BIND_FUNC_2_5(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_5(...) \
BIND_FUNC_1_5(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_4(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, TYPE3, TYPE4, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2, TYPE3 p3, TYPE4 p4) { \
    return self.FUNCNAME(p1, p2, p3, p4); \
}))

// 5 Macros for MAXARGS = 4
#define BIND_FUNC_4_4(...) \
BIND_FUNC_4(__VA_ARGS__)

#define BIND_FUNC_3_4(...) \
BIND_FUNC_4_4(__VA_ARGS__) \
BIND_FUNC_3(__VA_ARGS__)

#define BIND_FUNC_2_4(...) \
BIND_FUNC_3_4(__VA_ARGS__) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_4(...) \
BIND_FUNC_2_4(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_4(...) \
BIND_FUNC_1_4(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_3(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, TYPE3, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2, TYPE3 p3) { \
    return self.FUNCNAME(p1, p2, p3); \
}))

// 4 Macros for MAXARGS = 3
#define BIND_FUNC_3_3(...) \
BIND_FUNC_3(__VA_ARGS__)

#define BIND_FUNC_2_3(...) \
BIND_FUNC_3_3(__VA_ARGS__) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_3(...) \
BIND_FUNC_2_3(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_3(...) \
BIND_FUNC_1_3(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_2(JSNAME, CLASSNAME, FUNCNAME, TYPE1, TYPE2, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, \
  TYPE1 p1, TYPE2 p2) { \
    return self.FUNCNAME(p1, p2); \
}))

// 3 Macros for MAXARGS = 2
#define BIND_FUNC_2_2(...) \
BIND_FUNC_2(__VA_ARGS__)

#define BIND_FUNC_1_2(...) \
BIND_FUNC_2_2(__VA_ARGS__) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_2(...) \
BIND_FUNC_1_2(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_1(JSNAME, CLASSNAME, FUNCNAME, TYPE1, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self, TYPE1 p1) { \
    return self.FUNCNAME(p1); \
}))

// 2 Macros for MAXARGS = 1
#define BIND_FUNC_1_1(...) \
BIND_FUNC_1(__VA_ARGS__)

#define BIND_FUNC_0_1(...) \
BIND_FUNC_1_1(__VA_ARGS__) \
BIND_FUNC_0(__VA_ARGS__)


#define BIND_FUNC_0(JSNAME, CLASSNAME, FUNCNAME, ...) \
.function(JSNAME, ems::optional_override([](CLASSNAME &self) { \
    return self.FUNCNAME(); \
}))

// 1 Macros for MAXARGS = 0
#define BIND_FUNC_0_0(...) \
BIND_FUNC_0(__VA_ARGS__)

#endif // JSMATERIALX_HELPERS_H