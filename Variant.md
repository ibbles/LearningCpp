A variant is alike a `union`, but with a safer interface [(1)](https://youtu.be/e2ZQyYr0Oi0?t=406).
It is a container that holds exactly one element that is one of a set of explicitly listed allowed types.

```cpp
using IntOrDouble = std::variant<int, double>;
```

The value is stored inside the variant itself, there is no pointer or heap-allocated memory.

We can create containers that contains the variant type, making, kind of, a non-homogeneous container.
```cpp
using IntOrDouble = std::variant<int, double>;
std::vector<IntOrDouble> ints_and_doubles {1, 2.0, 3, 4.0, 5, 6, 7, 8.0};
```

Since the variant store its value inside itself, by-value, the data in the vector is tightly packed.
This is good for cache performance when iterating.

To access the value in the variant we can pass in a callable object with call operator overloads for the types in the variant to `std::visit`.
```cpp
struct Process
{
	void operator()(int i)
	{
		std::cout << "int: " << i << '\n';
	}

	void operator()(double d)
	{
		std::cout << "double: " << d << '\n';
	}
};

for (IntOrDouble& int_or_double : int_or_doubles)
{
	std::visit(Process(), int_or_double);
}
```
# References

- 1:  [_C++17 - The Best Features - Nicolai Josuttis ACCU 2018_ by Nicolai Josuttis, ACCU Conference @ youtube.com 2018](https://youtu.be/e2ZQyYr0Oi0)
