Copy elision is when the compiler puts two variables in the same memory location after it notices that one is initialized by copy from the other right before the other is destroyed, which means that no actual copy needs to happen.
Made mandatory in some circumstances with [[C++17]] [(1)](https://www.youtube.com/watch?v=QpFjOlzg1r4), [(2)](https://youtu.be/fI2xiUqqH3Q?t=2520).
Commonly used when a local variables is initialized from the return value of a function call where the called function returns another local variable of the same type.
This case is called _return value optimization_ or RVO.
Similar to [[Move Elision]].

Example of copy elision of a return value.
```cpp
// Some type with a bunch of data, potentially expensive to copy.
struct Data { /* Omitted: member variables. */ };

// A function that returns a Data by value.
Data get_data()
{
	// 'data' appears to be on the stack frame of 'get_data', but
	// it is actually on the stack frame of the caller and 'get_data'
	// is given a pointer to it.
	Data data;

	// Omitted: Assignment to the member variables of 'data'.

	// It looks like we copy 'data' out of the function, but this
	// return is actually a no-op.
	return data; // Important to not do std::move here.
}

void work()
{
	// This looks like a copy from the return value of 'get_data',
	// but it is not.
	Data data = get_data();
}
```

Without copy elision the stack frame for `get_data` would contain one instance of `Data` and the stack frame for `work` would contain another, separate instance.
Upon return from `get_data` the bytes making up the `Data` instance in `get_data` would be copied into the bytes making up the `Data` instance in `work`,
possibly using a non-trivial copy constructor.

With copy elision there would only be one instance of `Data` the one in `work`.
The member variable assignments in `get_data` would not write to its own stack frame but to the stack frame owned by `work`.

A complete example with copy elision:
```cpp
#include <array>

// A function that produces a value. Doesn't matter
// what it does.
int get_value();

// A collection of functions that consume a value.
// Doesn't matter what they do.
void consume(int);
void consume(char const*);

// A data-carrying type that is "expensive" to copy.
struct Data
{
    std::array<int, 8> values;
};

// Declaration of a function that returns data by value,
// i.e. that would traditionally return by copy.
Data get_data();

// Functions that gets a Data from somewhare and does something
// with it.
void work()
{
	// This looks like a copy, but it isn't.
    Data work_data = get_data();
    consume(reinterpret_cast<char const*>(&work_data));
    consume(work_data.values[4]);
}

__attribute__((noinline))
Data get_data()
{
    Data data;
    std::fill(
        data.values.begin(),
        data.values.end(),
        get_value());

    consume(reinterpret_cast<char const*>(&data));
    return data;
}
```
Resulting assembly code when compiled with GCC 14.2 and `-O2`:
```c
work():
        pushq   %rbx
        subq    $32, %rsp  // Allocate stack space for 'work_data'.
					       // 4 (sizeof int) * 8 (values.size()) = 32.
                           // %rsp points to the start of 'work_data'.
        movq    %rsp, %rdi // Set first parameter to 'work_data'.
        call    get_data() // Call get_data, passing in 'work_data'.
        movq    %rsp, %rdi // First parameter, %rdi, is not callee saved,
                           // so set it back to 'work_data' again.
        call    consume(char const*) // Call consume with pointer to 'work_data'.
        movl    16(%rsp), %edi  // Load an int from byte 16 on the stack.
                                // 16 bytes is 4 ints into 'work_data'.
                                // 4 (index) * 4 (sizeof(int)) = 16.
        call    consume(int)    // Consume the int.
        addq    $32, %rsp       // Dealocate stack space for 'work_data'.
        popq    %rbx
        ret

get_data():
        pushq   %rbx
        movq    %rdi, %rbx   // First parameter is pointer to return value,
                             // i.e. a pointer into the caller's stack frame.
                             // Save it in %rbx since we will need it later.
        call    get_value()  // Get the value to fill 'data' with.
                             // Is placed in %eax.
        movq    %rbx, %rdi   // Restore the parameter, i.e. pointer to
                             // caller's stack frame.
        movd    %eax, %xmm1      // Prepare for some SIMD by moving the value
        pshufd  $0, %xmm1, %xmm0 // to a vector register and duplicate.
        movups  %xmm0, (%rbx)    // Write values[0] through [3] into the
                                 // caller's stack frame.
        movups  %xmm0, 16(%rbx)  // Write values[4] through [7] into the
                                 // caller's stack frame.
        call    consume(char const*) // %rdi, the parameter, still holds
                                     // pointer into the caller's stack frame.
        movq    %rbx, %rax   // %rbx also holds a pointer to the caller's
                             // stack frame. Put it into %eax, which is where
                             // return values are placed. So we return that
                             // same pointer we were given.
        popq    %rbx
        ret
```

A complete example without copy elision:
```cpp
#include <array>

char const* get_name();
void consume(char const*);

int get_value();
void consume(int);


struct Data
{
    std::array<int, 8> values;
};

Data get_data();

void work()
{
    Data work_data = get_data();
    consume(reinterpret_cast<char const*>(&work_data));
    consume(work_data.values[4]);
}

__attribute__((noinline))
Data get_data()
{
    Data data;
    std::fill(
        data.values.begin(),
        data.values.end(),
        get_value());

    consume(reinterpret_cast<char const*>(&data));
    return std::move(data); // std::move prevents copy elision.
}
```
Resulting assembly code when compiled with GCC 14.2 and `-O2`:
```C
work():
        pushq   %rbx
        subq    $32, %rsp    // Allocate stack space for 'work_data'.
                             // %rsp points to the start of 'work_data'.
        movq    %rsp, %rdi   // Set first parameter to 'work_data'.
        call    get_data()   // Call get_data, passing in 'work_data'.
        movq    %rsp, %rdi   // First parameter, %rdi, is not callee saved,
                             // so set it to 'work_data' again.
        call    consume(char const*) // Call consume with pointer to 'work_data'.
        movl    16(%rsp), %edi  // Load an int from byte 16 on the stack.
                                // 16 bytes is 4 ints into 'work_data'.
        call    consume(int)    // Consume the int.
        addq    $32, %rsp       // Deallocate stack space for 'work_data'.
        popq    %rbx
        ret

get_data():
        pushq   %rbx
        movq    %rdi, %rbx    // First parameter is pointer to return value.
                              // i.e. a pointer into the caller's stack frame.
                              // Save it in %rbx since we will need it later.
        subq    $32, %rsp     // Allocate stack space for 'data'.
                              // With copy elision we didn't need to do this.
        call    get_value()   // Get the value to fill 'data' with.
                              // Is placed in %eax.
        movq    %rsp, %rdi    // Put pointer to 'data' in first parameter.
        movd    %eax, %xmm1      // Prepare for some SIMD by moving the value
        pshufd  $0, %xmm1, %xmm0 // to a vector register and duplicate.
        movaps  %xmm0, (%rsp)    // Write data[0] through [3].
        movaps  %xmm0, 16(%rsp)  // Write data[4] through [7].
        call    consume(char const*) // %rdi, the parameter, still holds 'data'.
        movq    %rbx, %rax       // %rbx holds pointer to the return value
                                 // in the caller's stack frame.
                                 // %eax is the return register.
                                 // So we return back the pointer we were given.
                                 //
                                 // At this point the copy elided variant was
                                 // already done. Here we need to copy from
                                 // 'data' to 'work_data', i.e. write into
                                 // the caller's stack frame.
        movdqa  (%rsp), %xmm0    // Load data[0] through [3] to %xmm0.
		movups  %xmm0, (%rbx)    // Write data[0] through [3] to return value.
        movdqa  16(%rsp), %xmm0  // Load data[4] through [7] to %xmm0.
        movups  %xmm0, 16(%rbx)  // Write data[4] through [7] to return value.
        addq    $32, %rsp        // Deallocate stack space for 'data'.
        popq    %rbx
        ret
```


# Guaranteed Copy Elision - Returning Non-Copyable Values

Guaranteed copy elision, introduced with C++17, makes it possible to return non-copyable, types from functions [(2)](https://youtu.be/fI2xiUqqH3Q?t=2520).
Only works for non-named variables, i.e. _named return value optimization_ a.k.a. NRVO, is not guaranteed.

```cpp
auto grab_lock(std::mutex& mutex)
{
	return std::lock_guard(mutex);
}
```

# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
- 2: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)

