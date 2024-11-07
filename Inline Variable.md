A way to have static or global objects inside header files without having to provide a definition in one of the translation units [(2)](https://youtu.be/e2ZQyYr0Oi0?t=22).
An inline variable is a non-member variable that can be defined in multiple translation units [(1)](https://youtu.be/fI2xiUqqH3Q?t=2125).
This means that we can put the definition, i.e. the value, in a header file.
This makes it possible to make more libraries header-only [(2)](https://youtu.be/e2ZQyYr0Oi0?t=47).
The definition must be the same in all translation units.
Putting it in a single header file is a goo way to ensure all definitions are the same.
Cannot have just an `extern` reference to an inline variable, a translational unit that accesses an inline variable must contain the variable's definition.
Non-static (i.e. with external linkage) inline variables have the same address in every translational unit.

Example:
```cpp
// Global, i.e. file sclope, variable.
// This can be shared by multiple translational unit and
// they will all see the same value.
inline std::atomic<bool> ready = false;

// Static member variable.
struct System
{
	inline static st::atomic<bool> ready = false;
};
```

[[constexpr]] data members, i.e. member variables, are implicitly inline.

Before inline variables we were required to put a definition, i.e. provide a place to live, for each variable.
With inline variables this is handled for us.
# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
- 2: [_C++17 - The Best Features - Nicolai Josuttis ACCU 2018_ by Nicolai Josuttis, ACCU Conference @ youtube.com 2018](https://youtu.be/e2ZQyYr0Oi0)
- 3: [_Core C++ 2021 :: C++17 key features_ by Alex Dathskovsky, CoreCppIL @ youtube.com 2021](https://www.youtube.com/watch?v=3gGhP0C-xOY)
