Copy elision is when the compiler puts two variables in the same memory location after it notices that one is initialized by copy from the other right before the other is destroyed, which means that no actual copy needs to happen.
Made mandatory in some circumstances with [[C++17]].
Commonly used when a local variables is initialized from the return value of a function call where the called function returns another local variable of the same type.

```cpp
struct Data { /* Omitted: member variables. */ };

Data get_data()
{
	Data data;
	// Omitted: Assignment to the member variables of 'data'.
	return data;
}

void work()
{
	Data data = get_data();
}
```

Without copy elision the stack frame for `get_data` would contain one instance of `Data` and the stack frame for `work` would contain another, separate instance.
Upon return from `get_data` the bytes making up the `Data` instance in `get_data` would be copied into the bytes making up the `Data` instance in `work`.

With copy elision there would only be one instance of `Data` the one in `work`.
The member variable assignments in `get_data` would not write to its own stack frame but to the stack frame owned by `work`.

A complete example with copy elision:
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
    return data;
}
```
Resulting assembly code when compiled with GCC 14.2 and `-O2`:
```c
work():
        pushq   %rbx
        subq    $32, %rsp  // Allocate stack space for 'work_data'.
                           // %rsp points to the start of 'work_data'.
        movq    %rsp, %rdi // Set first parameter to 'work_data'.
        call    get_data() // Call get_data, passing in 'work_data'.
        movq    %rsp, %rdi // First parameter, %rdi, is not callee saved,
                           // so set it back to 'work_data' again.
        call    consume(char const*) // Call consume with pointer to 'work_data'.
        movl    16(%rsp), %edi  // Load an int from byte 16 on the stack.
                                // 16 bytes is 4 ints into 'work_data'.
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
                                 // caller'sstack frame.
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
                             // With copy elision we didn't need to do this.
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

# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
