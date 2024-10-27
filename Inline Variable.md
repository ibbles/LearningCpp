An inline variable is a non-member variable that can be defined in multiple translation units [(1)](https://youtu.be/fI2xiUqqH3Q?t=2125).
This means that we can put the definition, i.e. the value, in a header file.
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
