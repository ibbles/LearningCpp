Type safety means that an object is only accessed according to the type with which it was defined [(1)](https://youtu.be/I8UvQKvOSSw?t=306).
Due to its C base, C++ does not guarantee this by default.
Accessing an object as another type than with which it was defined is called type punning.
Many, but not all, type safety violations are undefined behavior due to the pointer aliasing rules.
This is undefined behavior:
```cpp
int encode_float(float f)
{
	return *(int*)&f;
}

float decode_float(int i)
{
	return *(float*)&i;
}
```

There are cases in C++ where we are allowed to break the type safety rules.
One such case is when using a `char` pointer to read or write the bytes of an object.
```cpp
void encode_float(float f, int* storage)
{
	static_assert(sizeof(f) == sizeof(*storage));
	memcpy(storage, &f, sizeof(*storage));
}

void decode_float(int storage, float* f)
{
	static_assert(sizeof(storage) == sizeof(*f));
	memcpy(f, &storage, sizeof(*f));
}
```

# References

- 1: [_Delivering Safe C++ - Bjarne Stroustrup - CppCon 2023_ by Bjarne Stroustrup, CppCon @ youtube.com 2023](https://www.youtube.com/watch?v=I8UvQKvOSSw)
