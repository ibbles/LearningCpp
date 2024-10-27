A nested namespace is a namespace inside of another namespace.

Example:
```cpp
namespace MyLibrary
{
	namespace Utilities
	{
		namespace Graphics
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