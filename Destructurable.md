There are two types of destructurable objects:
- `struct`-like.
- `get`-accessible.

# `struct`-Like

A user-defined type is destructurable if all non-static data member [(1)](https://youtu.be/fI2xiUqqH3Q?t=227):
- are public.
- are in the same, possibly parent, type, i.e. may be be spread across an inheritance hierarchy.
- are not an anonymous union.

# `get`-Accessible

A type that is not `struct`-like we can made destructurable by providing a templated `get` function.
Either as a member function or as an [[Argument Dependendent Lookup]] reachable free function.
We also need to provide specialization of `std::tuple_size`  and `std::tuple_element`.

# Standard Library

The [[Standard Library]] provides the following destructurable types [(1)](https://youtu.be/fI2xiUqqH3Q?t=268):
- `std::array`
- `std::tuple`
- `std::pair`



# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q?t=227)
