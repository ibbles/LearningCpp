Also called _compile-time if_ [(2)](https://youtu.be/e2ZQyYr0Oi0?t=153).
An if-statement that is evaluated at compile time.
The Boolean expression must be [[constexpr]].
If the expression evaluates to false then the if-block is not instantiated [(1)](https://youtu.be/fI2xiUqqH3Q?t=767).
This means that the code in the if-block must be parseable, but it doesn't need to compile.
For example, it may call function overloads that doesn't exist and use variables that are declared but not defined.

Example detecting if a type parameter is a pointer or not [(3)](https://youtu.be/3gGhP0C-xOY?t=401):
```cpp
template <typename T>
auto getT(T t)
{
	if constexpr (std::is_pointer_v<T>)
	{
		return *t;
	}
	else
	{
		return t;
	}
}
```

If called with an `int*` argument the `getT` function will be compiled as
```cpp
int getT(int* t)
{
	return *t;
}
```

If called with an `int` argument the `getT` function will be compiled as
```cpp
int getT(int t)
{
	return t;
}
```


Example using a [[Template Parameter Pack]] [(1)](https://youtu.be/fI2xiUqqH3Q?t=652):
```cpp
template <typename THead, typename... TTail>
void print(THead&& head, TTail&&... tail)
{
	std::cout << std::forward<THead>(head) << '\n';

	if constexpr (sizeof...(TTail) > 0)
	{
		print(std::forward<TTail>(tail)...);
	}
}
```

Example that converts a bunch of different types to `std::string` [(2)](https://youtu.be/e2ZQyYr0Oi0):
```cpp
template <typename T>
std::string to_string(T value)
{
	if constexpr (std::is_arithmetic_v<T>)
	{
		return std::to_string(value);
	}
	else if constexpr (std::is_save_v<T, std::string>)
	{
		return x;
	}
	else
	{
		return st::string(x);
	}
}

to_string(43); // Enters the first if.
to_string("hello"s); // Enters the second if.
to_string("hello"); // Enters the thrird if.
```

# With Runtime Initializer

An if-[[constexpr]] may be combined with an initializer even when the initializer is not [[constexpr]].
An example where we operate on a container differently depending on if the container contains pointers or not, but regardless of which we always lock a mutex first, and we only lock the mutex for the block of code where processing is happening [(2)](https://youtu.be/e2ZQyYr0Oi0?t=257):
```cpp
template <typename ElementT, typename MutexT>
void process(std::vector<ElementT>& elements, MutexT mutex)
{
	if constexpr (std::lock_guard lock(mutex);
		std::is_pointer_v<typename ElementT>)
	{
		// Process pointer elements.
	}
	else
	{
		// Process non-pointer elements.
	}
}
```


# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
- 2: [_C++17 - The Best Features - Nicolai Josuttis ACCU 2018_ by Nicolai Josuttis, ACCU Conference @ youtube.com 2018](https://youtu.be/e2ZQyYr0Oi0)
- 3: [_Core C++ 2021 :: C++17 key features_ by Alex Dathskovsky, CoreCppIL @ youtube.com 2021](https://www.youtube.com/watch?v=3gGhP0C-xOY)
