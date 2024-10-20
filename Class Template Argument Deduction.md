Allows the compiler to deduce the type of a class from its initialization parameters.
Is used in:
- Declarations.
- Function-style cast expressions.
- `new` expressions.

```cpp
// Without class template argument deduction.
// Must specify both the element type and number
// of elements of an std::array.
std::array<int, 5> data {1, 2, 3, 4, 5};

// With class template argument deduction.
// Element type and number of element is deduced.
std::array data {1, 2, 3, 4, 5};
```

Can be used to initialize `std::tuple` instances [(2)](https://youtu.be/fI2xiUqqH3Q?t=1563).
In the following the type of `t` is deduced to be `std::tuple<int, double>` from the types of the parameters passed to the constructor.
```cpp
std::tuple t(42, 3.14);
```

This only works if the constructor template type parameters match the class template type parameters.

The class template parameters must be fully deduced from the parameters, no template parameter may be explicitly provided in the type name.
The following is not allowed:
```cpp
std::tuple<int> t(0, 1);
```


# Simply Taking Locks

Class template argument deduction simplifies mutex locking since we don't need to explicitly tell `std::lock_guard` what type the mutex it is locking is[(2)](https://youtu.be/fI2xiUqqH3Q?t=1612).
The following can lock any mutex-type that has the required functions.
```cpp
std::lock_guard lock(mutex);
```


# Class Template Deduction Guides

A deduction guide is a mapping from a set of parameter types to a set of class template parameter types [(2)](https://youtu.be/fI2xiUqqH3Q?t=1648).
That is, it specifies that if the initialization parameters are of types `T0, T1, ...` then the class template parameters should be `Ta, Tb, ...`.
For example, we may have a container type that when given two `T*` then it should be a container of `T`, not `T*`.

The syntax is
```cpp
template <typename Param0, ...>
template_name (param0, ...) -> template_name<...>;
```
where `template_name` is the name of a class template and `...` is an explicit list of types.
Not sure if [[Template Parameter Pack]]s can be used here.

Example from the standard library:
```cpp
namespace std
{
	template <typename It>
	vector(It begin, It end)
		-> vector<typename std::iterator_traits<It>::value_tpe>;
```
This says that when a `std::vector>` is initialized with two iterators then we get a vector of whatever the type iterators' value type is, not a vector of iterators.

It is recommended that the deduction guides matches the behavior of the constructor overload set.


# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
- 2: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
