One can have template parameters that are values instead of types.
For example, `std::array` uses a type template parameter of type `size_t` for the size of the array.
If the type of that non-type template parameter is also a template parameter then we introduce some code duplication.
Or at least redundant code.
With auto non-type template parameters we can use `auto` in the template instead of specifying the type explicitly and the type will be deduced from the usage code [(2)](https://youtu.be/3gGhP0C-xOY?t=1048).

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

# Compile Time List

We can create a type whose purpose is to hold an ordered list of values at compile time [(2)](https://youtu.be/3gGhP0C-xOY?t=1048).
Makes use of [[Variadic Templates]].
Can be either heterogeneous, i.e. potentially containing values of different types, or homogeneous, i.e. all values in the list having the same type.
```cpp
template <auto... values>
struct HeterogeneousList
{
}

using MyList1 = HeterogeneousList<42, 'X', 13u>;

template <auto v0, decltyp(v0)... vs>
struct HomogeneousList
{
}

using MyList2 = HomegeneousList<1, 2, 3>;
```

I don't know how to access the values.

# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
- 2: [_Core C++ 2021 :: C++17 key features_ by Alex Dathskovsky, CoreCppIL @ youtube.com 2021](https://www.youtube.com/watch?v=3gGhP0C-xOY)
