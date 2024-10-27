A static assert is an assert that is evaluated at compile time.
If a static assert fails then the compilation fails.
You cannot have a compiled binary that violates any static assert.

A static assert is added with `static_assert(CONDITION, MESSAGE);` where `CONDITION` is a Boolean expression that should be true and `MESSAGE` is a string that is printed by the compiler whenever `CONDITION` is false and compilation fails.

The message is optional [(1)](https://youtu.be/fI2xiUqqH3Q?t=2462).

Example:
```cpp
static_assert(sizeof(int) == 4, "'int' must be 32-bit.");
static_assert(sizeof(int) == 4);
```


# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)

