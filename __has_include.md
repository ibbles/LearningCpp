A preprocessor predicate used to test if a file is accessible by the preprocessor.
Can be used to test if a particular feature should be enabled or not.

Example where we test if some third-party library is available:
```cpp
#if __has_include(<third_party_library.h>)
	#include <third_party_library.>
	#define HAVE_THIRD_PARTY_LIBRARY 1
#else
	#define HAVE_THIRD_PARTY_LIBRARY 0
#endif
```

