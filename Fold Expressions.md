Used with [[Variadic Templates]].

```cpp
template <typename ... T>
auto add(T const& ... param)
{
	return (param + ...);
}

int work()
{
	return add(1, 2, 3, 4, 5);
}
```


# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
