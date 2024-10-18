Allows the compiler to deduce the type of a class from its initializer.

```cpp
// Without class template argument deduction.
// Must specify both the element type and number
// of elements of an std::array.
std::array<int, 5> data {1, 2, 3, 4, 5};

// With class template argument deduction.
// Element type and number of element is deduced.
std::array data {1, 2, 3, 4, 5};
```


# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
