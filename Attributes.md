Attributes are markers or decorators we put on symbols to tell the compiler something about that symbol.
The are put within `[[` and `]]` brackets

# `nodiscard`

Tell the compiler that the return value from a function must be used by the caller [(1)](https://youtu.be/3gGhP0C-xOY?t=717).
We accept assigning it to a variable to count as being used.
```cpp
[[nodiscard]]
float square(float x)
{
	return x * x;
}

void work(float a, float b)
{
	float a2 = square(a); // No error.
	square(b); // Error, return value not used.
```


# `maybe_unused`

Specify that a function's return value or a variable might not be used [(1)](https://youtu.be/3gGhP0C-xOY?t=617).
Prevents the compiler from emitting a warning about it.

```cpp
[[maybe_unused]]
float work([[maybe_unused]] int x, float y)
{
	return y;
}
```


# References

- 1: [_Core C++ 2021 :: C++17 key features_ by Alex Dathskovsky, CoreCppIL @ youtube.com 2021](https://www.youtube.com/watch?v=3gGhP0C-xOY)
