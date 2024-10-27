A preprocessor predicate used to test if a file is accessible by the preprocessor [(1)](https://youtu.be/fI2xiUqqH3Q?t=2638).
Can be used to test if a particular feature should be enabled or not.

Example where we test if some third-party library is available:
```cpp
#if __has_include(<third_party_library.h>)
	#include <third_party_library.h>
	#define HAVE_THIRD_PARTY_LIBRARY 1
#else
	#define HAVE_THIRD_PARTY_LIBRARY 0
#endif
```

Not sure, but I worry that `__has_include` will be true also for headers that will be all `#if`'d out because of the language standard version current being compiled for.
For example, consider the following program being compiled with a c++20 compatible compiler but set to compile in C++17 mode.
```cpp
#if __has_include(<span>)
	#include <span>
	#define HAVE_SPAN 1
#else
	#define HAVE_SPAN 0
#endif
```

Is it that case that the `__has_include` predicate will return true because the file is there on the file system, but `#include <span>` will be expanded to nothing by the preprocessor because the `#if __cplusplus > 201703L` guard at the top will cause the entire file contents to be ignored?

# Reference

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
