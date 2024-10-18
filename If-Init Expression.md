An if-init expression is an if-statement that has two statements in the if-header.
The first statement, called the if-init expression, declares and initializes one or more variables.
The second statement is evaluated as a Boolean expression and is used to determine if the if-block should be executed or not.
Variables declared in the if-init expression have their lifetime tied to the if-block, i.e. they are not accessible before or after the if-block.
The purpose is to avoid having to declare the variable before the if-statement, which would make it accessible also after the if-statement.

Example where we get a handle to a widget and only work with it if the handle is valid.
```cpp
void work(Widget widget)
{
	if (Widget parent = widget.getParent(); parent.isVaid())
	{
		// Work with 'parent'.
	}
}
```

Example where we do a find and only work with the returned iterator if it is valid [(2)](https://youtu.be/fI2xiUqqH3Q?t=339).
```shell
template <typename Key, typename Value, typename Function>
void findAndUpdate<std::map<Key, Value>& table, Key key, Function getNewValue)
{
	if (auto it = table.find(key); it != table.end())
	{
		it->second = getNewValue(it->first);
	}
}
```

Example where we hold a mutex for a limited scope [(2)](https://youtu.be/fI2xiUqqH3Q?t=457):
```cpp
int sum(std::vector<int>& container, std::mutex& mutex)
{
	int sum = 0;

	if (std::lock_guard lock(mutex); !container.empty())
	{
		sum = std::accumulate(
			container.begin(), container.end(), 0, std::plus());
	}

	return sum;
}
```

If-init expressions makes if-statements very similar to for-loops.
The only difference is that for-loops loop and have a third expression that is run after each loop iteration, and if-statements can have an else-block.
```cpp
if (init; cond) {}
for (init; cond; update) {}
```


Variables declared in the if-init expression are in scope in all following blocks [(2)](https://youtu.be/fI2xiUqqH3Q?t=418).
```cpp
if (auto x = /* expression */; /* expression */)
{
	// x is accessible but not y.
}
else if (auto y = /* expression */; /* expression */)
{
	// x and y are accessible.
}
else
{
	// x and y are accessible.
}
```

# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
- 2: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)

