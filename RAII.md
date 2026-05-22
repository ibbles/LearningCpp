RAII is short for Resource Acquisition Is Initialization.
What it really means is that when you create an object then it will be created in an initialized state, and when the object is no longer needed then it will clean up after itself.
A constructor constructs, initializes, and establishes the invariants the object [(1)](https://youtu.be/I8UvQKvOSSw?t=1339).
A destructor releases any resources owned by the object.

Examples of resources that are often managed in an RAII-fashion:
- Memory allocations.
- File handles.
- Locks.
- Sockets.
- Shaders.


Example code showing what RAII is meant to protect us from [(1)](https://youtu.be/I8UvQKvOSSw?t=1409):
```cpp
void work(char const* file_path)
{
	FILE* file = fopen(file_path, "r");
	// Use f.
	fclose(file);
}
```
There will be a resource leak if the `// use f.` part contains a block that returns or throws an exception without calling `fclose(file)`.
We want the lifetime of the resource to be linked to the lifetime of the variable representing it, the `file`  function scope variable in this example.
This is what `std::[io]?fstream` does:
```cpp
void work(char const* file_path)
{
	std::fstream file(file_path, std::ios_base::read);
	// Use file.

	// No need to close 'file', it is done automatically.
}
```

The file will be closed whenever `file` goes out of scope, for whatever reason.

The standard library contains a wealth of classes for handling various types of resources in an RAII way:
- Memory allocations.
	- Smart pointers and container types.
- File handles.
	- `std::[io]?fstream`
- Locks.
	- `std::unique_lock` and `std::lock_guard`.


# References

- 1: [_Delivering Safe C++ - Bjarne Stroustrup - CppCon 2023_ by Bjarne Stroustrup, CppCon @ youtube.com 2023](https://www.youtube.com/watch?v=I8UvQKvOSSw)
