One can have template parameters that are values instead of types.
For example, `std::array` uses a type template parameter of type `size_t` for the size of the array.
If the type of that non-type template parameter is also a template parameter then we introduce some code duplication.
Or at least redundant code.
With auto non-type template parameters we can use `auto` in the template instead of specifying the type explicitly and the type will be deduced from the usage code.

The following is an example of a class holding a compile time constant that uses a auto non-type template parameter [(1)](https://youtu.be/fI2xiUqqH3Q?t=1869).
```cpp
template <auto value>
struct Constant
{
	// Will this do the right thing, do do I need to
	// rename the non-type template parameter to
	// disabiguate 'value'?
	static constexpr T value {value};
};

using tiny = constant<0.001>; // tiny::value is double.
using small = constant<100>; // small::value is int.
using large= constant<100000l>; // large::value is long.
```


# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
