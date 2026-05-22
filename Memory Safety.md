
A program being memory safe means that it satisfies the following requirements [(1)](https://youtu.be/I8UvQKvOSSw?t=329):
- Every pointer either points to a valid object or is the `nullptr`.
- Every reference through a pointer is not through the `nullptr`.
- Every access to an array is in-range.

A "pointer" in this context can be more than just a C++ language pointer.
It can be a smart pointer, which is just a class with a pointer inside.
It can be a reference, which is similar to a pointer in that it is a way to get at an object somewhere else.
It can be an ID, key or index that is used to look-up an object in a table or container.


# Every Pointer Either Points To A Valid Object Or Is The `nullptr`

A pointer that points to memory that does not hold a valid object is said to be a dangling pointer [(1)](https://youtu.be/I8UvQKvOSSw?t=2738).
Common ways to get a dangling pointer:
- The pointer is uninitialized and holds garbage.
- The pointer points to memory that used to hold an object but that object has been destroyed.

Use smart pointers to:
- Make ownership semantics clear
- To ensure that objects are deleted and memory freed only when it is supposed to.
- Ensure there are no uninitialized pointers.

Don't use owning raw pointers.

## Container Reallocation And Pointer / Iterator Invalidation

A common way to get a dangling pointer is by container reallocation [(1)](https://youtu.be/I8UvQKvOSSw?t=3003).
If we have a pointer to an element in a container and then the container performs a reallocation then the pointer becomes a dangling pointer.
```cpp
// A sentinel value is used to determine when 'work' should stop.
const int SENTINEL = /* Some value. */;

void collect_work(std::vector<int>& container)
{
	// Add the next batch of work to the container
	// and sort by priority.
	container.push_back(9);
	std::sort(container);
}

void work(std::vector<int>& container)
{
	// Add a sentinel value so we can know when to stop.
	container.push_back(SENTINEL);
	// Keep an iterator to the sentinel value's initial position
	int* sentinel_it = --container.end();
	collect_work(container);
	// At this point 'sentinel_it' may have become invalidated.
	while (*sentinel_it != SENTINEL)
	{
		// Do work on 'container'.
	}
}
```

To fix this we can either call `reserve` on the container with a sufficiently large capacity to ensure that it never needs to reallocate, or use a container that don't need reallocation, such as `std::list` or `std::deque`.


## /


A bunch of bad things can happen if we use a dangling pointer [(1)](https://youtu.be/I8UvQKvOSSw?t=2738):
- Unexpectedly write into memory used by an unrelated object allocated in that memory.
	- This is a [[Type Safety]] violation.
- Unexpectedly read from it and get garbage data.
	- This is a [[Type Safety]] violation.



# Every reference through a pointer is not through the `nullptr`

Make sure you know if a particular pointer can be `nullptr` or not [(1)](https://youtu.be/I8UvQKvOSSw?t=2530).
Always check for `nullptr` if it can be `nullptr`.
Always `assert` if it cannot be `nullptr`.


# Every access to an array is in-range

Use containers for all multi-element collections.
Use `span` when passing around a contiguous collection of elements.
Use ranged-for instead of indices when accessing a container.
Don't index into raw pointers because there isn't sufficient information available to range checking [(1)](https://youtu.be/I8UvQKvOSSw?t=2445).


# Ownership

Every object should have one and only one owner [(1)](https://youtu.be/I8UvQKvOSSw?t=2854).
The owner is the one who is responsible for doing the delete, close, cleanup, etc.
There can be other pointers to the same object, but they cannot be after the end of the lifetime of the owner.


# References

- 1: [_Delivering Safe C++ - Bjarne Stroustrup - CppCon 2023_ by Bjarne Stroustrup, CppCon @ youtube.com 2023](https://www.youtube.com/watch?v=I8UvQKvOSSw)
