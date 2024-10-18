A way to unpack a [[Destructurable]] object into named pieces, i.e. individual variables.
This is called destructuring [(2)](https://youtu.be/fI2xiUqqH3Q?t=84).
Given a composite object named `obj` it is destructured into local variables named `a`, `b`, `c`, etc with
```cpp
auto [a, b, c, ...] = obj;
```

The `...` is not part of the syntax, you must know the number of pieces and give each a name.

Example using a `std::pair`, returned from a function, for `obj`:
```cpp
std::pair<int, int> find_range();

void work()
{
	auto [begin, end] = find_range();
}
```

# Iterating Over A Table

Example using table lookup [(2)](https://youtu.be/fI2xiUqqH3Q?t=293):
```cpp
template <typename Key, typename Value, typename Function>
void update(std::map<Key, Value>& table, Function getNewValueForKey)
{
	for (auto&& [key, value] : table)
	{
		value = getNewValueForKey(key);
	}
}
```

Iterating over a `std::map` gives you (key, value) pairs.
The example uses structured bindings to assign names to the first and second elements of the pair.
I'm not sure what difference `auto&&` makes over `auto&` in this case.

The variables will have appropriate const-ness.
Since iterating over a map gives you a `std::pair<const Key, Value>` for each element, the destructured variables `key` and `value` will have types `Key const&` and `Value&`, respectively.
So inside the for-loop we can write to `value`, which will update the value stored in the table, but we cannot write to `key` since it is a `const&`.


# `auto` Variants

The `auto` need not be just `auto` but can be any variant supported by the regular type deduction rules for [[Auto]]  [(2)](https://youtu.be/fI2xiUqqH3Q?t=268).
- `auto`: By value even if right hand side is a reference.
- `auto const`: Local variable is `const`.
- `auto&`: Capture by reference.
- `auto&&`: Capture by TODO Name?

# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
- 2: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
