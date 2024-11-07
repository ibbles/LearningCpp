A template parameter pack is a template parameter that contains multiple types.
Can be used to forward an unknown number of parameters to another function.
```cpp
template <typename T, typename... Args>
T construct(Args... args)
{
	return T(args...);
}
```

The number of parameters is returned by the `sizeof...` operator [(1)](https://youtu.be/3gGhP0C-xOY?t=1283).
```cpp
template <typename... Args>
void work(Args... args)
{
	if (sizeof...(args) > 5)
	{
		std::cerr << "Too many parameters passed to work.\n";
	}

	// Work with 'args'.
}
```

# References

- 1: [_Core C++ 2021 :: C++17 key features_ by Alex Dathskovsky, CoreCppIL @ youtube.com 2021](https://www.youtube.com/watch?v=3gGhP0C-xOY)
