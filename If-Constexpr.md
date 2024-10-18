An if-statement that is evaluated at compile time.
The expression must be `constexpr`.
If the expression evaluates to false then the if-block is not instantiated [(1)](https://youtu.be/fI2xiUqqH3Q?t=767).
This means that the code in the if-block must be parseable, but it doesn't need to compile.
For example, it may call function overloads that doesn't exist and use variables that are declared but not defined.

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


# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
