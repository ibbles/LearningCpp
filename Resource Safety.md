Resource safety means that every object is properly constructed and destroyed [(1)](https://youtu.be/I8UvQKvOSSw?t=315).
One way to do this in C++ is the [[RAII]] idiom, which means that a type's constructor should fully initialize an object and it destructor should release any resources that has been acquired.
There should be no need for the programmer to explicitly call `ini` or `shutdown` functions.

Every resource should be rooted in, e.g. owed by, a scope [(1)](https://youtu.be/I8UvQKvOSSw?t=2564).

Use smart pointers to keep track of ownership of objects representing resources [(1)](https://youtu.be/I8UvQKvOSSw?t=2661).

If possible, prefer non-allocated local objects [(1)](https://youtu.be/I8UvQKvOSSw?t=2683).
```cpp
void work(int n)
{
	Gadget gadget(n);
}
```
No risk of leakage.
No manual memory management.
No pointers we need to keep track of.
Just a name of an object.


# References

- 1: [_Delivering Safe C++ - Bjarne Stroustrup - CppCon 2023_ by Bjarne Stroustrup, CppCon @ youtube.com 2023](https://www.youtube.com/watch?v=I8UvQKvOSSw)
