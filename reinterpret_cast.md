`reinterpret_cast` is used to change the type of a value [(2)](https://eel.is/c++draft/expr.reinterpret.cast).

# Allowed Type Conversions
Only a very limited set of types can be converted between using `reinterpret_cast` [(2)](https://eel.is/c++draft/expr.reinterpret.cast#1).
- An integer type to itself, i.e. a no-op [(2)](https://eel.is/c++draft/expr.reinterpret.cast#2).
- An enumeration type to itself, i.e. a no-op [(2)](https://eel.is/c++draft/expr.reinterpret.cast#2).
- A pointer type to itself, i.e. a no-op [(2)](https://eel.is/c++draft/expr.reinterpret.cast#2).
- A pointer to a large enough integer type, typically `std::uintptr_t` [(2)](https://eel.is/c++draft/expr.reinterpret.cast#4).
- An integer type to a pointer [(2)](https://eel.is/c++draft/expr.reinterpret.cast#5).
	- A round-trip pointer-integer-pointer is guaranteed to produce the same pointer again [(2)](https://eel.is/c++draft/expr.reinterpret.cast#5).
- A function pointer to another function pointer [(2)](https://eel.is/c++draft/expr.reinterpret.cast#6)
	- The resulting function pointer can only be called if the two types are call-compatible.
- A pointer to an object of some type to a pointer to an object of some other type [(2)](https://eel.is/c++draft/expr.reinterpret.cast#7).
	- The resulting object pointer can only be dereferenced if the object stored at the pointed-to memory location is of a compatible type.
- Some platforms support converting back and forth between object pointers and function pointers.
- A pointer to a member to another member [(2)](https://eel.is/c++draft/expr.reinterpret.cast#10).
	- This is weird to me.

`reinterpret_cast` can be used to cast a pointer to a large enough integral type [(4)](https://eel.is/c++draft/expr.reinterpret.cast#4).
```cpp
#include <cstdint>

std::uintptr_t get_stack_address()
{
    int on_stack {1};
    std::uintptr_t stack_addr = reinterpret_cast<std::uintptr_t>(&on_stack);
    return stack_addr;
 }
```


# Misuse: Reading Raw Bytes As An Object

`reinterpret_cast` is sometimes used to take a pointer that points to some type and cast it so that it points to some other non-related type.
Dereferencing such a pointer is undefined behavior.

A common misuse is to take a byte buffer, for example received from the network or read from a file, and using those bytes as a struct directly [(1)](https://andreasfertig.com/blog/2026/04/what-reinterpret_cast-doesnt-do/).
```cpp
#include <array>
#include <cstdint>
#include <span>

struct Message
{
	uint32_t type;
	uint32_t channel;
	uint32_t size;
	std::array<uint32_t, 128> payload;
};

bool on_message_received(std::span<std::byte> bytes)
{
	if (bytes.size() != sizeof(Message))
		return false;
	
	// BAD Treat the byte buffer as-if it was a Message.
	// Even though the bytes may actually come from a real
	// Message at some point, for this process and this
	// particular byte buffer, there is no Message object
	// there and treating the bytes as one is undefined behavior.
	Message* message = reinterpret_cast<Message*>(bytes.data());

	// Do something with 'message', even though 'message'
	// doesn't actually point to an instance of 'Message'.
	// That's undefined behavior.
}
```

We are trying to take a piece of memory that has some type, `std::byte`, and treat it as some other unrelated type, `Message`.
This is undefined behavior.

The traditional way to handle this properly is with `memcpy`.
```cpp
	Message message;
	memcpy(&mesage, bytes.data(), sizeof(Message));
	
	// Do something with 'message', which is an actual
	// 'Message' object populated with the bytes we were given.
```

With C++23 we got `std::start_lifetime_as`.
I don't have a C++23 compiler so I haven't tested it much.
Something like this, I guess:
```cpp
	Message* message = std::start_lifetime_as<Message>(bytes.data());
```

A requirement is that the pointer passed to `std::start_lifetime_as`, (`bytes.data()` in this case), is suitable aligned for the new type, and large enough to hold it.


# References

- 1: [_What reinterpret_cast doesn't do_ by Andreas Fertig @ andreasfertig.com 2026](https://andreasfertig.com/blog/2026/04/what-reinterpret_cast-doesnt-do/)
- 2: [_Reinterpret cast_ @ eel.is/c++draft](https://eel.is/c++draft/expr.reinterpret.cast)
