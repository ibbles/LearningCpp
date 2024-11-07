A nested namespace is a namespace inside of another namespace [(1)](https://youtu.be/3gGhP0C-xOY?t=55).

Example:
```cpp
namespace MyLibrary
{
	namespace Utilities
	{
		namespace Graphics
		{
		}
	}
}
```

or equivalently:
```cpp
namespace MyLibrary::Utilities::Graphics
{
}
```


# References

- 1: [_Core C++ 2021 :: C++17 key features_ by Alex Dathskovsky, CoreCppIL @ youtube.com 2021](https://www.youtube.com/watch?v=3gGhP0C-xOY)
