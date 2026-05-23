I don't get it.
Something about letting the compiler know that a new object has been created at a memory location.
This is fine if we use a pointer to the memory location give to us when the new object is created, not not fine if we use an old pointer that pointed to the old object.
```cpp
struct Data
{
	int data;
};

void work()
{
	Data* data = new Data(); // Create the first Data.
	new(data) Data(); // Create the second Data.
	// Not safe to use 'data', it points to the first data.
	data = new(data) Data(); // Create the third Data.
	// Now 'data' is safe to use because we got the pointer
	// from the creation of the third Data.
	new(data) Data();
	// Now 'data' is stale again, unsafe to use.
	data = std::launder(data);
	// Now 'data' is safe to use again.
}
```

The whole thing is weird to me.


# References

- 1: [_From Undefined to Defined: Using std::launder in C++_ by Andreas Fertig @ anreasfertig.com 2026](https://andreasfertig.com/blog/2026/05/from-undefined-to-defined-using-stdlaunder-in-cpp/)