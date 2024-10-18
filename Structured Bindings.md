A way to unpack a composite into named pieces, i.e. variables.

```cpp
std::pair<int, int> find_range();

void work()
{
	auto [begin, end] = find_range();
}
```

# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
