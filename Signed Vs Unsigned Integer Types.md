# Motivation

C++ provides two families of integer types: signed and unsigned.
This means that every time we need a variable, constant, or literal of integer type we also need to decide if it should be signed or unsigned.
The purpose of this note is to discuss reasons for choosing one variant or the other, to evaluate the advantages and disadvantages of using signed or unsigned integers.
Neither option is obviously better than the other and both come with their own set of problems [(34)](https://youtu.be/Fa8qcOd18Hc?t=3080), [(75)](https://youtu.be/82jVpEmAEV4?t=3675).
Either variant can be used and all problematic code snippets presented here can be fixed regardless of the variant used.
We can always blame "bad code" on "bad programmers", but systematic decisions should be done with a greater context in mind.
The goal is to reduce the numbers mistakes, bugs, vulnerabilities, system outages, and intrusions as much as possible and to do that we need to embrace recommendations that has the widest positive impact possible.

The purpose for this note is to explore the pros and cons of using each in cases where it is not obvious which one we should use, such as for indices and sizes.
There is no universally agreed upon choice and domain specific considerations may cause one or the other to be favored.
Use this note as a collection of things to consider when making a project-wide decision for a new project.

The basis for the evaluation is that code should be clear and obvious.
The straight-forward way to write something should not produce incorrect results or unexpected behavior.
If what the code says, or at least hints at after a quick glance, isn't what the code actually does then there is a deeper problem than just "bad code, bad programmer".
We are not content with having the code being correct when we first check it in, we also want it to remain correct through multiple rounds of modification and extension by multiple people.
Real-world programmers writing real-world programs tend to have other concerns in mind than the minutiae of implicit integer conversion rules and overflow semantics.
What we want to do is reduce the number of ways things can go wrong, mitigate the bad effects when something does go wrong, and minimize the cognitive load white reading or writing code to reduce bugs overall [(62)](https://news.ycombinator.com/item?id=29766658).
We want to determine which choice leads to errors that are more common, have more dire consequences, and which are going to be harder to identify when they happen [(75)](https://youtu.be/82jVpEmAEV4?t=3675), and then avoid that choice.
We want tools; language and compiler included but also others, that help us prevent errors [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
We want to help the compiler help us.
If one choice makes it possible for the compiler to identify more of our mistakes then that is a point in favor of that choice.
If one choice puts a larger responsibility on the programmer to eliminate edge cases then that is a point against the choice.
The idea is the same as using `const` as much as possible [(73)](https://news.ycombinator.com/item?id=2364065).
Programmers do plenty of dumb mistakes, it is better if those can be found quickly in the editor rather than during testing or, even worse, by the users [(75)](https://youtu.be/82jVpEmAEV4?t=3830).

We can have different priorities when deciding between using signed or unsigned integers, and put different importance on each priority from project to project.
- Robustness: It should be difficult to write bugs and easy to identify them when they happen.
- Readability: The code should make sense, whatever that means.
- Runtime performance: The code should execute as fast as possible.
- Memory usage: Always use the smallest type possible.
- Avoid undefined behavior.
- Undefined behavior on error: It is diagnosable with an undefined behavior sanitizer.
	- Though being undefined behavior means the compiler may do unexpected transformations to our code, meaning that the error might not actually happen at runtime and instead the application does something completely different.

There is a fundamental difference in mindset between an application that should keep going and do its best despite detected errors, and an application where an incorrect result is fatal and it is much preferable to terminate early and in an obvious way [(80)](https://www.martinfowler.com/ieeeSoftware/failFast.pdf).
Do we want to detect and handle errors as best as we can and let the application keep running, or should errors lead to obvious incorrect application behavior such as a crash, infinite loop, failed assert, or similar.
An obvious error means we can fix our code, but may lead to brittle software and unsatisfied users.

Some data is inherently signed.
Sometimes we need to use such data to compute indices or sizes (citation needed).
By making the container interface unsigned we force unsigned values upon the user,
we make the decision for them.

Is this more of a problem than the opposite? Forcing signed values upon the user?
It is if the range of the signed type can be assumed to be large enough.
All not-too-large unsigned values can be represented in a signed value.
The conversion is safe.
The same is not true for signed to unsigned.
There are many reasonable sized negative values that cannot be represented by an unsigned type.


# Integer Basics

## Limited Range

Fixed sized integer types, which all primitive integer types are, have the drawback that they can only represent a limited range of values.
When we try to use them outside of that range they wrap, trap, saturate, or trigger undefined behavior [(27)](https://blog.regehr.org/archives/1401).
Typically, unsigned integers wrap while signed integer trigger undefined behavior.
In most cases neither of these produce the result we intended and often lead to bugs and / or security vulnerabilities [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
A notable exception is algorithms that explicitly need wrapping behavior, such as some encryption and hashing algorithms.
If you know that you need an unsigned integer with wrapping behavior then use an unsigned integer with wrapping behavior, this note does not apply.

Where on the number line the representable values are placed varies between signed and unsigned integers, and between integer types with different bit widths.
A signed integer type is one that can represent both positive and negative numbers, including zero.
An unsigned integer is one that can only represent positive numbers, including zero.
This means that a signed and an unsigned integer of the same bit width have some, but not all, values in common.
For each integer size we can split the range into three segments.
Example using `int` and `unsigned int`:
- `INT_MIN` to -1: Only representable by `int`.
- 0 to `INT_MAX`: Representable by both `int` and `unsigned int`.
- `INT_MAX` + 1 to `UINT_MAX`: Only representable by `unsigned int`.


## Integer Sizes

C++ provides the integer types `char`, `short`, `int`, `long`, and `long long`, with an unsigned variant of each.
They all have an implementation defined sized and an unsigned type is always the same size as the corresponding signed type.
`char` is a bit special in that there are three `char` types (`char`, `signed char`, and `unsigned char`) and they are all 1 byte in size.
There are integer types with fixed size as well: `int8_t`, `int16_t`, `int32_t`, and `int64_`, and an unsigned variant for each.
These are not separate types from the earlier integer types, but type aliases.
It is common that `int32_t` and `int` are the same type.

Another pair of integer types is `size_t` and `ptrdiff_t`.
These are integer types with an implementation defined size such that `size_t` can hold the size of any memory object, including arrays and heap allocated buffers, and `ptrdiff_t` is the result of pointer subtraction.
It is common for `size_t` and `ptrdiff_t` to be the same size, but it is not required.
In the case where `size_t` and `ptrdiff_t` have the same size, `size_t` can hold larger positive values than `ptrdiff_t` can.
This means that there are memory objects so large, i.e. in the upper half of the range of `size_t` in size, that `ptrdiff_t` doesn't have enough range to represent the difference between two pointers pointing to bytes in the object far away from each other.
That is a problem and trying compute such a difference is undefined behavior.
```cpp
size_t const size = static_cast<size_t>(PTRDIFF_MAX) + 10;
char const* const memory = malloc(size);
char const* const end = memory + size;
ptrdiff_t const size = end - memory; // Undefined behavior.
```

There is also `intptr_t` which is an integer type large enough to hold the result of a data pointer cast to an integer.
On most modern machines `intptr_t` and `size_t` have the same size.
On machines with segmented memory they may be different.


## Integer Behavior

Choosing either a signed or unsigned integer types comes with a number properties of the declared constant or variable.
A problem is that signed and unsigned expresses multiple properties and it is not always possible to get the exact combination that we want.
- Modular arithmetic / wrapping.
- Value may only be positive.
- Value can be negative.
- Over / underflow not possible / not allowed.


## Implicit Type Conversions

Arithmetic operations are always performed with a single type [(49)](https://eel.is/c++draft/expr.arith.conv).
There are operations that are not arithmetic, such as bit shifts and Boolean logic.
If the operator is not a unary operator and the operands doesn't have the same type but are at least as large as `int` then one of them will be converted to the type of the other.
This process is called the usual arithmetic conversions.
The way this is done is non-trivial, but for this note we simplify it to the following rules:
- If they are the same size then the signed value is converted to the unsigned type.
- Otherwise, the value with a smaller type is converted to the type of the larger.

If you multiply an `int` and an `int64_t` then the computation will be performed using `int64_t` because `int64_t` is (typically) larger than `int`.
If you multiply an `int` and an `unsigned int` then the computation will be performed using `unsigned int` because `int` and `unsigned int` are the same size.

All arithmetic operations are performed on at least the size of the `int` type, so if any value has a type smaller than that then it is converted to `int` before evaluating the operator.
This is called integer promotion.

Unexpected type conversions that changes the sign of a value is a common source of bugs.
Many compilers can therefore emit warnings for this, typically enabled with `-Wsign-conversion`.
See also _Dangers_ > _Mixing Signed And Unsigned Integer Types_.

Implicit conversions can also happen when we assign a value of one type to a variable of another type, or pass an argument to a function parameter.
In this case we may be suffering from truncation.
Truncation is when the value of a larger type is stored in a variables with a smaller size.
The high bits that don't fit are simply removed.
To be notified when this happens enable the `-Wshorten-64-to-32` warning.
Not sure why there is no `-Wshorten-32-to-16` etc, but there is `-Wimplicit-int-conversion` that catches at lest that case.

The alternative to implicit type conversion is explicit type conversions, i.e. casts.
If we chose to mix signed and unsigned integer types, see _Dangers_ >  _Mixing Signed And Unsigned Integer Types_, for example to use a signed loop counter when iterating over a container with an unsigned size type, then we may chose to either rely on implicit conversions or explicit casts.
An alternative is to wrap the container types in a view that provides a signed interface.

For non-arithmetic operations, such as bit shifts and Boolean operators, the operands are not converted to the same type.
In the following example [(87)](https://youtu.be/Iz2UOgLMj58?t=1723) the left hand side operand of `<<` is an `int` and the right hand side is `uint64_t`.
The code tries to guard against too large shifts, since it is not allowed to shift more than the number of bits in the shifted value, by making sure the shift amount is never larger than 63.
```cpp
uint64_t work(uint64_t count)
{
	return 1 << (count % 64);
}
```
We may think that the `int` is converted to `uint64_t`, the larger unsigned type, before the shift happens, but that is not the case.
If `count` is in the range \[32, 63\] then we will shift the 32-bit `1` by more than its bit width and we get undefined behavior.

The operators that does not trigger the usual arithmetic conversions are:
- Bit shifts `<<` and `>>`.
- Assignment operators `=`, `+=`, `-=`, etc.
- The conditional operator `?:`.
- Boolean operators `&&`, `||`.
- Unary operators.
	- Since there is only one operand there cannot be a larger and a smaller type.

## Illegal Operations

Some operations are illegal with integer types.
It is the responsibility of the programmer to eliminate all possibility of illegal operations.
I believe all of these trigger undefined behavior.

- Division by zero.
	- The integer types doesn't have a value representing infinity or NaN.
- Shift by more than the bit width.
- Multiplying or dividing (I think) the smallest signed integer by  -1.
	- Because the range of negative values is larger than the range of positive values. The smallest negative value doesn't have a corresponding positive value.
	- (So why is only multiply and divide illegal? Why not also unary minus? Or is that also illegal?)
- Multiplying two unsigned integers with size smaller than `int` with a result out of range for `int`.
	- This is because integers smaller than `int` are promoted to `int` before use, which means that the operation is subjected to all the limitations of signed integer arithmetic, including overflow being undefined behavior. This is problematic with multiplication because, on a platform where `int` is 32-bit, there are `uint16_t` values that when multiplied produces a result larger than `std::numeric_limits<int>::max`, and thus overflow, and thus undefined behavior.
	- So we cannot say "unsigned integers wrap on overflow" because that is not true for unsigned types smaller than `int` but large enough that a product between two values may overflow the range of `int`.


# Standard Library

Many containers in the standard library use `size_t`, which is an unsigned type, for indexing and counts.
We may chose to assume that the standard library designers knew what they were doing, and take this as a sign indicating that we should do the same, and that we should use unsigned integer when working with the standard library.
Not everyone agrees [(9)](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/)[(7)](https://google.github.io/styleguide/cppguide.html#Integer_Types), not even some of the standard library maintainers (`citation needed`) and committee members [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf).
It may be that it was the correct decision at the time, but things changed (e.g. 16-bit to 32-bit to 64-bit CPUs.) and now we are stuck with what we have for legacy / consistency / backwards compatibility reasons.
The design of any API that is difficult to change by definition happens before widespread usage of that API, and it is not until widespread usage we we discover real-world implications that wasn't though of beforehand.

Since the standard library containers use `std::size_t`, it is not unreasonable that our code using them also should.
By making our own container classes similar to the standard library containers we make them familiar to other C++ programmers [(18)](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc).

There is `ptrdiff_t std::ssize(const T& container)` that returns the size of the container as a signed integer.
It is undefined behavior to call this on a container with a size larger than `PTRDIFF_MAX`.
Such containers are uncommon.

I don't know of any way to index with a signed index, other than passing in the signed index and letting it implicitly convert to the unsigned equivalent or do the cast explicitly.
I've seen the following recommendations:
- Don't do direct indexing and instead use ranged base for loops,  iterators, named algorithms, ranges, ... (TODO More to add here?).
- Pass the signed value to `operator[]` [(5)](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es107-dont-use-unsigned-for-subscripts-prefer-gslindex).
	- This will work as long as the value is non-negative, so make sure you have a check for that or some other way to guarantee it.
	- This works as long as the the index you are trying to access isn't too large.
	- Disable the `sign-conversion` warning since every indexing will trigger it.
- Provide a short-named conversion function.
	- Such as `std::size_t toUZ(std::ptrdiff_t)`.
	- Use as `container[toUZ(index)]`.
	- This seems excessively tedious and noisy.
- Use a view that provide signed size and element access.
	- Possibly using `toUZ` and `fromUZ` internally.
	- `std::span` was meant to do this, but that was sabotaged by the committee.

`std::next` and `std::prev` uses a signed type.
This means that they cannot be used to step to the end of a very large container.

The standard containers will assert (Or throw, no sure which.) if an out-of-bounds index is passed to `operator[]` in debug builds.
If you want bounds checking in release builds then use `at` instead of `operator[]`.
Or see of your standard library implementation has a compiler flag to enable bounds checking in release builds.


# Note Conventions

While not guaranteed by the standard, it is assumed that `size_t` and `ptrdiff_t` have the same size.

We assume a 64-bit machine, i.e. 64-bit `intptr_t`, `size_t` and `ptrdiff_t`, with 32-bit `int`.
Some results are different on other machines.

We assume that signed integers are represented by two's complement, something that has only recently been required by the language standard.

In this note `integer_t` is an integer type that is an alias for either `size_t` or `ptrdiff_t` depending on if we use signed or unsigned indexing.
It is used in code snippets where the surrounding text discusses the various ways in which the code fails with either type.
The generic `integer_t` code snipped is often followed by a specific `size_t` and / or `ptrdiff_t` snippet with additional checks or other modifications.

In this note `Container` represents any container type that has `integer_t size() const` and `T& operator[](integer_t)` member functions, for example `std::vector` with `integer_t = size_t` .
In most cases it is assumed that valid indices for a container are in the range `[0, container.size() - 1]`, any exceptions are explicitly noted.

Many code snippets in this note represent actual work with a vaguely defined function named `work`.
It often takes a `Contaner` parameter and possibly also an index to work on.
If no index is provided then the function typically loops over the elements of the container.
The function may have a Boolean return value, if it performs error detection.
A simple example of a `work` function:
```cpp
void work(Container& container)
{
	for (integer_t i {0}; i < container.size(); ++i)
	{
		// Work with container[i].
	}
}
```
Many chapters start with a function with a narrow contract [(82)](https://quuxplusone.github.io/blog/2018/04/25/the-lakos-rule/), [(83)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3279.pdf) returning `void` and ends with one or more functions with a wide contract that returns `bool`.

In this note "mathematical results" means the result of an arithmetic operation interpreting all operands as values in ℤ, the infinite number line of integers in both directions.
I know that the ℤ/n ring is also "mathematics" but I don't care, it is not what we usually mean when talking about counts, sizes, and offsets, which are the subject for this note.

In this note "arithmetic underflow" refers to an arithmetic operation that should produce a result that is smaller than the smallest value representable by the expression's type.
For a signed type that is some large negative value.
For an unsigned type that is 0.

In this note "arithmetic overflow" refers to an arithmetic operation that should produce a result that is larger than the largest value representable by the expression's type.
Similar to underflow, but at the high end of the integer type's range instead of the low end.

These definitions are different from how these words are defined in the language specification since that text uses the ℤ/n definition for unsigned integers.

Signed numbers are not allowed to underflow or overflow, that triggers [[Undefined Behavior]].
Unsigned numbers are allowed to overflow and underflow, in which case they wrap around.
This is called modular arithmetic, which is basically what ℤ/n means.
From a language specification standpoint this isn't under- or overflow at all, it is just the way the unsigned types work.

Because overflow of signed integers is undefined behavior the compiler does not need to consider this case when generating machine code.
This can in some cases produce more efficient code and in some cases introduce bugs not visible in the source code as checks or entire code blocks are removed by the compiler.


# Dangers

It would be nice if we could pick any type we felt like and have things just work.
It would be nice if there were a single integer type that could hold all possible integer values.
Unfortunately, that is not how the language, or computer hardware, has been designed.
Things that lead to unexpected results include:
- Implicit type conversions resulting in unexpected values.
- Arithmetic operations causing under- or overflow resulting in undefined behavior.
- Arithmetic operations causing unexpected or unintended wrapping.
- Unexpected results when comparing signed and unsigned values.

This chapter describes and exemplifies the language behavior.
The effects of these dangers on code will be explored in the rest of this note.

When we say that some arithmetic operation produced an unexpected result we don't only mean that it is unexpected that fixed-width integers wrap at the ends of the type's range,
we may also mean that it was unexpected that this particular execution of that operation reached that end.


## Integer Promotions

C++ does all integer arithmetic using types that are at least as wide as `int`.
The conversions performed to achieve this is called integer promotions [(50)](https://eel.is/c++draft/conv.prom).
If you try to add, or do any other arithmetic operation on, two variables whose types are smaller than `int`, such as two `int8_t`, then the values will first be promoted to `int` or `unsigned int` if `int` cannot hold all values of the source type  [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/).
This makes expressions involving small types work as expected even when intermediate results are outside the range of the source type since intermediate results are allowed to go beyond the range of the original type without issue as long as they stay within the range of `int` or `unsinged int` [(51)](https://wiki.sei.cmu.edu/confluence/display/c/INT02-C.+Understand+integer+conversion+rules).
```cpp
int8_t c1 {100};
int8_t c2 {3};
int8_t c3 {4};
int8_t result = c1 * c2 / c3;
```
In the above example `c1 * c2` is larger than `std::numberic_limits<int8_t>::max()`, but that's OK since it is smaller than `std::numeric_limits<int>::max()`, and the actual computation is done with `int`.
The division by `c3` brings the value down into the range of `int8_t` again and all is well.

This can be unexpected for people who think they know how limited range unsigned integers and wrapping works.
Consider the following example that adds two unsigned 8-bit integers [(87)](https://youtu.be/Iz2UOgLMj58?t=1148).
```cpp
auto add_uint8(uint8_t lhs, uint8_t rhs)
{
	return lhs + rhs;
}

void work()
{
	auto sum = add_uint8(255u, 1u);
}
```
We may think that `add_uint8` should return `uint8_t` and that `255u + 1u` should wrap back to 0 since 255 is the largest possible unsigned 8-bit number.
This is not what happens, since addition is always done with at least `int`.
So the program actually looks like this:
```cpp
int add_uint8(uint8_t lhs, uint8_t rhs)
{
	return static_cast<int>(lhs) + static_cast<int>(rhs);
}

void work()
{
	int sum = add_uint8(255u, 1u);
	// Sum is 256.
}
```

Integer promotion happens even when the compiler knows that the receiver of the evaluated expression is of the same type as the operands to the operator.
Consider the following variant of the `add_uint8` function where we have change the return type from `auto` to `uint8_t`.
```cpp
uint8_t add_uint8(uint8_t lhs, uint8_t rhs)
{
	return lhs + rhs;
}
```
This is compiled as
```cpp
uint8_t add_uint8(uint8_t lhs, uint8_t rhs)
{
	return static_cast<uint8_t>(static_cast<int>(lhs) + static_cast<int>(rhs));
}
```
Some compilers may warn that the implicit conversion from `int` to `uint8_t` may change the value.
That is, the following code is not type correct, for some strong definition of "correct", since implicit conversions happen.
It is well defined, so it is not wrong.
```cpp
uint8_t add_uint8(uint8_t lhs, uint8_t rhs)
{
	return lhs + rhs;
}
```

This conversion only happens up to the size of `int`.
If you do the same operations where `c1`, `c2`, and `c3` are all `int` and the initial values chosen so that the `c1 * c2` multiplication overflows then your application is toast, as you have triggered undefined behavior.

Integer promotion doesn't require two operands, it applies also to unary operators as well.
Consider
```cpp
uint8_t a {1}; // 0000'0001.
~a; // We might think this is 1111'1110.
    // But it is actually 1111'1111'1111'1111'1111'1111'1111'1110.
```


## Same Sign-ness But Different Sizes

(
I think I mean to write something about truncation here.
Or widening `int32_t` → `uint32_t` → `uint64_t`.
)


## Mixing Signed And Unsigned Integer Types

C++ has counter-intuitive and mathematically incorrect implicit conversion rules [(70)](http://ithare.com/c-thoughts-on-dealing-with-signedunsigned-mismatch).
Arithmetic operations are always performed on two values of equal type [(49)](https://eel.is/c++draft/expr.arith.conv).
One source of bugs is when signed and unsigned values are mixed in an expression [(7)](https://google.github.io/styleguide/cppguide.html#Integer_Types), [(12)](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities), [(15)](https://youtu.be/Puio5dly9N8?t=2561), [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU), [(41)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1089r2.pdf). [(45)](https://stackoverflow.com/a/18796546), [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity).
This leads to implicit conversions and results that are difficult to predict for many programmers.
If two values with the same size but different signed-ness are passed to an operand then the compiler will implicitly convert the signed value to unsigned [(49)](https://eel.is/c++draft/expr.arith.conv).
The problem is that C++ doesn't use the values two variables have when an operator is given variables of different types.
Instead integer promotion [(50)](https://eel.is/c++draft/conv.prom) and the usual arithmetic conversions happen [(49)](https://eel.is/c++draft/expr.arith.conv).
No check is made to ensure that the signed value is representable in the unsigned type [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
This is unfortunate, a compiler error would have been better [(68)](https://github.com/ericniebler/stl2/issues/182).
These conversions are called the usual arithmetic conversions [(48)](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions).

One reason for why mixing signed and unsigned integers in C++ is risky is because of unfortunate implicit conversion rules [(46)](https://stackoverflow.com/a/18248537).
Consider
- `unsigned - unsigned` which produces an unsigned result, and
- `signed + unsigned` which also produces an unsigned result (except if the signed integer type has higher range than the unsigned type).

If we take "unsigned" to mean "cannot be negative", i.e. "is positive", we can write
`positive - positive is positive` and `signed + unsigned is unsigned`.
Both of these are logically false.
Subtracting two positive values can produce a negative value,
and adding a positive value to a negative value may result in another negative value.

If you multiply an `int` and an `int64_t` then the computation will be performed using `int64_t` because `int64_t` is wider and wider wins.
If you multiply an `int` and an `unsigned int` then the computation will be performed using `unsigned int` because they are the same size and unsigned wins.
This can wreck havoc with your application since the usual arithmetic conversions happens not only for addition and multiplication and such, but also for comparison operators.

### Surprisingly, -1 > 1

That is, -1 is greater than 1 if 1 is unsigned [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU), [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/), [(70)](http://ithare.com/c-thoughts-on-dealing-with-signedunsigned-mismatch).
This is surprising to many, and arguably bad language design.
```cpp
int small {-1};
unsigned int large {1};
if (small < large)
{
	// This will never execute even though in the real world
	// -1 is smaller than 1, because -1 is actually 4294967295
	// in this context.
}
```

So why is -1 not less than 1?
What happens, in this case, is that the signed variable, which has a low value, is converted to the type of the unsigned variable [(49)](https://eel.is/c++draft/expr.arith.conv).
Since the signed variable has a value not representable in unsigned type we get "garbage".
Not actual garbage, the value we get is well defined, but in many cases it is indistinguishable for garbage since we got a completely different value.
The value we get is a very large one because the negative signed value had the sign bit set and that bit represents some large value in an unsigned type.

Expressed another way, in the following `compare_small_and_large` will return `true`.
```cpp
bool compare_small_and_large()
{
    int small {-1};
    unsigned int large {4294967295};
    return small == large;
}
```

### Guard Against Invalid Indices

Consider the following code that guards against invalid indices.
```cpp
void work(Container& container, integer_t index)
{
	if (index >= container.size())
	{
		report_error("Index passed to work is out of range for the container.");
		return;
	}

	// Work on container[index].
}
```

When `integer_t` is a signed type the `>=` operator is given the signed `index` parameter and the unsigned return value or `data.size()`.
Assuming the two types are the same size, the signed `index` is converted to the unsigned type of `data.size()` [(12)](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities).
If `index` has a small to moderately large negative value then the result of the conversion is a very large positive value which often will be larger than the size of the container and we enter the error reporting code block.
However, if `index` is very negative and the container is very large then we might wrap back into the range of valid indices and perform work on a container element we shouldn't.

Some say [(1)](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/comment/lfymu23/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button) , including the C++ Core Guidelines [(3)](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic), that this is not a problem, that we should pass `-Wno-sign-conversion` to the compiler, ignore the problem, and hope that it never happens.
This is OK if we take care to eliminate all cases where a negative value can be converted to an unsigned integer type.
I don't know how to ensure that.
I would prefer to keep the sign-conversion warning enabled, but I'm not sure that is realistic with an signed `integer_t` type when using container types with unsigned sizes and indices such as the standard library.

Let's consider what happens with the guard against invalid indices code above if `integer_t` is unsigned, but we call it with a negative value, i.e the call size looks like this:
```cpp
void work_on_element(Container& data)
{
	integer_t const index {-1};

	// This will log the error message even though index is not larger
	// than data.size().
	work(data, index);
}
```
When `integer_t` is an unsigned type the conversion from a negative value to a positive value happens in the initialization of `index` in `work_on_element` instead of when evaluating `>=` in `work`, but the effect is the same.
The difference is the lie in the signature of `work` in the signed `integer_t` case.
The function claims to handle negative values when in reality it does not.

If we want to use a signed type then we can use `std::ssize(container)` instead of `container.size()` to get the size, which will give us the size as a signed type.
In that case there will be no sign conversion and the comparison will work as expected, i.e. `-1 >= std::ssize(data)` will always evaluate to false.

When using singed `integer_t` the range check in `work` should check both sides:
```cpp
if (index < 0 || index >= std::ssize(data)) { /* Report error. */ }
```

Another option is to flip the test.
Instead of detecting the error case we detect the valid case.
```cpp
void work(Container& container, ptrdiff_t index)
{
	if (index < container.size())
	{
		// Work with container[index].
	}
	else
	{
		report_error("Index passed to work is out of range for the container.");
	}
}
```
The effect is the same.

In some cases the index we are guarding isn't a simple index parameter to be compared with a container size.
Sometimes the index is computed and the threshold we are comparing against is another computed index.
Consider the following example where unsigned integers are used for indices and sizes, as some suggest we do, while the value that can be negative, `block_offset`, uses a signed integer.
The intended behavior of the function is to process a block of elements of `container` where the position of the block relative to a `base_index` determines the type of processing to perform.
The `transition_offset` parameter controls which blocks need which type of processing.
Blocks starting below the transition index need one type of processing while elements at or above that index need another type of processing.
```cpp
/**
                    base_index             block_offset
                        v                       3
|-------|-------|-------|-------|-------|-------|-------|
block_size              |---------------|
    8                      transition
                           offset 16
     These need one type of processing <|> These need another type of processing.
*/
void work(
	Container& container,
	// We are responsible for the neighborhood around base_index.
	unsigned int base_index,
	// The index, relative to base_index, where the processing type changes.
	unsigned int transition_offset,
	// Which block, relative to base_index, we should process.
	int block_offset,
	// The size, in container elements, of each block.
	unsigned int block_size)
{
	// Convert the block offset into an element offset.
	int element_offest {block_offset * block_size};

	// Determine which type of processing we should perform.
	if (element_offset < transition_offset)
	{
		// Code for processing blocks before the transition.
	}
	else
	{
		// Code for processing elements at or after the transition.
	}
}
```

The if statement condition is essentially the same as the common `index < container.size()` that we know to be weary of when `index` is a signed integer, but the problem is easier to miss since the pattern is unfamiliar.
The problem happens when `element_offset` becomes negative because `block_offset` is negative.
It is not actually a problem for `element_offset` to be negative, it is not as if we are using it to index directly into the container.
But the check still fails because `negative < unsigned` is always false, i.e. we will incorrectly enter the `after the transition` block.

### Unsigned Values Can Taint Signed Expressions

An example [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned) where a single unsigned value taints an otherwise signed expression, causing an incorrect result:
```cpp
int revenue {-5};            // Can be negative, i.e. a loss, so signed.
unsigned int taxRefund {3};  // Cannot be negative, so unsigned.
std::cout << "total earnings: " << revenue + taxRefund << std::endl;
```
Output:
```
total earnings: 4294967294
```

We get a very large value because in the addition we mix signed and unsigned.
The signed `revenue` is converted to unsigned and since it is negative we get a very large value.
It is converted to a value that is 1 lower than `std::numeric_limits<unsigned int>::max()`.
1 lower because -1 is converted to the largest possible value and -5 is four smaller than -1.
Then 3 (`taxRefund`) is added and we end up 1 (4 - 3) away from the max.
To get the correct result we need to explicitly convert all unsigned value to signed every time they are used in an expression that also includes signed values.
The following prints `-2`, as expected.
```cpp
int revenue {-5};            // Can be negative, i.e. a loss, so signed.
unsigned int taxRefund {3};  // Cannot be negative, so unsigned.
std::cout << "total earnings: " << revenue + static_cast<int>(taxRefund) << std::endl;
```

Remember this rule: Explicitly convert all unsigned integers to signed when used in an expression with possibly negative values.

### No Sign Extension On Unsigned To Signed Widening

An example of an unintended underflow with wrapping and an implicit type conversion with unexpected result is the following [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
Here the programmer expected a very large unsigned value, i.e. `uint32_t(-1)`, to become an actual -1 when converted to a signed type.
```cpp
uint32_t a {10};
uint32_t b {11};
int64_t c = a - b;
```
We may expect `c` to have the value -1 since we have used the signed `int64_t` type.
However, we get the value `std::numeric_limits<uint32_t>::max()` instead, i.e. only half of the bits in `c` is set.
The problem is that `(a, b)` and `c` has different sizes.
`a - b` is computed to be a very large 32-bit value that is then widened to the 64-bit type.
Since `a - b` is an unsigned type no sign bit extension will be performed, all the upper bits will be set to 0.
As a last step the 64-bit unsigned value is converted to a signed 64-bit value, which is a no-op at the bit-level.
Had all values been the same size then `c` would have the value -1, as we wanted, since the highest bit would have been set in the unsigned value and that bit would become the sign bit in the signed value, signaling a negative value.

### Changing The Size Of A Variable Can Change The Sign Of An Expression

Since the sizes of the types matter, changing the size of a variable can have a big influence on code using that variable.
Let's look at two code snippets that use the same values but despite that produce different results [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity).
The code snippets computes an index using a base index and an offset and then checks the computed index against a container size.

The first expression:
```cpp
int base = 12;
unsigned short offset = 25;
int size = 32;
if (base - offset < size)
{
	// This is true since 12 - 25 = -13, which is less than 32.
}
```
The above behaves as one would expect since the promotion rules change the small unsigned `offset` to a signed `int` in the computation.

But change a single type to a wider one and the semantics changes completely.
In the code snippet below, where `offset` is `int`-sized instead of `short`-sized, we get an usual arithmetic conversion [(48)](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions) [(49)](https://eel.is/c++draft/expr.arith.conv) instead of integer promotion:
```cpp
int base = 12;
unsigned int offset = 25;
int size = 32;
if (base - offset < size)
{
	// This is false since 12 - 25 = 4294967283, which greater than 32.
}
```


The same happens if we make `size` unsigned instead:
```cpp
int base = 12;
int offset = 25;
unsigned int size = 32;
if (base - offset < size)
{
	// This is false since 12 - 25 = 4294967283, which is greater than 32.
}
```
This variant is particularly insidious if the `size` value is a function call since in that case ALL values we declare are signed and a signed left hand side is computed for the comparison, but then the value is implicitly converted to unsigned and destroyed in the process.
```cpp
int base = 12;
int offset = 25;
if (base - offset < container.size())
{
	// This is often false since, surprisingly,
	// 12 - 25 = 4294967283
	// (Even though all our variables are signed.)
	// which is greater than most container sizes.
}
```
To avoid this problem use `std::ssize`, which returns a singed integer, instead of `Container::size`.

Make another type change and we get the expected result again:
```cpp
int64_t base = 12;
int offset = 25;
unsigned int size = 32;
if (base - offset < size)
{
	// This is true since 12 - 25 = -13, which is less than 32.
}
```
In this case it is the unsigned right hand side, `size`, that is converted to the type of the left hand side, `base - offset`, since the left hand side has a larger bit width.
Alas, if we use `container.size()` instead of `unsigned int size` for the right hand side then we have two 64-bit values and in that case the unsigned value wins and we compare the size against a very large positive value again.

What I'm trying to communicate here is that it is not obvious what will happen when we compare two values, especially if we don't know what types are involved or if the types may be changed.
A real-world example may be a line like the following [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity):
```cpp
if (source_index - findTargetIndex() < getSize())
```
It is not obvious what the types are here, or what implicit conversions will happen.

This type of implicit conversion become even more difficult to keep track of when using typedef'd integer types and the application support multiple  platforms possibly with different integer sizes.


### Unwittingly Fixing Conversion Warnings - Don't Cast Just Because The Compiler Says You Should

A problem with implicit conversion warnings is that it is not unusual that programmers try to fix them without understand the details of the code and the implication of a change [(70)](http://ithare.com/c-thoughts-on-dealing-with-signedunsigned-mismatch).
Difficult to give an example because the issues tend to only happen in larger non-trivial codes since the mistake is obvious in isolation.
A real world case was pieces of code from `unrar` being incorporated into Windows Defender, but with signed integers changed to unsigned which caused a security vulnerability [(72)](https://www.theregister.com/2018/04/04/microsoft_windows_defender_rar_bug/).

A guideline is to never add a cast just because the compiler says you should.
If you do add a cast from signed to unsigned then also add a check for non-negativity and decide what to do in the case where the value is negative.
If you do add a cast from unsigned to signed then also add a check for too large values and decide what to do in the case where the value is too large.

### Avoiding Mixing Signed And Unsigned Values

Assuming we are required to use signed values for some variables, some data is inherently signed [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), it follows that we want to also use a singed type for any value that is used together with the inherently signed data.
This process repeats and a front of the signed-ness spreads like a virus across the code base until everything, or at least most things, are signed.
Every time we introduce a variable with an unsigned integer type we risk introducing mixing the newly introduced unsigned variable with the must-be-signed values we already have and thus introduce the possibility of bugs.

Unfortunately the reverse of this is also true since the standard library uses a lot of unsigned types in its containers.
This means that a similar front of unsigned-ness spreads in the opposite direction, outwards from any code using such containers.
Trouble and headaches happen where these fronts meet.
Unless we chose to use a different set of containers that use a signed integer type instead.

A goal should be to keep this front of mixed signed-ness as small and manageable as possible.

Since we cannot, in many applications, avoid negative, and thus signed, integers it is not unreasonable to confine the unsigned types as much as possible.
As soon as we are given an unsigned value we check that it isn't too large and then cast it to signed.
As soon as we need to supply an unsigned value somewhere we check that the signed value we have isn't negative and then cast it to unsigned [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).

A counter-point is that if a function is so large that it becomes difficult to track which variables are signed and which are unsigned then the function should be split anyways (`citation needed`).
Trying to force everything to the same signedness is a false sense of security.

A counter-point to that is that as programmers we absolutely do not want to discard whatever we currently have in our own working memory, for example while debugging, to start analyzing the impacts of possible implicit conversions, sign extensions, and wrapping behavior.
We want code to do what it looks like the code is doing.
We cannot memorize an ever-growing number of tricks and idioms to make loops using unsigned counters work correctly in all edge and corner cases.




### CONTINUE HERE


Because of the implicit conversion rules unsigned values tend to contaminate the arithmetic in mixed signed/unsigned expressions [(45)](https://stackoverflow.com/a/18795564).
```cpp
void work(Container& container, std::ptrdiff_t a, std::ptrdiff_t b)
{
	if (container.size() - (a + b) >= 0)
	{
		// a + b seems to be smaller than the container size.
		// All variables we declared are signed because we want
		// signed arithmetic.  However, because the size_t from
		// size()tainted the expression we end up with an unsigned
		// value and a tautology conditional.
	}
}
```

One place where the type is not visible is in functions calls.
At the call site we cannot see the type of the parameter.
This means that we can unknowingly pass a signed integer that is implicitly converted to an unsigned one.
It is is difficult for the called function to detect if large positive values are also valid use cases.
We can chose to enable compiler warnings for this, e.g. `-Wsign-conversion`, but some advocates for using signed integers advice against enabling this warning in order to make interacting with the standard library easier [(5)](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es107-dont-use-unsigned-for-subscripts-prefer-gslindex).
They want to following code, where `container[index]` contains an implicit signed → unsigned conversion, to be warning-free:
```cpp
void work(Container& container)
{
	for (ptrdiff_t index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

While the above loop is safe, `container.size()` can never be larger than `std::numeric_limits<ptrdiff_t>::max()`, the same is not true for other loop counter types.
For example, the following may lead to integer overflow and undefined behavior [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).
```cpp
void work(Container& container)
{
	for (int index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```
The implicit conversion rules may "save" us in practice here, depending on what the compiler chooses to do.
An `int OP size_t` expression converts the signed `int` to an unsigned `size_t` which means that the wrap-arounded -1 `int` value is turned into an unsigned value, 2147483648, and then expanded to the width of `size_t`.
Assuming the container isn't larger than `std::numeric_limits<unsigned int>::max()`, eventually the unsignified negative `int` reaches the last element of the container and the loop stops.

Negative values often appear from subtractions and it can be non-obvious that the left hand side may be smaller than the right hand side [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf).
```cpp
std::size_t area(std::size_t width, std::size_t height)
{
	return width * height;
}

void work(
	std::size_t width1, std::size_t height1,
	std::size_t width2, std::size_t height2)
{
	std::size_t area = area(
		width1 - width2,
		height1 - height2);
}
```

 Loops over small or empty container are easy to get wrong when the container's size is an unsigned value, even when we chose a signed loop counter.
 The following fails when passed an empty container.
 ```cpp
 void draw_curve(const Container<Point>& points)
 {
	 for (std::ptrdiff_t index = 0; index < points.size() - 1; ++index)
	 {
		 draw_line(points[index], points[index + 1]);
	 }
 }
```
The intention is that the `points.size() - 1` upper bound ensures that when we enter the loop body we know that we have one extra point to use as the end point for the line being drawn.
When the container is empty the first loop iteration condition expression is
- `index < points.size() - 1`
- `0 < 0 - 1`
- `0 < -1`
- `0 < std::numeric_limits<std::size_t>::max()`
- `true`

This expression will be true for many values of `index`, as it makes its way from zero towards 18 quintillion, generating lines from whatever data it happens to find in memory until the application, most likely, crashes, possibly leaking all manners of secrets to whomever knows how to listen. Possibly the same person who deliberately sent an empty list of points to `draw_curve`.

To fix this we can either let `index` start at 1 and look backwards instead of starting at 0 and look forwards.
Then the `- 1` in the condition isn't needed anymore and the wrap, and the problem, goes away.
Another way is to do the arithmetic using a sane integer type instead.
Either be letting the `Container::size` member function return a signed type or by casting it to signed as soon as possible [(46)](https://stackoverflow.com/a/18248537).
```cpp
void draw_curve(const Container<Point>& points)
{
	if (isTooLarge(points, "points", "draw_curve"))
		return; // Prevent overflow in num_points.

	const std::ptrdiff_t num_points = static_cast<std::ptrdiff_t>(points.size());
	for (std::ptrdiff_t index = 0; index < num_points - 1; ++index)
	{
		draw_line(points[index], points[index + 1]);
	}
}
```

(
What follows is me speculating and writing without thinking, it's draft text.
)

Mixing `while (unsigned < signed)`, the opposite of the common case, is safer that `while (signed < unsigned)`, the common case, when the right hand side is guaranteed to be positive, e.g. the return value from a `size()` function.
That is because all possible positive signed values can be converted to the corresponding value of an equally sized unsigned value.
At the bit level it's not even a conversion, the bit representation is exactly the same.
This means that if we use signed integers for our sizes and indices then the users can freely choose if they want to use signed or unsigned indices.
The opposite is not save, which is the whole reason for this note to exist.

(
End of draft text.
)

Since unsigned values are often implicitly converted to unsigned values when the two are mixed we can get surprising results like this [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU):
```cpp
signed int a {-1};
unsigned int b{1};
a < b // Evaluates to false because a is converted
      // to unsigned and wraps in the process.
```

We can show that integers doesn't form a transitive set under an ordering operator (wording?) with the following setup  [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
```cpp
int i {-1};
int j {1};
unsigned k {2};

i < j; // True.
j < k; // True.
k < i; // True.
```

When the index comes from a non-trivial arithmetic expression - each type conversion incurs additional cost, be it in the form of additional runtime checks, reduced code clarity or a risk of semantic mistakes. So, for this scenario, there is an objective preference: we want to use the same type for indexes as is used for the majority of arithmetic expressions that generate them [(21)](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699).

Signed and unsigned integer types have different ranges so every time we convert between them we risk accidental overflow [(62)](https://news.ycombinator.com/item?id=29766658).

Since there are a number of bugs stemming from unintended conversions between signed and unsigned integer types [(10)](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/) many compilers include warnings for this, often enabled with one or more of
- `-Wconversion`
- `-Wsign-conversion`
- `-Wsign-compare`


Martin Beeger summarizes the mixing issue well  [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post15)

> ... the unsigned-signed promotion and comparison rules are just a as
> big hazard. We should never be forced to really think about them.
> This is possible through consistent use of one type of indexes, sizes,
> slices etc. And negative slices and strides have well-defined meaning
> and should be allowed, so they must be signed. Deciding on a
> case-by-case basis forces peoples into all the nitty gritty details of
> integer promotion and comparision, which is exactly what we should IMHO
> protect users from.

Some of the issues described above can be avoided by using the `cmp_*` family of functions [intcmp @ en.cppreference.com](https://en.cppreference.com/w/cpp/utility/intcmp).
## Integer Over- / Underflow And Wrapping

Wrapping is the act of truncating away the bits beyond those supported by the type we are using.
This makes the value jump from one end of the value range to the other.
If you add 1 to the largest value an unsigned integer type can hold then the value doesn't become one larger, it wraps around back to zero.
Similarly, if you subtract one from an unsigned zero you don't get -1, instead the value wrap around to the largest value the type supports.
This violates our intuition of regular arithmetic [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/), especially when it happens with intermediate results.
There is a discontinuity in the value range where we teleport from one place on the number line to another.
This means we can no longer depend on regular algebraic rules.
```cpp
int32_t a = -2;
uint32_t b = 4;
int32_t c = 2;
int32_t d = (a - b) / c + 10;
```
Let's evaluate the above using regular algebraic rules.
- (-2 - 4) / 2 + 10
- (-6) / 2 + 10
- -3 + 10
- 7

Unfortunately, this is not what happens.
The unsigned `b` taints the entire expression due to the usual arithmetic conversions.
So `(a - b)` isn't -6, it's 4'294'967'293.
And the rest of the computation is just garbage.
It doesn't help that `b` represents a value that "can never be negative", it still causes problems.

This types of weirdness can limit the optimizer [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/).
See _Compiler Optimization_ > _Algebraic Expression Manipulation_.

Signed integer behaves as most people expect number to behave most of the time.
`x - y` is negative when `y > x`.
`x - 1` is less than `x`  for pretty much every value that will show up in practice,
but it is not true for the most common unsigned number: 0.

While the wrapping behavior of unsigned integers can be surprising and often not what we want, it is at least well-defined and we can reason about the results produces by a misbehaving program [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).
With well defined behavior for the under- and overflow after a bug had been identified it is possible to read the C++ code and understand what happened [(66)](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/#comment-487024).
However, even though the computation of the bad index isn't undefined behavior, using it may be, depending on what it is being used for.
Indexing into an array or `std::vector` would be undefined behavior if the index is out of bounds for the container.

The situation is arguably worse with signed integers [(66)](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/#comment-487024).
If you add beyond the largest value or subtract beyond the smallest value on a signed integer type then you get undefined behavior [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/) instead of wrapping.
This means that the compiler is allowed to optimize based on the assumption that it never happens.
With signed integer that is not possible since under- and overflow is undefined behavior.
Anything can happen.
We need to read assembly code from the binary to learn what our program does.

This means that signed integers follow the regular algebraic rules.
A value cannot suddenly teleport from one place on the number line to another.
`x + 1` is always larger than `x`.
`(a - b) / c` is always equal to `(a / c) - (b / c)`. (Is this true?)
"Always" meaning "for all applications that follow the rules".
Make sure you follow the rules.

More algebra that holds for signed integers but not unsigned integers [(62)](https://news.ycombinator.com/item?id=29769851):
```cpp
a - b > c
a - b - c > 0
a - c > b
```

This can be useful if we know that `a - c` will not under- or overflow, but we are not sure about `a - b`.

These are all equivalent with signed integer but not with unsigned ones [(69)](https://github.com/kryptan/rect_packer/issues/3):
```cpp
a - b > 5
a > b + 5
a - 5 > b
a - b - 5 > 0
```

Using the type that is more similar to our intuition of how numbers work makes it faster to teach new programmers, and even intermediate programmers will make fewer mistakes (citation needed).

Based on the above, the guideline for deciding if a variable should be signed on unsigned is 

> Is it an error for this value to over- or underflow?

If yes, then use a signed integer type [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/).
If you use an unsigned integer instead and accidentally wrap then you will get well-defined silently incorrect behavior that can be difficult to detect.
See _Detecting Error States_ > _Detecting Overflow_.

Small negative numbers are more common than very large positive numbers.
Mixing signed and unsigned numbers adds even more surprising behavior [(41)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1089r2.pdf), [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
See _Mixing Signed And Unsigned Integer Types_.

With a signed integer type we can use tools such as sanitizers [(55)](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) to detect and signal signed integer over- and underflow in our testing pipeline.
This won't detect cases in production though, which is a cause for concern, since we usually don't ship binaries built with sanitizers enabled [53](https://lemire.me/blog/2019/05/16/building-better-software-with-better-tools-sanitizers-versus-valgrind/).
We can also compile with the `-ftrapv` flag to catch signed under- and overflow [(54)](https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html), though this comes with a performance cost.
We can also compile with `-fwrapv` to eliminate the undefined behavior and get wrapping behavior also with signed integers.

We must ensure we never cause an overflow when computing indices or buffer sizes [(37)](https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap).
It doesn't matter, from a correctness point of view, if the result is defined by the language or not, it is still wrong.

## Unexpected Arithmetic Results With Unsigned Values

A common way to detect odd numbers is `n % 2 == 1`.
This does not work for negative `n` since the result is `-1` instead of `1`.
Use `n % 2 != 0` instead.

Shift operators behavior can be unexpected for negative numbers.
(
TODO Description and examples here.
)

Given a listing of all operators on signed and unsigned integers a larger fraction of them can produce undefined behavior when operating on signed integers compared to the fraction of operators that can produce wrap around when operating on unsigned integers [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2643).
There are more things that can go wrong with signed integers.
This is because when using two's complement representation of signed integers we have one more negative number than we have positive ones.
We can't negate the most negative value because there is no corresponding positive value.
Which means that we also can't take its absolute value, i.e. `std::abs(singed)` is not a safe operation.
The same is true for dividing by or taking the remainder of the most negative number and -1.

The following identities does not hold with signed integers [(35)](https://lemire.me/blog/2017/05/29/unsigned-vs-signed-integer-arithmetic):
- `a/b = sign(b) * a / abs(b)`
- `a * b = sign(a) * sign(b) * abs(a) * abs(b)`


## Changing The Type Of A Variable Or Return Value

Without warnings is can be difficult to find all the places where changing the type of a variable or function's return type changes the behavior of the code that uses it.
IDE's and other tools can help within the same project but projects that use our code is more difficult.
The developers on that project may not even be aware that the type changed.

# Operations

This chapter lists some common operations we may want to perform without going into any detail about the challenges and pitfalls associated with them.

- Loop over container.
- Test if an index is valid for a container.
- Compute an index with a non-trivial expression.
- Loop over container backwards.
- Detecting error states.

# Common Bugs

This chapter lists some common bugs that we should look out for when reading or writing code.

- Incorrect type of loop control variable.
- Unexpected conversion from signed to unsigned.
- Changing the type of a variable, constant, or return value.
- Unintended underflow or overflow.
- Performing an illegal operation.

# Representing A Value That Cannot Be Negative

Using unsigned is a natural choice when working with non-negative quantities such as indices and counts [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), [25](https://graphitemaster.github.io/aau/). [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/). [(68)](https://github.com/ericniebler/stl2/issues/182).
It clearly signals the limitation on the domain of the value.
It doesn't make sense for a container to have a negative size so we should use a type that encodes this into the type system.
If we use a signed type then we force the user to consider what it would mean for a size or index to be negative, and what to do in that case.

Taking it one step further.
Proponents of signed integers say that unsigned integers doesn't model natural numbers, instead they model the ℤ/n ring, which is another concept (citation needed).
As programmers we don't need to care about that unless we chose to, i.e. unless we chose to write our code to take wrapping into account and either exploit or prevent it.
As an alternative, we can pretend that wrapping isn't a thing if we place the requirement on the application's users that they may never provide input that causes any value to wrap.

(I know nothing about torsors, this is a my simplified explanation of the descriptions I've seen.)
Another way to view the set of unsigned values is to relate them to torsors [(58)](https://math.ucr.edu/home/baez/torsors.html), [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
A torsor is a value that encodes some quantities relative to some arbitrarily chosen reference point.
A torsor is the opposite of an absolute value.
For any measurement we can chose a reference point so that the measured value is positive.
When talking about indices into a container we note that they are not torsors, there is a well-defined location that the index is relative to: the start of the container [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
`std::ptrdiff_t` on the other hand, represents the subtraction between two indices, which is a torsor describing the location of the first index relative to the second.
That is why the index type `std::size_t` is unsigned and the signed torsor variant `std::ptrdiff_t` is used to represent differences between indices.

Restricting what values can be passed to a function through the type system is a good way to communicate how the function is meant to be used to both programmers and the compiler [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU), [(73)](https://news.ycombinator.com/item?id=2364065).
It simplifies the written documentation required.

At least it would be good if we didn't have implicit signed → unsigned conversions [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
And if arithmetic over- and underflow was an error instead of wrapping.
Passing a negative signed value into a function taking an unsigned parameter is a common source of bugs [(15)](https://youtu.be/Puio5dly9N8?t=2561),
making the parameter an unsigned integer type doesn't protect us from that unfortunately, it simply hides it [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
Expressions that should have resulted in a negative value instead becomes a positive value, i.e. it conforms to the restrictions of the interface even though it really should not.
Using a signed type reveals the bug instead [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU), which is better.

By definition, `std::size_t` is a type large enough to hold the size of any object.
Therefore it is the type returned by `sizeof()`.
Therefore, any time you wish to work with the size of objects in bytes, you should use `std::size_t` [(18)](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc).
For consistency, we should also use `std::size_t` for other sizes, such as the number of elements in a container.

An unsigned integer can address twice as many container elements as an equally-sized signed integer can [(33)](https://stackoverflow.com/questions/10040884/signed-vs-unsigned-integers-for-lengths-counts), [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java), [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/).
If you don't need a sign then don't spend a bit on it.
When no negative numbers are required, unsigned integers are well-suited for networking (I have a source for this, but forgot to add it h ere.) and systems with little memory, because unsigned integers can store more positive numbers without taking up extra memory.
The extra range may be important when the index type is small [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), [(18)](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc), such as 16 or possibly even 32-bit in some cases.
On 32-bit machines we don't want to be limited to 2 GiB `std::byte` buffers since we have up to 4 GiB of address space.
This is not a problem for element types larger than a single byte since by then even a signed integer is able to address all of memory due to the size multiplication.

In many cases we can use a larger signed type instead of a small unsigned type to get more range.
The signed option gives even more range [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

I'm not sure when only having half the range will become a problem for 64-bit signed indices, the largest `std::ptrdiff_t` value is 9'223'372'036'854'775'807.
For most modern applications on modern hardware the extra bit is not necessary [(15)](https://youtu.be/Puio5dly9N8?t=2561), the range limitation does not come up in practice very much.
The limitation only comes into effect when the container contains single-byte elements such as char,
with any larger type we run out of addressable memory for the data before we run out of index values in a signed integer.
Unsigned are sort of legacy of small bit sized machines where the extra bit mattered for range [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).

If we chose a signed integer instead then we need to motivate the loss of maximum size.

However, having the extra bit does not mean it is actually used [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).
At least [GCC's standard library implementation of `st::vector` is limited](https://github.com/gcc-mirror/gcc/blob/releases/gcc-11.4.0/libstdc%2B%2B-v3/include/bits/stl_vector.h#L1776) to the range of `std::ptrdiff_t`, and the same is stated under _Note_ on [cppreference.com/vector/max_size](https://en.cppreference.com/w/cpp/container/vector/max_size).
Maybe `std::deque` can handle larger container sizes since it doesn't use a continuous memory buffer for all elements.


They model modular arithmetic [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), [(45)](https://stackoverflow.com/a/18796084).
The arithmetic rules are different [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity) from regular mathematics.
It models the ℤ/n ring, not the natural numbers.
Which is weird for a container size since there is no sane situation where adding more elements to a container suddenly makes it empty (Forgot to add citation here.).
Decreasing an unsigned value doesn't necessarily make it smaller since it may wrap around [(45)](https://stackoverflow.com/a/18795568), and similar for increasing it not necessarily making it larger.
This is true, in some sense, for both signed and unsigned integers, but for signed integers the point where that happens far away from commonly used numbers while for unsigned integers it is right next to the most common number: 0 [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
Unsigned under/overflow has unexpected / nonintuitive behavior, which can easily leads to bugs, so it's also bad (even though it's well defined).
For singed integers under- and overflow is undefined behavior, i.e. it must never happen.
With unsigned integers neither we as programmers nor the compiler can assume that `x - 1 < x < x + 1`.
Since under- and overflow is undefined behavior with signed integers the inequalities can be assumed to hold for them.

An unsigned type is not a good way to maintain the invariant [(68)](https://github.com/ericniebler/stl2/issues/182) that a value should never be negative, the only thin it does is to force the value to be non-negative regardless of whether that makes sense as a result for a particular computation or not.

Do not use an unsigned integer type if you ever need to do subtraction [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), too often it silently does the wrong thing [(62)](https://news.ycombinator.com/item?id=29766658).

The wrapping behavior of unsigned integers helps for some applications, but not if you have high requirements on correctness, safety, and security since accidental wrapping is a common source of bugs and security vulnerabilities (citation needed).
See also _Advantages Of Signed_ > _Underflow Is Farther Away From Common Numbers_.

Unsigned integers have surprising conversions to/from signed integers, see _Dangers_ > _Mixing Signed And Unsigned Integer Types_.

[The idea that 'signed types are safer' is nonsense](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/comment/lfzonsz/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button).

# API Design
## Wide Contracts

A function with a wide contract is one that behaves correctly for all possible inputs [(82)](https://quuxplusone.github.io/blog/2018/04/25/the-lakos-rule/), [(83)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3279.pdf) .
The alternative is a narrow contract, which means that only a subset of all possible inputs are legal and passing illegal inputs may lead to the program misbehaving.

It is easier to write code that covers all possible cases when using unsigned integers [(62)](https://news.ycombinator.com/item?id=29767877).
Consider the following bit of code.
```cpp
void g(int);

void f(int x, int y)
{
	g(x - y);
}
```

Without any checks the `x -  y` expression can both overflow (both `x` and `y` very large) and underflow (`x` very small, `y`  very large).
It is very rare these cases are explicitly checked for in real-world code bases.

Consider an unsigned variant:
```cpp
void g1(unsigned);
void g2(unsigned);

void f(unsigned x, unsigned y)
{
	if (x >= y)
		g1(x - y);
	else
		g2(y - x);
}
```

Assuming `g1` and `g2` are well-defined for all unsigned inputs, so is `f`.
The separation of `g` into `g1` and `g2` makes it clear that we have two different cases, something that may be important in the signed variant as well, but is so implicitly in the fact that the parameter to `g` may be positive or negative.
Are we sure that `g(int)` will handle negative values correctly?
We can be reasonable confident that `g2(unsigned)` handles the `x < y` case correctly since that is it whole reason for existing.

If we do want to handle under- and overflow in the signed integer case we can make use of GCC's arithmetic functions with overflow checking [(65)](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html).
```cpp
extern void g1(int); 
extern void g2(int); 
extern void g3(int); 

void f(int x, int y)
{
  int x_minus_y;
  if(__builtin_ssubl_overflow(x, y, &x_minus_y))
	 g1(x_minus_y);
  else if(x > y)
	 g2(/* What do we pass here? */); // Overflow.
  else
	 g3(/* What do we pass here? */); // Underflow.
}
```

## Return Type

Having a signed return value signals that one should expect negative values to be returned.
In scenarios where only positive values are expected the user may be confused as to during what circumstances the return value could be negative.
Maybe the documentation explains the situation, but maybe it doesn't.

An inline error reporting is where you use the main communication channel, where the expected data should go, to communicate error information.
For a function the main communication channel is the return value.
A common mistake is to return a negative value, or other special value, to signal an error, the absence of a result.
This is bad practice, you should separate out your errors from your values [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
There are other ways to communicate that a result could not be produced:
- Out parameter.
- Exception.
- `std::optional`.
- `std::expected`.
- Assert.

# Operations

## Comparisons

The value compared with an unsigned integer might not itself be an unsigned integer.
The implicit conversion rules will, in many cases, convert a signed value to unsigned and wreck the comparison for us [(45)](https://stackoverflow.com/a/18795564).
Unsigned values have a tendency to contaminate arithmetic and spread like a virus in mixed signed/unsigned expressions.
```cpp
void work(Container& container, std::ptrdiff_t a, std::ptrdiff_t b)
{
	if (container.size() - (a + b) >= 0)
	{
		// a + b seems to be smaller than the container size.
		// All variables we declared are signed because we want
		// signed arithmetic.  However, because the size_t from
		// size()tainted the expression we end up with an unsigned
		// value and a tautology conditional.
	}
}
```

When working with unsigned integers, the index range has to set such that the index never tries to be decremented past 0 [(43)](https://www.cppstories.com/2022/ssize-cpp20/).
It takes effort to guarantee this, and precludes seemingly "obviously correct" / "natural" implementations.

## Test If An Index Is Valid

The fact that signed integers are well behaved around zero can lead developers to forget about the limited range entirely and not consider the lower and upper bounds that still do exist for signed integers just as they do for unsigned integer [(75)](https://youtu.be/82jVpEmAEV4?t=3739).
May of the problems that exist with unsigned integers exists for signed integers as well, assuming adversarial inputs.
If a programmer cannot reason about unsigned integers not be able to take on negative values, then that programmer also likely won't be able to reason about large negative or positive signed integers.
Using a signed integer type will make bad code produce the correct result a larger number of times, but it doesn't offer complete protection.

### Test If An Index Is Valid For A Container

It is common for index ranges to be valid from zero to some positive number, such as the size of a container.
With a signed integer type we must check both the lower and upper end of the range.
With an unsigned integer we only need to check the upper range since the lower bound is built into the type [(28)](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/), [(36)](https://wiki.sei.cmu.edu/confluence/display/cplusplus/CTR50-CPP.+Guarantee+that+container+indices+and+iterators+are+within+the+valid+range), [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java), [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/).

Unsigned integer for both the size and the index:
```cpp
template <typename Container>
bool isValidIndex(Container& const container, size_t const index)
{
	return index < container.size();
}
```

This assumes the computation of `index` didn't under- or overflow before we got here, which is impossible to detect after the fact.
The situation is made worse if we don't have an known upper bound to check against,
such as when resizing a container.
See _Detecting Error States_ > _Detecting Overflow_ for a longer discussion on this.

Signed index for both the size and the index:
```cpp
template <typename Container>
bool isValidIndex(Container& const container, ptrdiff_t index)
{
	return index >= 0 && index < container.size();
}
```

Using unsigned integers may come with a performance improvement dues to the lower number of instructions, but that is unlikely to be the case on a modern computer in most cases since the number of loads is the same and ALU saturation is rarely the limiting factor for execution speed [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf).

We can invert the logic to get a check for invalid indices.
```cpp
bool work(Container& container, size_t const index)
{
	if (index >= container.size())
		return false;

	// Work with container[index].
	return true;
}

bool work(Container& container, ptrdiff_t const index)
{
	if (index < 0 || index >= container.size())
		return false;

	// Work with container[index].
	return true;
}
```

Unsigned size, signed index.
In this case we need to handle the non-overlapping part of the two integer types' range, i.e. the size values larger than the largest possible signed integer.
```cpp
template <typename Container>
bool isValidIndex(const Container& container, ptrdiff_t index)
{
	// Negative indices are never valid.
	if (index < 0)
		return false;

	// Sanity check of the container size. If this check fails then
	// you don't have a choice but to use size_t instead of ptrdiff_t
	// for indexing.
	constexpr size_t const max_allowed_index =
		static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max());
	if (container.size() > max_allowed_index)
	{
		report_error("Container too large for signed indexing");
	}

	return index < std::ssize(container);
}
```

If the the container has implementation size limitations smaller than the full range of `size_t` then we can turn the runtime check of the upper bound to a compile-time check instead.
```cpp
template <typename Container>
bool isValidIndex(const Container& container, ptrdiff_t index)
{
	// Sanity check of the possible container sizes. If this check fails
	// then you don't have a choice but to use size_t instead of ptrdiff_t
	// for indexing if you want to guarantee that all possible container
	// sizes will work.
	size_t constexpr max_allowed_size =
		static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max());
	static_assert(std::vector<T>().max_size() <= max_allowed_size);

	return index >= 0 && index < std::ssize(container);
}
```

The above `static_assert` passes for `std::vector<char>`.

Since unsigned integers cannot be negative there is no need to test whether a given unsigned index is less than zero.
So it is enough to test the index against one end of the range, since the other end is built into the type.
With unsigned, the set of invalid indices consists of only one kind: too large indices [(21)](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699/42).

With a signed type, since it can contain negative values, we must also check the lower end of the range, i.e. 0 [(36)](https://wiki.sei.cmu.edu/confluence/display/cplusplus/CTR50-CPP.+Guarantee+that+container+indices+and+iterators+are+within+the+valid+range).

```cpp
// Unsigned index.
if (index >= container.size())
	return false;

// Signed index.
if (index < 0 || index >= container.size())
	return false;
```

Fewer comparisons are not worth the huge risk of the programmer doing mixed-type arithmetic/comparisons/conversions incorrectly.
It’s better to be just a little more verbose than to have subtle bugs hiding implicitly in the code [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).

Example showing now easy it is to get wrong.
A range check mistake causes an unexpected value to be used as a signed integer is passed to a function taking an unsigned parameter [(73)](https://news.ycombinator.com/item?id=2364065):
```cpp
int size = /* Some value. */;
char buffer[10];
if (size <= 10) {
    // Yay, I have plenty of space.
    memcpy(buffer, src, size);
}
```


A suggestion [(21)](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699) to test signed integers with a single comparison is to cast it to unsigned.

```cpp
if (static_cast<std::size_t>(index) >= container.size())
	return false;
```

This will wrap moderately sized negative values to very large positive values,
probably larger than the container size.
This only works if the index isn't more negative than the size of the container.
This means that if we want to be guaranteed that this trick works then we may never create a container larger than half the range of the unsigned type.
Which is true for e.g. `std::vector` and `std::ptrdiff_t`.

Another option is to wrap the index in a type that expresses the restriction while still retaining signed arithmetic.
The type can provide useful helper functions.

```cpp
struct Positive
{
	integer_t value;

	Positive(integer_t value)
		: value(value)
	{
		if (value < 0)
		{
			report_fatal_error(
				"Negative value provided where a positive is required."
			);
		}
	}

	template <typename C>
	bool isValidIndexFor(const C& container) const
	{
		using index_t = C::size_type;
		if (value > std::numeric_limits<index_t>::max())
		{
			// Guard against overflow / truncation in the
			// integer_t to index_t cast.
			return false;
		}
		const auto index = static_cast<index_t>(value);
		return index < container.size();
	}
}
```



### Test If An Index Is Valid For A Begin / End Pointer Pair

Sometimes we have a `begin` / `end` pair holding a buffer [(76)](https://youtu.be/DRgoEKrTxXY?t=725).
The following bounds check is incorrect since the `begin + index` computation may overflow [(77)](https://www.kb.cert.org/vuls/id/162289/).
This is undefined behavior even though `index` is unsigned because `begin` is a pointer.
(
TODO Find ref. to the relevant section in eel.is/c++draft.
)
```cpp
template <typename T>
bool work(T* begin, T* end, size_t index)
{
	// Possible undefined behavior.
	if (begin + index >= end) ??
		return false;

	// Work with begin[index].
}
```

An attempt to fix it is to check for wrap-around.
Does not work since the overflow is undefined behavior, not wrapping.
```cpp
template <typename T>
bool work(T* begin, T* end, size_t index)
{
	if (begin + index < begin) // The compiler may
		return false;          // remove this code.

	if (begin + index >= end)
		return;

	// Work with begin[index].
```
Unfortunately, since overflow on the `begin + index` computation is undefined behavior the compiler is allowed to optimize based on the assumption that this never happens.
If the overflow cannot happen then `begin + index`, with `index` being an unsigned type, can never be less than `begin` since `index` cannot be negative.
Therefore the check may be removed by the compiler.
There is a warning for this, `-Wstrict-overflow=3`, but it doesn't work since GCC 8 (Find where I read this and add a link).

Another source of undefined behavior is that `begin + index` is undefined behavior even without the overflow.
Simply producing a pointer beyond the end of the underlying memory object, be it an array of a heap allocated buffer, is undefined behavior.

Another way to do the check is
```cpp
template <typename T>
bool work(T* begin, T* end, size_t index)
{
	if (index >= (end - begin))
		return false;

	// Work with being[index].
}
```

I think this is correct assuming the buffer pointed to isn't larger than `std::numeric_limits<ptrdiff_t>::max`, in which case the subtraction is undefined behavior.
For such large buffers I believe the only safe way is to use `(pointer, size)` instead of `(begin, end)`.

### Test If An Index I Valid For A Pointer / Size Pair

```cpp
template <typename T>
bool work(T* begin, size_t size, size_t index)
{
	if (index >= size)
		return false;

	// Work with being[index].
}
```
I believe this way is able to correctly address all of memory.

The extension to signed integer is straight-forward.
```cpp
template <typename T>
bool work(T* begin, ptrdiff_t size, ptrdiff_t index)
{
	if (index < 0 || index >= size)
		return false;

	// Work with being[index].
}
```



## Container Loops
### Loop Over A Container

Looping over a container, such as an array or an `std::vector` is still a common operation.
The classical for loop is written as follows:
```cpp
void work(Container& container)
{
	for (integer_t index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

Things to be aware of [(28)](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/):
- Comparison between `index` and `container.size()`.
	- May trigger an implicit conversion and unexpected behavior.
	- If `integer_t` is signed and `container.size()` unsigned you may get a compiler warning.
		- This is common.
		- Prefer to keep the signedness the same for all values in an expression.
- Increment of `index`.
	- If the type of `container.size()` has a larger maximum value than `integer_t` then the `++index` at the end of each loop iteration may overflow.
- Accessing a container element with `container[index]`.
	- May trigger implicit conversion of `index`.
- More?

`container.size()` is often unsigned, specifically `size_t`, which is an indication that `integer_t` should be `size_t` as well.

It is not uncommon to see code that uses `int` for the loop counter [(13)](https://www.sandordargo.com/blog/2023/10/18/signed-unsigned-comparison-the-most-usual-violations).
Remember that for this note we assume that `int` is 32-bit and `size_t` is 64-bit.
```cpp
void work(Container& container)
{
	for (int index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```
This isn't terribly bad as long as the container isn't very large.
The counter will only ever be positive and a positive `int` can always be implicitly converted to `std::size_t` without loss.
This will work as expected until the container holds more elements than the largest possible `int`.
At that point an undefined behavior sanitizer will report the overflow,
but it is unlikely that data sets of that size are included in unit tests run with sanitizers.

A similar variant is to use `unsigned int` instead, since the index will never be negative.
```cpp
void work(Container& container)
{
	for (unsigned int index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```
This will loop forever if the container size is larger than the largest `unsigned int`, visiting each element again and again until we kill the process.
The reason is that once `index` reaches `std::numeric_limits<unsigned int>::max()` and we increment one more time we wrap back to zero and we start all over again.
A sanitizer typically don't report this because the this behavior is well-defined by the language and might be intended by the programmer.
Enable `-fsanitize=unsigned-integer-overflow` to get a sanitizer warning on this.
Also, beware that infinite loops without side effects are undefined behavior so depending on what the loop body does you may actually have more serious problems than an infinite loop [(85)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2809r2.html).


### Looping Over Two Containers

```cpp
void work(Container1& container1, Container2& container2)
{
	for (integer_t index = 0;
		index < container1.size() && index < container2.size();
		++index)
	{
		// Work with container1[index] and container2[index].
	}
}
```

`Container1::size` and `Container2::size()` may be different return types.
(
What manners of evil may that cause?
)


### Iterating Over All But The Last Element

Sometimes we need to visit all but the last element of a container [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
The straight-forward way to write this is
```cpp
void work(Container& container)
{
	for (integer_t index = 0; index < container.size() - 1; ++index)
	{
		// Work with container[index].
	}
]
```

This fails with unsigned integers for empty containers [(68)](https://github.com/ericniebler/stl2/issues/182) while with signed integers it works as intended.
With unsigned, `container.size() - 1` becomes `0 - 1` which wraps around to a very large number which causes the loop to run too many iterations operating on invalid memory.
We can fix the empty container case with unsigned integers by checking before starting the loop.
Being explicit about precondition and handling special cases separately if often a good idea [(68)](https://github.com/ericniebler/stl2/issues/182).
It forces us to write more straight-forward code with less "trickery", which means that is is easier to understand and easier to get right.
```cpp
bool work(Container& container)
{
	if (container.empty())
		return false;

	for (size_t index = 0; index < container.size() - 1; ++index)
	{
		// Work with container[index].
	}

	return true;
]
```
Or we can move the offset to the other side of the comparison:
```cpp
void work(Container& container)
{
	for (size_t index = 0; index + 1 < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

The guideline is to not use subtraction with unsigned integers.
Instead use the algebraic rules to rewrite all expressions to use addition instead.
Not sure if that is always possible.
(
TODO Find a counter-example.
)
This goes against the guideline in _Midpoint_ since in that case the problem is overflow in the addition of two large numbers, not as here where the problem is an underflow after a subtraction.
So the actual guideline is actually to eliminate all possibilities of underflow and overflow but that is too vague of a guideline so we need to find more specific formulations.
In each case we need to consider the range of possible values for all inputs and all intermediate results.
This is difficult to do correctly every time, especially when editing existing code rather than writing something new.

A signed `integer_t` doesn't save us here if `Container::size()` returns an unsigned integer, such is the case with `std::vector`, since the problem has already happened before the signedness of `integer_t` event comes into play, and even then the implicit conversion rules would convert our signed index to the unsigned type anyway, which is not helpful in this case.

We must either explicitly check for the empty container case or make sure the end-of-range computation is done using a signed type.
```cpp

// Handle empty container separately before the loop.
template <typename T>
bool work(std::vector<T>& container)
{
	if (container.empty())
		return false;

	for (integer_t index = 0; index < container.size() - 1; ++index)
	{
		// Work with container[index].
	}

	return true;
}

// Use std::ssize instead of .size() to get a signed size.
template <typename T>
void work(std::vector<T>& container)
{
	for (ptrdiff_t index = 0; index < std::ssize(container) - 1; ++index)
	{
		// Work with container[index].
	}
}

// Cast the size before the subtraction.
template <typename T>
void work(std::vector<T>& container)
{
	ptrdiff_t const size = static_cast<ptrdiff_t>(container.size());
	for (ptrdiff_t index = 0; index < size - 1; ++index)
	{
		// Work with container[index].
	}
}
```

For many types, such as `std::vector`, it is safe to cast the unsigned size to a signed integer type of equal size because `std::vector::max_size` will never exceed that, but for other types that may not be the case.
Then you need to  use the early-out-if-empty variant.

In summary, the initial loop definition works when the container uses signed integers but if it uses unsigned then we must either explicitly check for the empty case or take care to use the size as an unsigned integer instead.
If the container type supports sizes larger than `std::numeric_limits<ptrdiff_t>::max` then we must explicitly check for the empty case.


### Iterating Over A Sub-Range

Sometimes we need to iterate through a subset of the elements of a container, skipping some number of elements at the start and some number of elements at the end [(28)](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/).
This could for example be to SIMD vectorize the bulk of an array computation but we need to skip a few elements at the front to get to the first vector size aligned element, and we need to skip a few at the end because the remaining elements don't evenly divide the vector size.
The straight-forward way to do this is to initialize the loop counter to the initial skip count and subtract the end skip from the container size.

```cpp
void work(Container& container, integer_t start_skip, integer_t end_skip)
{
	for (integer_t index = start; index < container.size() - end_skip; ++index)
	{
		// Work with contianer[index].
	}
}
```

The above fails for unsigned indices when `end_skip > container.size()` because `container.size() - end_skip` underflows, wraps around, and produces a very large number.
This is most common with unexpectedly small, maybe empty, containers.
The resulting behavior is that the loop will visit not only container elements it should visit but also the elements at the end that should be skipped, and then it will overrun the buffer and start operating on invalid memory.
With this implementation it is the responsibility of the caller to eliminate all such cases.
We can improve the implementation by first checking that we shouldn't skip the entire loop.
```cpp
bool work(Container& container, size_t start_skip, size_t end_skip)
{
	if (end_skip > container.size())
		return false;

	for (integer_t index = start; index < container.size() - end_skip; ++index)
	{
		// Work with contianer[index].
	}

	return true;
}
```

With signed indices we don't have this problem since `container.size() - end_skip` will correctly produce a negative value and `index < container.size() - end_skip` will terminate the loop immediately, assuming `start_skip` is positive.

Signed indices will fail to behave correctly if `start_skip` or `end_skip` is negative since that will cause it to index out of bounds of the container.
So we need to check for that explicitly.
```cpp
bool work(Container& container, ptrdiff_t start_skip, ptrdiff_t end_skip)
{
	if (start_skip < 0 || end_skip < 0)
		return false;

	for (ptrdiff_t index = start; index < container.size() - end_skip; ++index)
	{
		// Work with container[index].
	}

	return true;
}
```

An integer type that is unsigned but produces signed results in arithmetic expressions, such as `container.size() - end_skip`, would save us as long as `container.size()` is less than the maximum of the signed integer type (I think.).

Another error case is the following [(45)](https://stackoverflow.com/a/18795746):
```cpp
void work(Container& container)
{
	for (std::size_t index = 0; index < container.size(); ++index)
	{
		// Special handling for the first few elements.
		if (index - 3 < 0)
		{
			// Will never get here.
		}

		// A better, and arguably more natural, way to write it.
		if (index < 3)
		{
		}
}
```

A less obvious variant:
```cpp
void work(
	Container& container,
	std::size_t kernel_center,
	std::size_t kernel_width)
{
	for (std::size_t index = 0; index < container.size(); ++index)
	{
		if (kernel_center - index < kernel_width)
		{
			// This element is covered by the kernel.
		}
	}
}
```


The prevalence of such errors shows that real-world programmers often make mistakes distinguishing signed and unsigned integer types [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).



Another operation made more difficult with unsigned integers is stopping some number of elements before the end.
```cpp
void work(Container& container)
{
	for (std::size_t index; index < container.size() - 1; ++index)
	{
		// Work with container[index].
	}
}
```
The above does not work when the container is empty  since `container.size()`


### Loop Over A Container Backwards

When iterating backwards we need to use different loop termination conditions depending on if the loop counter is signed or unsigned.

With a signed loop counter we can initialize the loop counter to the last index of the container, i.e. size - 1, and stop when the index becomes negative because that means that we have gone past the start of the container.
This works since signed integer types behaves normally around zero, decrementing from zero produces a values less than zero [(75)](https://youtu.be/82jVpEmAEV4?t=2455).
```cpp
void work(Container& container)
{
	for (ptrdiff_t index = std::ssize(container) - 1; index >= 0; --index)
	{
		// Workd with container[index].
	}
}
```
This works for all cases, including with an empty container, as long as the container isn't larger than `std::numeric_limits<ptrdiff_t>::max()`.

With an unsigned loop counter we cannot do the same since the value cannot become negative.
Decrementing from zero produces a larger value, not a smaller.
The `index >= 0` condition is always true.
Such as `unsigned_value >= 0` [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc) or the opposite `unsigned_value < 0` [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).
It is a mistake that is easy to do by accident, but luckily the error is quickly detected in testing since it produces an infinite loop and most likely a segmentation fault after the loop counter has wrapped around.
The classical accidentally infinite loop:
```cpp
void work(Container& container)
{
	for (size_t index = container.size() - 1; index >= 0; --index)
	{
		// Work with container[index].
	}
}
```


This is not illegal per the language, this code should compile without error.
You may get a warning, but that is just the compiler trying to be helpful.


Can we save it by making the loop counter signed [(75)](https://youtu.be/82jVpEmAEV4?t=2455) and cast the unsigned container size?
```cpp
void work(Container& container)
{
	for (ptrdiff_t index = static_cast<ptrdiff_t>(container.size()) - 1;
		index >= 0;
		--index)
	{
		// Work with container[index].
	}
}
```
This is equivalent to the `std::ssize` variant above but for the following discussion we allow for very large containers, larger than `std::numeric_limits<ptrdiff_t>::max()`.

A drawback is that there is an unsigned to signed conversion in the code.
Not all `size_t` values can be represented in `ptrdiff_t`.
It will work for `std::vector`, but it may not work for any particular container type you may have.
If you have a too large size, more than 9 quintillion for 64-bit, then that would become a negative value when cast to signed and the loop would not run.
This is a case of "It works almost all the time.", which is really bad from a safety and security perspective since it makes the problem difficult to discover in testing.
The fact that the loop doesn't run at all instead of for the first 9 quintillion elements may be an advantage since that will hopefully make it easier to detect.

A more common way for the unsigned integer case is to detect the wraparound of the loop counter and stop when the index becomes larger than the container size, because that means that the counter wrapped around at zero.
Instead of asking "Is the index still above zero?" we ask "Is the index still smaller than the size?".
This is the same test we usually do with unsigned indices, with regular forward loops and index parameters, so why not use it here as well?
```cpp
void work(Container& container)
{
	for (size_t index = container.size() - 1;
		index < container.size();
		--index)
	{
		// Work with container[index].
	}
}
```

A drawback of this approach is that prevents the use of `-fsanitize=unsigned-integer-overflow` since here we depend on the wrapping behavior, which is precisely what that sanitizer checks for.
We want to be able to use the sanitizers so I don't like this approach for that reason.
(
Is there an annotation, perhaps a comment, we can add to tell the sanitizer that wrapping is expected here?
)

The signed variant checks for, to me at least, the more intuitive condition since the intention when the code was written was to loop from the size down to, and including, zero.
The unsigned variant has a more familiar condition since it uses the same one for both forwards and backwards loops.
To reverse the loop direction with an unsigned loop counter we simply start at the end instead of the  beginning and step the other direction, no need to mess with the termination condition.
To reverse the loop direction with a signed loop counter we must edit all three parts of the loop header.

Note that there is no way to write a reverse for loop like this when the signed-ness of the loop counter is unknown.
We cannot use the `index >= 0` condition with an unsigned counter because that expression is always true, for any value of `index`.
We cannot use the `index < container.size()`  condition with a signed counter because it won't wrap at zero and negative indices will be passed to `container[index]`.
Two options we have are to either check both conditions and suffer the "expression is always true" warning when the container uses an unsigned type, or to delegate the loop termination condition to an overloaded helper function.
```cpp
template <typename Container>
bool continueReverseLoop(size_t index, Container const& container)
{
	return index < container.size();
}

template <typename Container>
bool continueReverseLoop(ptrdiff_t index, Container const& container)
{
	return index > 0;
}

template <typename Container>
void work(Container& container)
{
	for (Container::size_type index = container.size() - 1;
		continueReverseLoop(index, container);
		--index)
	{
		// Work with container[index].
	}
}
```

Another option when using unsigned indices is to take a completely different approach,  one based on a while loop instead of the conventional for loop [(33)](https://stackoverflow.com/questions/10040884/signed-vs-unsigned-integers-for-lengths-counts).
```cpp
void work(Container& container)
{
	size_t index = container.size();
	while (index-- > 0)
	{
		// Work with container[index].
	}
}
```

Here we use the post-fix decrement operator within the loop condition.
This means that index starts one-past the actual index we want to start work on, but it is decremented to a valid index in the loop header before it is first used to index into the container within the loop body.
If there is no valid index, i.e. the container is empty, then `index` starts at 0 and the condition, which sees the initial value of 0, ends the loop immediately since 0 isn't larger than 0.
If the container is non-empty then we first get the size of the container, check it against 0 and find that we should enter the loop, the index is decrement to the last valid index, and  that index is used to access an element of the container in the loop body.
Then the once-decremented index is tested against 0 and if still larger then we do another round in the loop.
It some point the index becomes 1, which means that we are about the enter the last loop iteration.
The condition tests 1 > 0, the index is decremented to 0 and we access `container[0]`.
Then we do the last condition check with the index being zero, which evaluates to false and the loop ends.
The final decrement still happens so at the end of the loop the index is `std::numeric_limits<size_t>::max()`.
So this variant also triggers wrapping even though we don't use the result, but it is still enough to trigger an error with `fsanitize=unsigned-integer-overflow`.
(
I assume, I haven't tested yet.
I assume the compiler is allowed to notice that the final value is never read and therefore structure the machine code in a way such that the final decrement never happens.
Not sure if a transformation like that is still possible to do with `-fsanitize=unsigned-integer-overflow` enabled, or if the add added instrumentation prevents that.
)

Some write the condition as `index --> 0`, instead of `index-- > 0`, and calls `-->` the "down-to" operator.
Don't do this. It hides what is actually happening.

Another variant is a do-while loop [(75)](https://youtu.be/82jVpEmAEV4?t=2455).
```cpp
void work(Container& container)
{
	if (container.empty())
		return;

	size_t index = container.size();
	do
	{
		index--;
		// Work with container[index].
	} while (index != 0);
}
```

This works by initializing the index to the container size and immediately decrementing it to a valid index to be used in the loop body.
Then the loop condition checks if this was the last iteration and if so terminates the loop.
This avoids the wraparound since we only decrement if it safe to do so.
A drawback is that we must remember to check for the empty container case explicitly since if we forget that we unconditionally decrement the 0 index and use an invalid index in the loop.
Here `-fsanitize=unsigned-integer-overflow` will correctly point out the error for us, if we have a test with an empty container.

My conclusion after all of this is that signed indices and sizes makes reverse loops easier to write correctly than unsigned ones, but we really should move away from these types of loops and instead use some higher-level way of visiting all elements of a container, such as a ranges library or iterators.


The following intuitive loop only works with signed integers since the `index >= 0` condition only makes sense for signed integer.

```cpp
void work(const Container& container)
{
	for (integer_t index = container.size() - 1; index >= 0; --index)
	{
		// Work on container[index].
	}
}
```

One way to make it work also for unsigned integers is to offset by one, making loop counter 1 the last iteration.
```cpp
void work(Container& container)
{
	for (integer_t i = container.size() ; i > 0; --i)
	{
		integer_t const index = i - 1;
		// Work with container[index].
	}
}
```

This works when the stride is one, which means that we are sure that we will hit 0 when the loop should end.

The following strided loop fails half of the time with unsigned integers, when `i` goes from `1` to `18446744073709551615`, while with signed integers the loops works just fine.
```cpp
void work(Container& container)
{
	for (integer_t i = container.size(); i > 0; i -= 2)
	{
		const integer_t index = i - 1
		// Work on container[index];
	}
}
```

Another way to write the loop, which works only for unsigned integers, is to check for the wrapping instead of the stop condition [25](https://graphitemaster.github.io/aau/).
Check the upper end of the range instead of the lower end, since on wrapping, i.e. when we should stop, the index will go very large.

```cpp
void work(Container& container)
{
	for (std::size_t index = container.size() - 1; i < size; --i)
	{
		// Work with container[index].
	}
}
```

This works, but if we make a habit of writing our loops like this then we must beware that if we every have a singed loop counter then we must switch to another construct.

Back to the check-for-reached-container-start variant again.
The stop-at-error condition doesn't need to be as obvious.
The following code has the same problem [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing), but only for some values of `end`.
```cpp
std::size_t find(
	Container<T>& container, std::size_t start, std::size_t end, T value)
{
	for (std::size_t index = start; index >= end; ++index)
	{
		if (container[index] == value)
		{
			return index;
		}
	}
}
```

When `end` is 0 we get the same `index >= 0` tautology, and an infinite loop again.
Note that the `end` index is inclusive, which is not how it is usually done in C++.
We may try to make end non-inclusive instead.
That changes the loop condition to `index > end` and the loop is no longer infinite for `end = 0`.
However, consider what happens if the caller wants to search the entire range, i.e. all the way to and inclusive the first element.
In that case `end` must be one-less than the first element, i.e. one-less than 0.
So we pass, perhaps unknowingly because the calling function is also operating on a parameter, `0 - 1`, which is `-1`, which is 18446744073709551614.
There is no `std::size_t` value larger than that so the `index > end` will never be true,
the loop will not run a single iteration.
Must do the `std::size_t index = i - 1` rewrite again to make it work as intended.

Another variant is to separate the loop counter from the index.
We let the loop counter count as we normally, i.e. from zero and up, and compute the index.
Now `i` counts how many elements we have already processed.
On the first iteration we have processed 0 elements so should move 0 indices down from the last element.
The last element is at `container.size() - 1`.
```cpp
void work(Container& container)
{
	for (integer_t i = 0; i < container.size(); ++i)
	{
		integer_t index = container.size() - 1 - i;
		// Work with container[index].
	}
}
```

Programs very often deal with with zero and nearby values.
That is right at the edge of underflow [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc), [(75)](https://youtu.be/82jVpEmAEV4?t=3692).
Even a small mistake is enough to fall over the edge [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
This magnifies off-by-one errors [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
This makes reverse loops non-trivial to write, the following classical example does not work:
```cpp
void work(const Container& container)
{
	for (std::size_t index = container.size() - 1; index >= 0; ++index)
	{
		// Work with container[index];
	}
}
```

This is better:
```cpp
void work(const Container& container)
{
	for (std::size_t i = container.size(); index > 0; ++i)
	{
		const std::size_t index = i - 1;
		// Work with container[index];
	}
}
```

Another option is something based on [`std::reverse_iterator`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator) or [`std::ranges::reverse_view`](https://en.cppreference.com/w/cpp/ranges/reverse_view).

An operation that is often written incorrectly with unsigned types is backwards loops [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
```cpp
void work(const Container& container)
{
	for (integer_t index = container.size() - 1; index >= 0; index--)
	{
		// Work on container[index];
	}
}
```

`index >= 0` will always be true when `index_t` is unsigned, so we get an infinite loop.

A fix is to offset the indexing by one.
The loop counter is always one larger than the index to work on,
starting at `container.size()` and ending on, i.e. not entering the loop body when, it becomes 0.
```cpp
void work(Container& container)
{
	for (integer_t i = container.size(); i > 0; i--)
	{
		const integer_t index = i - 1
		// Work on container[index].
	}
}
```

This works for both signed and unsigned loop counters.

This technique fails for unsigned loop counters when the step-size is larger than 1.
```cpp
void work(Container& container)
{
	for (integer_t i = container.size(); i > 0; i -= 2)
	{
		const integer_t index = i - 1
		// Work on container[index].
	}
}
```

Here the condition is `i > 0` so we will stop after having processed `i = 1`, i.e. index 0, which works when walking with stride 1.
But here we work with stride 2 and may miss 0 and instead wrap back up the very large values when using unsigned integer.
The error will happen half of the time, on average, assuming the contains has a random number of elements each time.
Even number: works fine. Odd number: wrap around to large values.
With a singed integer we instead compare `-1 > 0` and things work as we intended.

A solution that works for unsigned loop counters is to invert the loop condition.
Instead of allowing indices that are above the lower bound, 0, we allow indices that are below than the upper bound, the container size.
In essence, we terminate the loop when we detect wrap-around to a very large value.
```cpp
void work(Container& container)
{
	for (std::size_t index = container.size() - 1; index < container.size(); --i)
	{
		// Work with container[index].
	}
}
```

This works for unsigned loop counters but not for signed loop counters since after 0 they would step to -1, which is not a valid index.
A drawback of this approach is that in many cases we don't want wrap around and may run our program with `-fsanitize=unsigned-integer-overflow`, and that would trigger on this (indented) wrap-around.

Another variant that works for single-step backwards loops is the following.
```cpp
void work(Container& container)
{
	for (std::size_t index = container.size(); index--; /* empty */)
	{
		// Work with container[index].
	}
}
```

The loop variables is updated in the conditional using a post decrement.
This means that when `index` is 1 at the start of the conditional it evaluates to true and we enter the loop, but before that `index` is decremented to 0.
On the first iteration we initialize `index` to the one-past-end index, i.e. `size()`, but before we enter the loop body the first time `index` has been decremented to the last valid index.
If there is no valid index, i.e. the container is empty, then `index` is initialized to zero, used in the conditional evaluated as false, and then decremented and wrapped to some very large values.
But that's OK since we never enter the loop body.
The invalid index is never used.

Another variant is to use an if-statement and a do-while loop instead [(73)](https://news.ycombinator.com/item?id=2364065):
```cpp
if (count > 0) {
    unsigned i = count - 1;
    do {
	    /* body */
    } while (i-- != 0);
}
```


#### Walking Image Rows Backwards

Another use-case [(22)](https://stackoverflow.com/questions/16966636/signed-unsigned-integer-multiplication) for a reverse loop is to walk over the rows of a matrix or an image.
In addition to counting backwards, this code also mixes both signedness and bit widths (here we assume that `std::size_t` is larger than `std::int32_t`) to break things.
In the following `image` contains pixel data representing a `width` x `height` large image in row-major order.
```cpp
void work(
	Pixel* const pixels,
	integer_t const num_rows,
	integer_t const num_columns,
	std::int32_t const stride)
{
	for (std::uint32_t i = 0; i < num_rows; ++i)
	{
		Pixel* row = pixels + i * stride;
		// Work with row[0] to row[num_columns - 1].
	}
}

void work_backwards(Image& image)
{
	Pixel* last_row = image.pixels + (image.height - 1) * image.width;
	work(last_row, image.height, -image.width);
}
```

With signed integers there is not problem here.
With unsigned integers we have a problem with the `i * stride` part when `stride` is negative.
When `stride` is implicitly converted to unsigned we end up with a very large number.
We therefore jump very far away from `pixels` when computing `row`.
For reasons not completely clear to me it works out the way we intend when the indexing calculation is done at the same bit width as the pointer calculation [(23)](https://stackoverflow.com/a/35253263).
My reasoning is that multiplication is simply repeated addition and since addition with a negative number wrapped to a very large number causes a shift upwards by almost the entire range, a wrap around at the top, and then all the way back up towards the number we started at, stopping just where we should to make it appear we actually did add a negative number, so when we multiply with a negative number we can imagine that the same thing happens multiple times.
The illusion breaks, however, when the bit widths of the pointer and the computed offset doesn't match.
The computation of `i * stride` produces a very large number that would be almost a full trip around the value range on a 32-bit machine, bringing us to the next pixel row in the backwards iteration, but on a 64-bit machine that very large 32-bit number isn't all that large compared to 64-bit addresses and when we add a not all that large number to the `Pixel` pointer we get a garbage pointer.


## Compute An Index With A Non-Trivial Expression

Not all functions simply loop over a container, sometimes we need to do non-trivial arithmetic to compute the next index.
This chapter contains a few examples of such cases.

In what ways can the following fail [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned)?
```cpp
uint64_t base = get_base_index();
int16_t delta = get_offset(base);
uint64_t index = base + delta;
```

- `get_base_index` might not return `uint64_t`.
	- Signed-unsigned conversion if return value is signed.
	- What happens if the returned value is negative?
- `get_offset` may not take an `uint64_t` parameter.
	- What if it is a smaller unsigned type?
	- What happens if is is a 64-bit signed type?
	- What happens if it is a signed type smaller than 64-bits?
- `get_offset` might not return `int16_t`.
	- Same questions as for `get_base_index` return value, with the additions that the returned type may be larger than 16 bits.
- `delta` is implicitly converted to `uint64_t` for the addition. What happens if it is negative?
- `delta` may be more negative than the value of `base`.
- `index` may be larger than the size of the container it is used to index into.


While a container size cannot be negative, one size minus another absolutely can be [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity), [(59)](https://stackoverflow.com/questions/2550774/what-is-size-t-in-c/2551647#2551647).
```cpp
void work(Container& container1, Container& container2)
{
	// This is bad.
	std::size_t size_diff = continer1.size() - container2.size();
}
```

While an index cannot be negative, the expression to compute the index may contain negative intermediate results.
You cannot use an unsigned integer types in these cases [(59)](https://stackoverflow.com/questions/2550774/what-is-size-t-in-c/2551647#2551647).

While not guaranteed, for many expressions it actually does work out correctly even when intermediate results are negative.
The wrapping behavior makes it so that once we compute ourselves back to positive values we end up at the correct value.
This works for addition, subtraction, and multiplication, but not for division.
Two examples that works.
```cpp
std::size_t base {4};
std::size_t neg_step {2};
std::size_t num_neg_steps {4};
std::size_t pos_step {3};
std::size_t num_pos_steps {3};
std::size_t index =
				base // 4, so 4 so far.
				- (neg_step * num_neg_steps) // -8, so -4 so far.
				+ (pos_step * num_pos_steps)); // +9, so index = 5.
```
```cpp
std::size_t base {4};
std::size_t neg_step (-2); // "Negative" value instead of subtraction below.
std::size_t num_neg_steps {4};
std::size_t pos_step {3};
std::size_t num_pos_steps {3};
std::size_t index =
				base // 4 so far.
				+ (neg_step * num_neg_steps) // 18446744073709551608
				+ (pos_step * num_pos_steps)); // Wrap back to 5.
```

Example that doesn't work, i.e. that uses division.
```cpp
std::size_t base {4};
std::size_t neg_step {2};
std::size_t num_neg_steps {4};
std::size_t div {2};
std::size_t num_pos_steps {7};
std::size_t corrected = base - (neg_step * num_neg_steps); // -4
std::size_t index = (corrected / div) + num_pos_steps; // -2 + 7 = 5? No :(
```

The problem with division is that it doesn't "walk across the line" like addition, subtraction and multiplication does.
Instead it takes the very large "negative" value and makes it smaller, i.e it walks _away_ from the wrap line.
So when we expected -2 we instead got 9223372036854775806.
Note that his is about half of the maximum value of 18446744073709551615.
So at that point we're completely off on the number circle and we won't ever be getting back.

It is unfortunate that the result of the computation ends up correct in some cases but not always.
Especially when the cases that work are more common (citation needed), but a slight change, e.g. divide by the size of something, and the whole thing falls apart unexpectedly.

It can also be safe to cast the unsigned value to signed and get the expected negative value.
```cpp
std::size_t large = 1000;
std::size_t small = 400;
std::size_t udiff = small - large;
std::ptrdiff_t sdiff = static_cast<std::ptrdiff_t>(udiff);
// udiff = 18446744073709551016
// sdiff = -600
```

With unsigned indexing if you accidentally compute a "negative" value and use that to index into a container you will often get a very obvious error since it will try to read very far off into memory.
Likely a crash.
With unsigned indexing a slight miscalculation resulting in a small negative number will cause indexing into a container to access memory  close to the elements and likely valid accessible memory [(21)](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699).
Memory that is used for something else.
This can lead to hard to diagnose errors since the bug manifests itself elsewhere, possibly far from where the actual error, the bad container access, really is.
Debug builds, asserts, and possibly memory sanitizers, can help identify such problems.

### Signed Underflow Is Farther Away From Common Numbers

With signed integers we have a larger margin from commonly used values to the underflow edge [(25)](https://graphitemaster.github.io/aau/), [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned). [(75)](https://youtu.be/82jVpEmAEV4?t=3712).
Signed values are well-behaved around zero, a common value [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
Small mistakes are unlikely to cause an underflow [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
For unsigned types, that boundary point at the busiest spot, zero  [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).

Though beware of malicious inputs that intentionally bring the program close to the domain boundaries [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
Just because the limits are far away from the common values doesn't mean that we can ignore them [(62)](https://news.ycombinator.com/item?id=29766658).
That is something that is easier to do, accidentally or not, if we don't expect them to ever matter.
With unsigned integer we are more often right at the edge of being thrown across the universe so it is easier to keep this in mind while coding [(68)](https://github.com/ericniebler/stl2/issues/182).
This can lead to security vulnerabilities.
An advantage of having the failure case close to zero is that bugs are more likely to be found in testing and fixed before the software goes to production [(67)](Unsigned integers, and why to avoid them).

Relying on the negative domain of signed integers to skip bounds checking is an appeal to luck, which is not something we should do.
(What do I mean by this? When would it ever be OK to skip bounds checking just because the value is signed? Do I mean the min/max values, and not valid index bounds?)

Some say that this property doesn't have any value [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
That \[-32768, 32767\] is no better/worse than \[0..65535\], just different.
That `a < 0` vs `a > 32767` is equivalent.


### Division In Index Calculation

```cpp
void work(
	Container& container,
	integer_t base_index,
	Container<std::ptrdiff_t>& byte_offsets,
	integer_t element_size)
{
	for (integer_t byte_offset : byte_offsets)
	{
		integer_t index = base_index + (byte_offset / element_size);
		// Work with container[index].
	}
}
```

### Midpoint, i.e. "Nearly All Binary Searches Are Broken"

A step in many algorithms involves finding the midpoint of an index range.
This is used during binary search, merge sort, and many other divide-and-conquer algorithms [(61)](https://research.google/blog/extra-extra-read-all-about-it-nearly-all-binary-searches-and-mergesorts-are-broken/) [(25)](https://graphitemaster.github.io/aau/).
The classical, but broken, implementation is as follows.
```cpp
integer_t search(Container<T>& container, T key, integer_t low, integer_t high)
{
	integer_t mid = (low + high) / 2;
	
	// Check if container[mid] is less than, equal to, or larger than key.
}
```

If we have a range at high indices then the `low + high` part will overflow, resulting either in undefined behavior (for signed integers) or the wrong result (for unsigned integers).
It doesn't matter if we use signed or unsigned indices, both cases will fail.
Though unsigned integers will be able to handle larger containers than signed integers.

The proposed solution is to use `mid = low + (high - low) / 2` instead.
This fixes the problem by computing a relative offset, which small, instead of a big sum.
```cpp
integer_t search(Container<T>& container, T key, integer_t low, integer_t high)
{
	integer_t mid = low + (high - low) / 2;

	// Check if container[mid] is less than, equal to, or larger than key.
}
```

This will instead fail if the indices are allowed to be negative since subtracting a very negative value is the same as adding a very large value.
Negative indices are more uncommon though, so this variant often works in practice.

If we use signed integers then we can use the sum version if we first convert `low` and `high` to the same-size unsigned integer type, do the arithmetic, and then convert back to the signed type.
This is not a recommendation, just an observation.
Since the unsigned type has twice the range it is guaranteed to be able to hold the result of the sum.
And since we divide by two before converting to signed we are guaranteed to have a value that is in range of the signed integer type.
So we use the extra space available to unsigned integers as an overflow protection area for the intermediate result, and then return back to signed land.
```cpp
std::ptrdiff_t high = /* Something. */;
std::ptrdiff_t low = /* Something. */;
std::ptrdiff_t mid =
	static_cast<std::ptrdiff_t>(
		(
			static_cast<std::size_t>(low)
			+ static_cast<std::size_t>(high)
		)
		/ 2u);
```

So we can "fix" the signed indices variant with this unsigned-hack, but that doesn't really make the signed indices choice better than unsigned integers since every index range that the hack fixes works as intended when using unsigned integers even with the original `(low + high) / 2` code.
An advantage of the hack is that it will work for all possible container sizes, while with unsigned integers there are container sizes that won't.
So the dangerous case is prevented at the point where the container is populated instead of in usage code.
Better to fail early [(80)](https://www.martinfowler.com/ieeeSoftware/failFast.pdf).

The take-away from this example is to avoid adding integers that can be large.
Try to rewrite the computation, and strive to use offsets instead of absolute values.

### Compute A Size From A (Element Size, Element Count) Pair

It is common to compute a buffer size by multiplying an element size with an element count.
```cpp
integer_t compute_size(integer_t num_elements, integer_t element_size)
{
	return num_elements * element_size;
}
```
This is at risk of overflow.

A real-world example of a vulnerability caused by a an integer overflow is the following code from the SVG viewer in Firefox 2.0 [(38)](https://bugzilla.mozilla.org/show_bug.cgi?id=360645) where a buffer to hold vertices is allocated:
```cpp
pen->num_vertices = _cairo_pen_vertices_needed(
	gstate->tolerance, radius, &gstate->ctm);
pen->vertices = malloc(
	pen->num_vertices * sizeof(cairo_pen_vertex_t));
```

When multiplied with `sizeof(cairo_pen_vertex_t)` the computation overflows and the buffer allocated becomes too small to hold the vertex data.
The fix in this case was to detect very large vertex counts and reject the SVG file.
```cpp
pen->num_vertices = _cairo_pen_vertices_needed(
	gstate->tolerance, radius, &gstate->ctm);

+    /* Make sure we don't overflow the size_t for malloc */
+    if (pen->num_vertices > 0xffff)
+    {
+        return CAIRO_STATUS_NO_MEMORY;
+    }

pen->vertices = malloc(
	pen->num_vertices * sizeof(cairo_pen_vertex_t));
```

Another option is to use a more lenient rejection limit and let `malloc` decide if there is enough virtual memory available or not.
```cpp
pen->num_vertices = _cairo_pen_vertices_needed(
	gstate->tolerance, radius, &gstate->ctm);

+  if (pen->num_vertices > SIZE_MAX / sizeof(cairo_pen_vertex_t))
+  {
+    return CAIRO_STATUS_NO_MEMORY;
+  }

pen->vertices = malloc(
	pen->num_vertices * sizeof(cairo_pen_vertex_t));
```

### Negative Intermediate Values

(TODO Find a good example of this.)

In some cases the index computation may produce intermediate negative values.
This is OK as long as we ultimately end up with a positive value being passed to `operator[]`.
This works with unsigned integers as long as the computation only involves operations that work as intended under modular arithmetic, e.g. additions, subtractions and multiplications [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).
Not division and reminder.
Also, for unsigned it is required that all variables have the same bit-width, since the usual arithmetic conversions don't do sign-extension for unsigned integers [(73)](https://news.ycombinator.com/item?id=2364065).
The modular arithmetic ensures that the correct number of steps is taken in both directions regardless of any wrapping at either side.
It may make debugging more difficult since printing values won't show the "semantically correct" value but instead the wrapped value.
However, if we do divides on a supposedly negative, but actually very large, values then we will get an incorrect result.

```cpp
void work(
	Container& container,
	size_t base,
	size_t bonus,
	size_t penalty,
	size_t attenuation)
{
	size_t index = base + (bonus - penaly) / attenuation;
	// Work with container[index].
}
```

However, since most subtraction is incorrect anyway [25](https://graphitemaster.github.io/aau/) , this doesn't matter.
Instead of computing the signed delta, use the absolute value of the delta:
```
integer_t delta = std::max(x, y) - std::min(x, y);
```
It won't be the value you wanted, but at least it will be legal and under-/overflow free.
(I understand Dale Weiler doesn't mean it like that, but the whole discussion is weird to me.)

## Detecting Error States

With signed integers we can test for negative vales where we only expect positive values.
With unsigned it isn't as obvious, but we can set a maximum allowed value and flag any value above that limit as possibly incorrectly calculated.

With signed integers we can use sanitizers and `-ftrapv` to detect under- and overflow.
There are sanitizers that do the same for unsigned integers as well, `-fsanitize=unsigned-integer-overflow` [(55)](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html), but we may get false positives since there are valid cases for unsigned calculations that wrap.

And thus not reported by error detection tools such as undefined behavior sanitizers, unless explicitly enabled with `-fsanitize=unsigned-integer-overflow` but beware that this may trigger on intentional wrapping [(55)](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).

```cpp
std::size_t x = /* Expression. */;
std::size_t y = /* Expression. */;
container[x - y];
```


Under- and overflow of signed integers is undefined behavior
This means that we must do all checks before any arithmetic operation.
We cannot detect errors by checking the result.
Many constructs that looks correct in fact aren't.
The following cannot be used to detect overflow:
```cpp
bool work(Container& container, signed int base, signed int offset)
{
	// Aassume offset is positive.
	signed int index = base + offset;
	if (index < base)
	{
		report_error("Overflow in work.");
		return false;
	}

	// Work on container[index].
}
```

Do the following to detect if an under- or overflow is about the happen,
which is non-trivial to memorize or derive when needed [25](https://graphitemaster.github.io/aau/).
```cpp
// Check for addition overflow, i.e a + b.
if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) 

// Check for subtraction overflow, i.e a - b.
if ((b > 0 && a < INT_MIN + b) || (b < 0 && a > INT_MAX + b))
```

Injecting undefined behavior into a program doesn't make it safer or more secure[(34)](https://youtu.be/Fa8qcOd18Hc?t=3110).
The presence of possible undefined behavior can cause the compiler to do unexpected transformations to our code [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), [(68)](https://github.com/ericniebler/stl2/issues/182).


### Detecting Overflow

Unsigned integers have well-defined under- and overflow behavior: they wrap.
This means that we can detect it after it has happened, assuming we still have access to the operands.
```cpp
bool work(Container& container, size_t const base, size_t const offset)
{
	// Unconditionally compute the index, detect overflow
	// by checking if it appears as-if we walked backwards,
	// which is impossible with unsigned integers in the
	// absence of wrapping.
	size_t const index = base + offset;
	if (index < base)
	{
		report_error("Overflow in index calculation.");
		return false;
	}

	// Work with container[index].
	return true;
```
This is a good option to have, but it is rarely used in practice [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
It gets a lot harder if we pass `index` somewhere and that code is responsible for detecting whether a prior calculation wrapped or not.

The above pattern does not work for detecting under- or overflow with signed integers since that is undefined behavior which means that anything can happen.
The under- or overflow must be prevented from ever happening, we as programmers are responsible for eliminating all possibilities of under- and overflow.

We can detect whether wrapping is about to happen.
The purpose of the following code is to compute and use an index if and only if the computation can be performed without causing an under- or overflow.
```cpp
bool work(
	Container& container, size_t const base, size_t const offset)
{
	// Detect the case where there isn't enough room
	// above base to fit offset.
	if (std::numeric_limits<size_t>::max() - base < offset)
	{
		report_error("Overflow in index calculation.");
		return false;
	}

	size_t const index = base + offset;
	// Work with container[index].
	return true;
}
```

Overflow detection with signed integers is more complicated.
We can't do the check after the arithmetic operation since signed under- and overflow is undefined behavior.
We also can't do the unexpectedly-less-than-base check since in the signed case the offset might actually intentionally be negative causing the computed index to be less than `base` without being incorrect.
Instead we must do all checking up-front and we must handle a bunch of different cases [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2911).
```cpp
bool work(Container& container, ptrdiff_t base, ptrdiff_t offset)
{
	if (base ^ offset >= 0)
	{
		// If the result of XOR is positive, i.e. sign bit is not set, then
		// the two values must have had the same sign since XOR produces a
		// 0-bit, i.e. a positive value when the bit is the sign bit, if
		// and only if the two input bits, i.e. the two sign bits, are equal.

		if (base >= 0)
		{
			// Adding a positive offset to a positive base. Make sure there
			// is enough room above the base to fit the offset.
			if (std::numeric_limits<ptrdiff_t>::max - base <= offset)
			{
				report_error("Overflow in index calculation.");
				return false;
			}
		}
		else
		{
			// Adding a negative offset to a negative base. Basically the
			// same situation as the code block above, but we need to check
			// against the minimum value instead of the maximum since adding
			// a negative value will move towards the minimum value.
			if (std::numeric_limits<std::ptrdiff_t>::min() - base <= offset)
			{
				report_error("Overflow in index calculations.");
				return false;
			}
		}
	}
	// No else-case necessary since if the two values have different sign then
	// there is no way that adding them can cause an under- or overflow. If the
	// signs are different then the addition would need to first walk the
	// distance to zero and then all the way to the minimum or maximum value,
	// and then some more to overflow. But the maximum value offset can have is
	// bounded by that zero-to-max/min distance so it cannot be large enough to
	// encompass both that distance and the steps from base to zero.
	//
	// So the addition is guarateed to be overflow-free.
	//
	// I think. I'm not sure the fact that signed integers have a
	// non-symmetrical value distribution between signed and unsigned values
	// doesn't mess this up somewhere.

	ptrdiff_t const index = base + offset.
	// Work with container[index].
	return true;
}
```


(
I'm not 100% sure that the above overflow detection for signed integers is correct.
It is based on a code snippet similar to the following from [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2911) but with the conditions inverted to early-fail instead of early success.
```cpp
bool can_add(std::ptrdiff_t base, std::ptrdiff_t offset)
{
	if (base ^ offset < 0)
	{
		return true;
	}
	else
	{
		if (base >= 0)
		{
			if (std::numeric_limits<std::ptrdiff_t>::max() - base > offset)
			{
				return true;
			}
		}
		else
		{
			if (std::numeric_limits<std::ptrdiff_t>::min() - base > offset)
			{
				return true;
			}
		}
	}

	return false;
```
)

Similar checks exists for subtraction and multiplication as well [(37)](https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap).


### Detecting Underflow

If an application occasionally miscalculates an index to be negative that might not be noticed if using signed integer for indexing other than difficult-to-diagnose bugs caused by reading valid memory but slightly off from the start of an array, assuming an assert didn't catch it.
With unsigned integers for indexing the negative value becomes a very large value and likely a segmentation fault or near-infinite loop on the first use [(68)](https://github.com/ericniebler/stl2/issues/182).
These are both easy to detect in testing [(75)](https://youtu.be/82jVpEmAEV4?t=3830).

When a signed expression unintentionally produces a negative values we can detect that by simply checking if the result is smaller than 0 [(30)](https://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf).
That makes error reporting easier, and thus more prevalent [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
With unsigned integers we can't do that since there can't be any negative values [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone), [(62)](https://news.ycombinator.com/item?id=29766658).
That means bad values can more easily pass deeper into our code while doing something wrong [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), possibly leading to security vulnerabilities.
The original bug is the same, but the impact is different.

In the following the `Container::Container(std::size_t size)` constructor creates a container with `size` elements.
```cpp
ptrdiff_t f();
ptrdiff_t g();
Container container(f() - g());
```
If for some reason `f()` returns a values smaller than `g()` we get a negative value that is implicitly converted to an unsigned `size_t` value.

There is no way to detect this after the fact, the only thing we can do is detect unexpectedly large positive values.
But where do we draw the line?

See _Disadvantages Of Unsigned_ > _Impossible To Detect Underflow_ for a longer discussion on this.
(
TODO Update the above reference.
)

Error detection like this especially important in public interfaces used by people other than ourselves.

Example inspired by example in [_ES.106: Don’t try to avoid negative values by using unsigned_][https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es106-dont-try-to-avoid-negative-values-by-using-unsigned]:
```cpp
short work(unsigned int max, unsigned short x)
{
	while (x < max)
	{
		x += 100;
	}
	return x;
}

void do_work()
{
	work(100'000, 100);
}
```

In this case `work` will never return because `x`, being a `short`, can never be equal to or larger than `100'000`.
Debugging will be required to find the cause for the freeze.
Had this been done with signed values instead then an undefined behavior sanitizer would have detected and reported the overflow and we would immediately know what the problem was.

For a motivation for why `max` and `x` has different sizes consider a scenario where `max` is actually `std::vector<VertexPosition>::size()` and `x` is the start index in a collection of meshlets each with 100 vertices that are being batched processed, and where it is assumed that each meshlets collection doesn't have more than a few thousands of vertices (So that `unsigned short` is large enough and we want to keep the type small since there may be a large number of meshlets.), and `work`, below `process_meshlets` is responsible for processing a consecutive subset of those meshlets for a limited amount of time:
```cpp
#include <chrono>
using namespace std::chrono;

constexpr unsigned short VERTICES_PER_MESHLET {100};

unsigned short process_meshlets(
	std::vector<VertexPosition> const& positions,
	unsigned short start_index,
	duration max_duration)
{
	time_point end_time = steady_clock::now() + max_duration;
	while (
		start_index < positions.size()
		&& stead_clock::now() < end_time
	)
	{
		std::span<VertexPosition> meshlet_vertex_positions {
			&positions[start_index],
			VERTICES_PER_MESHLET
		};
		process_meshlet(meshlet);
		
		// Risk of overflow in this addition.
		start_index += VERTICES_PER_MESHLET;
	}
	return start_index;
}
```


It is impossible to detect earlier arithmetic underflow in an unsigned integer, other than with heuristics [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), [(62)](https://news.ycombinator.com/item?id=29766658), [(73)](https://news.ycombinator.com/item?id=2364065).
By the time we get to the `work` function the damage has already been done and it is difficult to track down where it occurred.
This is a common source of vulnerabilities and memory safety issues  [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
By using an unsigned integer type for a parameter a programmer may believe they are protected from negative inputs.
Which is true, in a sense, but it doesn't decrease the number or probability of bugs since the same mistakes that produces a negative value can still happen with unsigned integers.
The only difference is that instead of getting an easily identifiable negative value we get a very large positive value [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity).
Using unsigned parameters in this way is an attempt at encoding a precondition with the type system, but unfortunately it fails because of implicit type conversions [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
It is a false sense of security, the type system does not help us in this case despite what the `unsigned` word in the parameter list may lead us to believe.
We are tricked into ignoring this whole category of bugs because we think we are safe from them.
We could improve the situation somewhat by introducing an unsigned integer like type that don't allow creation from signed values [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
Then we won't have any unexpected surprises from implicit conversions.
```cpp
class Unsigned
{
public:
    Unsigned() : m_value(0) {}
    Unsigned(std::uint8_t value) : m_value(value) {}
    Unsigned(std::uint16_t value) : m_value(value) {}
    Unsigned(std::uint32_t value) : m_value(value) {}
    Unsigned(std::uint64_t value) : m_value(value) {}
    // TODO Are more types here as they are added to the language.

    Unsigned& operator=(Unsigned other) { m_value = other.m_value; }
    operator std::size_t() const { return m_value; }

    Unsigned(std::int8_t) = delete;
    Unsigned(std::int16_t) = delete;
    Unsigned(std::int32_t) = delete;
    Unsigned(std::int64_t) = delete;
    // TODO Are more types here as they are added to the language.

private:
    std::size_t m_value;
};
```

But it comes with drawbacks as well [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
- It adds friction and other unwanted behavior until get get the type "perfect", if that is even possible.
	- For example, this doesn't compile when used with the `Unsigned` type above:
```cpp
template <typename T>
volatile T dont_optimize {};

template <typename T>
__attribute((noinline))
void consume(T value)
{
    dont_optimize<T> = value;
}
```
- (TODO add more here.)

The above can be expanded to a unit library, with types defining what operations are allowed on them, and have the resulting type of an operation not necessarily being the same as the operands [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).

In the code below the programmer decided that the container passed to `work` should never contain more than `VERY_LARGE_NUMBER`, some constant defined somewhere, and any index argument larger than that is a sign that we had an underflow in the computation of that argument.

```cpp
void work(const Container& container, std::size_t index)
{
	if (index > VERY_LARGE_NUMBER)
	{
		report_error("Possible artihmetic underflow detected in work.");
		return
	}
	if (index >= container.size())
	{
		report_error("Index out of range passed to work.");
		return;
	}

	// Work on container[index].
}
```

What should `VERY_LARGE_NUMBER` be set to?
The smaller we set it to the more cases of underflow we are able to detect, but we also further restrict the set of allowed sizes for the container.
The larger we set it the bigger containers we support, but we risk missing underflows that wrap past the zero/max boundary and incorrectly enter into the legal range again.

Signed types also has the same problem, but it is less frequent in practice (`citation needed`) since the underflow happens much farther away from commonly used numbers [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
For unsigned types the underflow happens near 0 and a lot of real-world arithmetic is done near 0.
Also, underflow with signed integer types is undefined behavior.

The result is that using a signed integer type is similar to setting `VERY_LARGE_NUMBER` to the halfway-point between 0 and the maximum representable value.
So when using unsigned integers for indices we have the flexibility to set `VERY_LARGE_NUMBER` larger than that and thereby support larger containers.

if we know the maximum value a parameter may have, such as when indexing in to a container, then we can use the size of the container to detect invalid values.
This does not work when the container doesn't have a size yet, such as when first allocating it.
In that case we have nothing to compare against and if a "negative" value is computed and passed to the allocation function then we will get a lot more memory than we expected,
or a memory allocation error.

```cpp
Container allocate(std::size_t allocation_size)
{
	Container container;
	if (allocation_size > VERY_LARGE_NUMBER)
	{
		report_error("Possible arithmetic underflow detected in allocate.");
		return container;
	}
	container.resize(allocation_size);
	return container;
}
```

There are cases where the `allocation_size > VERY_LARGE_NUMBER` heuristic check fails to detect a prior unsigned underflow or a signed negative value.
In some cases we can end up in a case where a small signed negative becomes a large but not a very large unsigned value.
It takes a few steps:
```cpp
// Actual work function, where we use the computed index.
void work_3(
    Container& container,
    std::size_t index)
{
    if (index > VERY_LARGE_NUMBER)
    {
	    reprt_error("Possible underflow detected in work.");
	    return;
    }

	// Work with container[index].
}

// Intermediate function that takes unsigned int and passes that
// to an std::size_t parameter.
void work_2(
    Container& container,
    unsigned int index // Unsigned type smaller than std::size_t.
)
{
	// This function represents some kind of preparation work.
	// It results in a call the the above function with the
	// index parameter passed through unchanged by the code
	// but implicitly converted to size_t. Since both the source
	// and destination types are unsigned no sign bit extension
	// will be performed.
    work_3(container, index);
}

// Intermediate function that takes a signed int and passes that
// to an unsigned int parameter.
void work_1(
    Container& container,
    int index
)
{
    // This function represents some kind of preparation work.
	// It results in a call the the above function with the
	// index parameter passed through unchanged by the code,
	// but implicitly converted to unsigned int. Since the
	// source and destination types are the same size the bits
	// will be passed through unchanged.
    work_2(container, index);
}
```

If `work_1` is called with a negative index then it is converted to a somewhat large unsigned value when passed to `work_2`.
It is then expanded to a `size_t` and passed to `work_3`.
Since the expansion happens from an unsigned type sign extension will not be performed and we get a somewhat large number in `work_3`, not a very large number as we would have if we had called `work_3` directly from `work_1`.
The result is that we get something that looks like a valid index and if the container is large enough then we will perform work on the wrong element.
This can result in a hard to track down bug since it only manifests with large data sets,
something that in many projects isn't as broadly tested in unit tests and only happens in production.

A signed integer underflow is just as difficult to test for as an unsigned one.
The difference is that the edge where the underflow happens is much farther away from common values for signed integers than it is for unsigned integers.
For unsigned integers the edge is at 0, for signed integers it is at some large negative number.



### Tool-Assisted Runtime Under- And Overflow Detection


Tools, such as sanitizers and static analysis, can report underflow and underflow on signed arithmetic operations [(45)](https://stackoverflow.com/a/18796084), [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity).
In most cases the user did not intend for the operation to under- or overflow,
and even if if was intended it is undefined behavior and should not be relied upon.
Build with `-fsanitizer=undefined` or `-ftrapv` to be instantly notified when that happens, assuming the compiler didn't optimize the code away on the assumption that singed under- or overflow never happens.
Tools cannot (while strictly following the standard, but see below) do this for unsigned operations in general since the behavior is defined in those cases, it is not an error.

There are tools that can be configured to report unsigned wrapping if we want to,
such as `fsanitize=unsigned-integer-overflow`,
but since there are cases where we actually want wrapping this is not something that can be done in general.
Enabling it may produce false positives.
We may want the check in some parts of the code but not others.

The real world is not as neat as some language lawyers would like to believe.
There is `-fwrapv`, (I had something more in mind to write here.).

For a real-world example consider the famous case [(61)](https://research.google/blog/extra-extra-read-all-about-it-nearly-all-binary-searches-and-mergesorts-are-broken/) of integer overflow plaguing many binary search implementations.
While running a binary search we need to compute a mid-point between two indices.
One way to do this is `mid = (low + high) / 2`.
If we have a large range then the `low + high` part can overflow.
It doesn't matter if we use signed or unsigned indices.
The proposed fix is to use `mid = low + (high - low) / 2` instead.
This fixes the problem by computing a relative offset, which small, instead of the big sum.
Would the following also work? (I think so, it is basically the second solution in [_Extra, Extra - Read All About It: Nearly All Binary Searches and Mergesorts are Broken_ (61)](https://research.google/blog/extra-extra-read-all-about-it-nearly-all-binary-searches-and-mergesorts-are-broken/)
```cpp
std::ptrdiff_t high = /* Something. */;
std::ptrdiff_t low = /* Something. */;
std::ptrdiff_t mid =
	static_cast<std::ptrdiff_t>(
		(
			static_cast<std::size_t>(low)
			+ static_cast<std::size_t>(high)
		)
		/ 2u);
```
The idea is that since unsigned has double the range of signed then it can safely hold the sum of two signed integers.
Then we divide by 2 and bring the value back down into the range of the signed integer type.
So we use the extra space available to unsigned integers as an overflow protection area for the intermediate result, and then return back to signed land.

## Safe Arithmetic

By "safe" I mean guaranteed to have produced the mathematical result,
the result we would have gotten with infinite precision, otherwise we are give a false return or similar.
It is easier to check for unsigned wrap around than signed overflow.
With signed integers safe addition requires up to three of branches and an extra subtraction.
With unsigned it is a single add instruction and a read from the status flags registry [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2910).
Similar for other operators as well.


```cpp
bool safe_add(std::size_t lhs, std::size_t rhs, std::size_t* result)
{
	*result = lhs + rhs;
	return *result > lhs;
}

bool safe_add(std::ptrdiff_t lhs, std::ptrdiff_t rhs, std::ptrdiff_t* result)
{
	if (lhs ^ rhs < 0)
	{
		*result = lhs + rhs;
		return true;
	}

	if (lhs >= 0)
	{
		if (std::numeric_limits<std::ptrdiff_t>::max() - lhs > rhs)
		{
			*result = lhs + rhs;
			return true:
		}
	}
	else
	{
		if (std::numeric_limits<std::ptrdiff_t>::min() - lhs > rhs)
		{
			*result = lhs + rhs;
			return true;
		}
	}

	return false;
}
```


## Using The Standard Library Containers

The standard library uses unsigned integer types for many things [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU), in particular `size_t size() const` and `T& operator[](size_t index)`.
This makes working with the standard library with `integer_t` being signed difficult since we are forced to mix signed and unsigned values in expressions and get sign-conversion warnings all over the place if `-Wsign-conversion` is enabled [(41)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1089r2.pdf),
or explicitly cast our index variables from signed to unsigned on every container access.
This creates verbose, noisy, and difficult to read code, and we risk incorrect behavior if we ever encounter a `std::size_t`  value larger than `std::numeric_limits<std::ptrdiff_t>::max()` [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
It is unclear how such a case should be handled.
The same is true for interactions with much of the C standard library, `malloc`, `strlen`, `memcpy`, and such [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
```cpp
void work(Container& container)
{
	std::size_t const max_allowed_size =
		static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());
	if (container.size() > max_allowed_size)
	{
		report_error("Too large container passed to work, cannot process it.")
		return;
	}

	std::ptrdiff_t size = static_cast<std::ptrdiff_t>(container.size());
	// Can also use
	//    index < std::ssize(container)
	for (std::ptrdiff_t index = 0; index < size; ++index)
	{
		// Work with container[static_cast<std::size_t>(index)].
	}
}
```

I'm not sure if it is legal to cast a `std::size_t` larger than the largest `std::ptrdiff_t` to a `std::ptrdiff_t`, or if that is undefined behavior.
In practice, it is often converted to a negative value, which is what the bit pattern would represent in the signed type since the most significant bit is set and most machines uses two's complement to represent signed integers.
This would cause immediate termination of the loop if we didn't have the size guard [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).

Example where a range check mistake causes an unexpected value to be used as a signed integer is passed to a function taking an unsigned parameter [(73)](https://news.ycombinator.com/item?id=2364065):
```cpp
int size = /* Some value. */;
char buffer[10];
if (size <= 10) {
    // Yay, I have plenty of space.
    memcpy(buffer, src, size);
}
```

Unclear how making `size` be a signed type would help us though.
If the computation of `size` produced a negative value when using a signed `size` then the same expression using unsigned integers instead would produce the same very large value as the signed `size` is implicitly converted to.
We gain nothing, but we lose the ability to check for a negative size.

The C++ Core Guidelines are conflicted on this issue.
[ES.100](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es100-dont-mix-signed-and-unsigned-arithmetic) says that we should not mix signed and unsigned integers and [ES.102](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic) says that we should use signed integers for indices.
This is a contradiction since the signed indices will be mixed with unsigned sizes and indices in the standard library containers.
[ES.102](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic) contains this example:
```cpp
void work()
{
	std::vector<int> data(10);
	for (signed int index = 0; index < data.size(); ++index)
	{
		data[index] = index;
	}
}
```

In the above the expressions `index < data.size()` mixes signed and unsigned integers, and `data[index]` passes a singed value to an operator expecting an unsigned argument, triggering an implicit conversion.
I would expect conversion warnings from the compiler here.

[ES.100](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es100-dont-mix-signed-and-unsigned-arithmetic) contains an exception:

> Do not flag on a mixed signed/unsigned comparison where one of the arguments is `sizeof` or a call to container `.size()` and the other is `ptrdiff_t`.

That's just weird to me, it doesn't feel right.

The example in [ES.102](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic) continues:

```cpp
// OK, but the number of ints (4294967294) is so large that we should get an exception
std::vector<int> data(-2);
```

I don't think that is OK.
-2 simply isn't 4294967294.
And it isn't 4294967294 on 64-bit machines, it is 18446744073709551614.

One solution is to disable `-Wsign-conversion`. 
Many projects do this, but I've also seen recommendations against this solutions,
for example [_Learn C++_ > _16.7 — Arrays, loops, and sign challenge solutions_ @ learncpp.com](https://www.learncpp.com/cpp-tutorial/arrays-loops-and-sign-challenge-solutions/).



## Computing Distance Between Indices

We cannot compute distances between values like we normally would when using unsigned integers.
This looks like it would give us the distance between `start` and `end`, and in some theoretical sense it does, but it doesn't give the shortest distance and the `std::abs` will trigger a compiler error [(32)](https://stackoverflow.com/questions/47283449/should-signed-or-unsigned-integers-be-used-for-sizes).
```cpp
void work(size_t start, size_t end)
{
	size_t distance = std::abs(end - start);
}
```

Beware that the computation may be hidden behind a function template defined elsewhere.
```cpp
// Math.hpp:
template <typename T>
T calculate_distance(T start, T end)
{
	return std::abs(end - start);
}

// Work.cpp:
void work(size_t start, size_t end)
{
	size_t distance = calculate_distance(start, end);
}
```

The problem is that `end - start`, with unsigned `start` and `end`, produces an unsigned result.
If `start > end` then we wrap around and get a very large positive value.
Which is correct, in the sense that if we start at `start` and walk in the positive direction then it will take a great many steps to reach `end`.
We get the distance from `start` to `end` if we start walking in the direction away from `end`, hit the upper bound, wrapped back down to zero, and finally continued on to `end`.
This is probably not what was intended.
The `std::abs` doesn't save us because by the time the argument has been computed we already have a too large value, since the expression cannot ever be negative.
Also, `std::abs` doesn't make any sense for an unsigned type since if we were to implement it it would be the identity function.
For this reason the standard library designers opted to not provide that overload,
giving a compiler error to inform us that we are probably not doing what we thing we are doing.

This is an example where blindly adding casts is a bad idea.
The following fixes the compiler error but we do not get the result we wanted.
```cpp
template <typename T>
std::make_signed_t<T> calculate_distance(T start, T end)
{
	return std::abs(std::make_signed_t<T>(end - start));
}
```

The following works for values that aren't too larger for the signed type,
but this code does not sit well with me.
```cpp
template <typename T>
std::make_signed_t<T> calculate_distance(T start, T end)
{
	using SignedT = std::make_signed_t<T>;
	SignedT const signed_start = static_cast<SignedT>(start);
	SignedT const signed_end = static_cast<SignedT>(end);
	return std::abs(signed_end - signed_start);
}
```


One way to correctly compute the distance where we know that the type is unsigned is
```cpp
void work(size_t start, size_t end)
{
	size_t const distance = std::max(start, end) - std::min(start, end);
}
```

With signed integers the original computation works as intended, as long as the subtraction doesn't underflow which would be undefined behavior.
```cpp
void work(ptrdiff_t start, ptrdiff_t end)
{
	ptrdiff_t const distance = std::abs(end - start);
}
```


## External Data

If you receive input data, such as from a network or sensor, that is defined to be unsigned then you must store it in a signed type that is twice as large as the original data type [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
A 16-bit unsigned value must be stored in a 32-bit signed value to ensure that all valid input values can be represented in the signed type.
This, potentially, leads to a lot of wasted memory.

There is the potential for security vulnerabilities if we fail to do this.
In the following the programmer read the message protocol specification and saw that buffer lengths are stored as 32-bit integers and read the buffer length into a 32-bit signed integer.
If a message with a buffer larger than `std::numeric_limits<std::int32_t>::max()`, which is possible when the value is actually unsigned, then `num_bytes` will hold a negative value.
Execution will pass the `num_bytes > 64` check since it is negative.
Bad things happen in the call to `read`.
`num_bytes` may be negative, but when passed to the, presumably, unsigned second parameter to `read` we actually pass a very large value.
This not only overflows the `buff` buffer, it also reads bytes that was mean for another reader.
```cpp
bool work(Socker& socket)
{
	char buff[64];
	std::int32_t num_bytes = socket.read_uint32();
	if (num_bytes > sizeof(buff))
		return false;
	std::int32_t num_read = socket.read(buff, num_bytes);
	return num_read == num_bytes;
}
```

Checking both sides of the range, as described in _Require Two Comparisons For Range Checks_ then we would at least avoid the buffer overflow, but it is still wrong.
```cpp
	if (num_bytes < 0 || num_bytes > sizeof(buff))
		return false;
```

The proper solution is to use a signed type large enough to hold all possible input values.
```cpp
bool work(Socket& socket)
{
	char buff[64];
	std::int64_t num_bytes = socket.read_uint32();
	assert(num_bytes >= 0);
	if (num_bytes > sizeof(buff))
		return false;
	std::int32_t num_read = socket.read(buff, num_bytes);
	return num_read == num_bytes;
}
```
In this case we can assert, as opposed to check, that `num_bytes` is not negative since there is no 32-bit unsigned integer bit pattern that is converted to a negative `std::int64_t`.
Since we know that we have a positive number in `num_bytes` that came from a 32-bit unsigned integer, we also know that the implicit conversion back to an unsigned 32-bit, or larger, value in the call to `read` will be value-preserving.

We do have a problem though if the buffer size is sent as a 64-bit unsigned value that really can be larger than `std::numeric_limits<std::int64_t>::max()`.
In that case we have no option but to use an unsigned type since on most platforms we don't have a 128-bit integer type.
(
At least not a standardized one.
Clang and GCC have `__int128` with limited library support.
I don't know of any MSVC extension for an 128 bit integer type.
)

## Bit Width Conversions

A bit width conversion is when an integer value with one bit width is converted to a different bit width.
For example `int16_t` to `int64_t`, or `uint64_t` to `uint32_t`.
Converting between signed and unsigned is not considered a bit width conversion.
Converting both to a different bit width and a different signedness is actually a two-step process where the value is first sign-converted and then bit width converted.

If you mix values with different bit width then unsigned is more efficient because the conversion is a no-op [(62)](https://news.ycombinator.com/item?id=29767762).
The value held by a CPU register is always 64-bit, a 16 or 32-bit value has zeros in the unused bits.
(
Is this always true, or can there be garbage data in the upper bits? 
)
With signed values one must perform sign extension when going from a lower bit-width to a higher [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post19), using e.g. `movsxd`.

On the other hand, with signed values the compiler may not need to do any conversion at all.
In some cases it can transform the code to use the target type form the start [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post23).
See _Advantages Of Signed_ > _More Opportunities For Compiler Optimizations_.

## Sign Conversions

It is usually safe to pass a signed integer value in a call to a function taking an unsigned integer parameter [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).
This includes `operator[]` in the standard library containers.
For example, a 64-bit signed integer type can represent all 63-bit unsigned integer values.
It is only when the 64th bit in an unsigned value is set that we get problems.
If we have a sufficiently large signed type then we don't need unsigned ones.
Unsigned and signed types are both useful, but signed types have strictly more functionality than unsigned types.
We need to ensure that the value is in range of the container, but that is true regardless.
We can enable warnings for when we do this if we want, `-Wsign-conversion`, but we can also chose not to enable those and just let the conversion happen.

This is fine even with a negative offset:
```cpp
bool work(
	Container& container,
	std::ptrdiff_t base, std::ptrdiff_t offset, std::ptrdiff_t stride
)
{
	std::ptrdiff_t index = base + offset * stride;
	if (!isValidIndex(container, index))
	{
		report_error(
			"Cannot do work, work item is not a valid index for container.");
		return false;
	}
	
	// Workd with container[index].

	return true;
}
```

The same with unsigned integers is not fine.
Note that only `offset` is signed since that is the only value that can be negative.
```cpp
bool work(
	Container& container,
	std::size_t base, std::ptrdiff_t offset, std::size_t stride
)
{
	// Here be footguns.
	std::size_t index = base + offset * stride;
	if (!isValidIndex(container, index))
	{
		report_error(
			"Cannot do work, work item is not a valid index for container.");
		return false;
	}

	// Work with container[index];

	return true;
}
```

The problem is that `offset * stride` may be more "negative" than `base` is positive.
The we get underflow and a large positive value instead.
In most cases this will assert in a debug build since the resulting index often will be out-of-bounds of the container, but in release we have a memory error and undefined behavior.
It will not assert if the wrapping is so large that it goes all the way back to valid indices again,
in which case we get incorrect behavior, neither a compiler nor sanitizer warning, and possibly a hard-to-locate bug.

"Negative" is in quotes above because the implicit conversion for the multiplication will be signed → unsigned, thus there will be no negative value.
The computation will be correct with correct usage anyway because of the wrapping behavior or unsigned integer types.

A similar problem can happen with the signed version as well, `offset * stride` or `base + offset * stride` can underflow or overflow the range of the singed integer type, 
but in this case an undefined behavior sanitizer will report the error as soon as the incorrect index is calculated since signed integer overflow isn't allowed.

# Error Codes

E.g.
```cpp
bool work(Container& container, T key)
{
	std::ptrdiff_t index = find(container, key);
	if (index < 0)
	{
		// Switch block with a bunch of negative cases,
		// one for each possible error code.
		return false;
	}

	// Work on container[index].
	return true;
}
```

I don't like this much.
It overloads the semantics of the return value of `find`.
Better to keep the error reporting separate, instead of in-band [(75)](https://youtu.be/82jVpEmAEV4?t=2150).
Either with an output parameter, returning a tuple or a struct, returning an optional (when we don't need multiple error values), or `std::expected`.

# Things That Often Work

This chapter is a summary of implementations that often work but still have cases we must guard for.
It contains code that we might find even in operational in-production software that seems to work fine but is susceptible to malicious inputs.

## Midpoint

See _Midpoint, i.e. "Midpoint, i.e Nearly All Binary Searches Are Broken"_ for details.

The typical way of finding the middle of an index range is `mid = low + (high - low) / 2;
This works for all unsigned values (citation/test needed) as long as `low` is less than or equal to `high`, but signed is susceptible to overflow if `high` is large and `low` is negative.

# Compiler Optimizations

Since signed integer under- and overflow is undefined behavior the compiler can optimize based on this knowledge.
For example it can assume that `index + 1` is greater than `index` and that `container[index]` is next to both `container[index - 1]` and `container[index + 1]`.
This is not true for unsigned types since both `- 1` and `+ 1` can cause the value to wrap around.
This can make auto-vectorization more difficult [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).

With signed integers the compiler can simplify `10 * k / 2` to `5 * k`.
This is not possible with unsigned since the `10 * k` part can wrap.

With signed integers the compiler can simplify `a+n < b+n` to `a < b` [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
With unsigned integers this is not possible since one of the additions might wrap while the other does not.

Loops with fixed but unknown number of iterations can be optimized better with signed integers [(16)](https://www.youtube.com/watch?v=g7entxbQOCc).

The compiler can chose to use a larger sized signed integer type if it believes it will make the loop faster since it knows that the smaller sized integer won't overflow and the larger sized integer can hold all values that the smaller can hold [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post23), [(17)](https://youtu.be/yG1OZ69H_-o?t=2357), [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
This is not possible with unsigned integer because wrapping is defined.
```cpp
// What the programmer wrote:
void work(Container& container, std::ptrdiff_t end)
{
	for (int index = 0; index < end; ++index)
	{
		// Work on container[index].
	}
}

// What the compiler does.
// Index became std::ptrdiff_t.
void work(Container& container, std::ptrdiff_t end)
{
	for (std::ptrdiff_t index = 0; index < end; ++index)
	{
		// Work on container[index].
	}
}
```
The programmer can do this transformation itself when using unsigned integer, e.g. instead of using` std::uint32_t` use `std::uint64_t` on a 64-bit machine, or `std::size_t` to work on "any" machine [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc), [(75)](https://youtu.be/82jVpEmAEV4?t=4359).
("any"in quotes because there may be machines where `std::size_t` is different from the native arithmetic type. You may consider using one of the `std::uint_fast*_t` types.)

The author for one of the sources [(17)](https://youtu.be/yG1OZ69H_-o?t=2357) of the above have since distanced himself from it [(75)](https://youtu.be/82jVpEmAEV4?t=4420).

Some code, compiled with Clang 18.1.0.

First example:
```cpp
// 32-bit unsigned both for the size and the counter.
// I see nothing preventing the compiler from producing
// very good code for this since ++index cannot wrap since
// it stops when it reaches size, which is must do.
__attribute((noinline))
double sum(double* data, unsigned int size)
{
    double sum {0.0};
    for (unsigned int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}
```
```S
sum(double*, unsigned int):
        testl   %esi, %esi
        je      .LBB38_1
        movl    %esi, %edx
        movl    %edx, %eax
        andl    $7, %eax
        cmpl    $8, %esi
        jae     .LBB38_8
        xorpd   %xmm0, %xmm0
        xorl    %ecx, %ecx
        jmp     .LBB38_4
.LBB38_1:
        xorps   %xmm0, %xmm0
        retq
.LBB38_8:
        andl    $-8, %edx
        xorpd   %xmm0, %xmm0
        xorl    %ecx, %ecx
.LBB38_9:
        addsd   (%rdi,%rcx,8), %xmm0
        addsd   8(%rdi,%rcx,8), %xmm0
        addsd   16(%rdi,%rcx,8), %xmm0
        addsd   24(%rdi,%rcx,8), %xmm0
        addsd   32(%rdi,%rcx,8), %xmm0
        addsd   40(%rdi,%rcx,8), %xmm0
        addsd   48(%rdi,%rcx,8), %xmm0
        addsd   56(%rdi,%rcx,8), %xmm0
        addq    $8, %rcx
        cmpq    %rcx, %rdx
        jne     .LBB38_9
.LBB38_4:
        testq   %rax, %rax
        je      .LBB38_7
        leaq    (%rdi,%rcx,8), %rcx
        xorl    %edx, %edx
.LBB38_6:
        addsd   (%rcx,%rdx,8), %xmm0
        incq    %rdx
        cmpq    %rdx, %rax
        jne     .LBB38_6
.LBB38_7:
        retq
```

We see after `.LBB38_9` that the compiler has unrolled the loop for us, passing eight additions at the time to the CPU.
We also see that the counter is stored in a 64-bit register, either `rcx` (unrolled part) or `rdx` (non-unrolled part under `.LBB38_6`).
It can do that since `index` starts at 0, the lowest possible value, and walks towards larger values.
Somewhere along that path it will encounter `size` and stop, and it will do that before `index` has had the chance to wrap.

Second example:
```cpp
// 32-bit signed both for the size and the counter.
// I see nothing preventing the compiler from producing
// very good code for this.
__attribute((noinline))
double sum(double* data, int size)
{
    double sum {0.0};
    for (int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}
```
```S
sum(double*, int):
        testl   %esi, %esi
        jle     .LBB39_1
        movl    %esi, %edx
        movl    %edx, %eax
        andl    $7, %eax
        cmpl    $8, %esi
        jae     .LBB39_8
        xorpd   %xmm0, %xmm0
        xorl    %ecx, %ecx
        jmp     .LBB39_4
.LBB39_1:
        xorps   %xmm0, %xmm0
        retq
.LBB39_8:
        andl    $2147483640, %edx
        xorpd   %xmm0, %xmm0
        xorl    %ecx, %ecx
.LBB39_9:
        addsd   (%rdi,%rcx,8), %xmm0
        addsd   8(%rdi,%rcx,8), %xmm0
        addsd   16(%rdi,%rcx,8), %xmm0
        addsd   24(%rdi,%rcx,8), %xmm0
        addsd   32(%rdi,%rcx,8), %xmm0
        addsd   40(%rdi,%rcx,8), %xmm0
        addsd   48(%rdi,%rcx,8), %xmm0
        addsd   56(%rdi,%rcx,8), %xmm0
        addq    $8, %rcx
        cmpq    %rcx, %rdx
        jne     .LBB39_9
.LBB39_4:
        testq   %rax, %rax
        je      .LBB39_7
        leaq    (%rdi,%rcx,8), %rcx
        xorl    %edx, %edx
.LBB39_6:
        addsd   (%rcx,%rdx,8), %xmm0
        incq    %rdx
        cmpq    %rdx, %rax
        jne     .LBB39_6
.LBB39_7:
        retq
```

Identical to the first example, both in terms of instructions and register sizes used, except for a signed vs unsigned constant under `.LBB39_8` and the use of `jle` instead of `je` at the top top handle negative sizes.

Third example:
```cpp
// 64-bit signed for the size, 32-bit signed for the counter.
// The compiler can assume that size isn't very large since
// if it were then ++index would overflow. It can chose to
// use either 64-bit or 32-bit instructions.
__attribute((noinline))
double sum(double* data, std::ptrdiff_t size)
{
    double sum {0.0};
    for (int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}
```
```S
sum(double*, long):
        testq   %rsi, %rsi
        jle     .LBB40_1
        movl    %esi, %eax
        andl    $7, %eax
        cmpq    $8, %rsi
        jae     .LBB40_8
        xorpd   %xmm0, %xmm0
        xorl    %ecx, %ecx
        jmp     .LBB40_4
.LBB40_1:
        xorps   %xmm0, %xmm0
        retq
.LBB40_8:
        movabsq $9223372036854775800, %rcx
        andq    %rcx, %rsi
        xorpd   %xmm0, %xmm0
        xorl    %ecx, %ecx
.LBB40_9:
        addsd   (%rdi,%rcx,8), %xmm0
        addsd   8(%rdi,%rcx,8), %xmm0
        addsd   16(%rdi,%rcx,8), %xmm0
        addsd   24(%rdi,%rcx,8), %xmm0
        addsd   32(%rdi,%rcx,8), %xmm0
        addsd   40(%rdi,%rcx,8), %xmm0
        addsd   48(%rdi,%rcx,8), %xmm0
        addsd   56(%rdi,%rcx,8), %xmm0
        addq    $8, %rcx
        cmpq    %rcx, %rsi
        jne     .LBB40_9
.LBB40_4:
        testq   %rax, %rax
        je      .LBB40_7
        leaq    (%rdi,%rcx,8), %rcx
        xorl    %edx, %edx
.LBB40_6:
        addsd   (%rcx,%rdx,8), %xmm0
        incq    %rdx
        cmpq    %rdx, %rax
        jne     .LBB40_6
.LBB40_7:
        retq
```

Again very similar assembly code, though some differences.
The key observation is that the loop is still unrolled.

Fourth example:
```cpp
// 64-bit unsigned for the size, 32-bit unsigned for  the counter.
// The compiler must ensure that ++index wraps properly.
__attribute((noinline))
double sum(double* data, std::size_t size)
{
    double sum {0.0};
    for (unsigned int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}
```
```S
sum(double*, unsigned long):
        xorpd   %xmm0, %xmm0
        testq   %rsi, %rsi
        je      .LBB41_3
        xorl    %eax, %eax
.LBB41_2:
        addsd   (%rdi,%rax,8), %xmm0
        incq    %rax
        movl    %eax, %ecx
        cmpq    %rsi, %rcx
        jb      .LBB41_2
.LBB41_3:
        retq
```

This time we have put up optimization blockers that prevents the compiler from unrolling the loop.
There is nothing preventing `++index` from wrapping so the compiler chose to handle one element at the time, expecting a wrap at any iteration.

In a micro benchmark, run on quick-bench.com, the difference is runtime is very small for a small buffer of 1024 elements.
The result is similar for larger buffers.
![Sum micro benchmark](./images/sum_loop_bench_1024.jpg)

- `SumUIntUInt`: 2'961
- `SumIntInt`: 2'963
- `SumPtrdiffInt`: 2'961
- `SumSizeUInt`: 3'039

For an even smaller buffer, 128 elements, the difference is larger.

![Sum micro benchmark](./images/sum_loop_bench_128.jpg)

- `SumUIntUInt`: 269
- `SumIntInt`: 270
- `SumPtrdiffInt`: 268
- `SumSizeUInt`: 339

I would like to make a plot with increasing buffer sizes.

Benchmark code:
```cpp
#include <numeric>
#include <vector>



std::size_t N {1<<10}; // Or 1 << 7 for 128 elements.

double* getBuffer()
{
  static std::vector<double> buffer(N);
  static bool initialized {false};
  if (!initialized)
  {
    std::iota(buffer.begin(), buffer.end(), 1.0);
    initialized = true;
  }
  return buffer.data();
}

static void baseline(benchmark::State& state)
{
  for (auto _ : state)
  {
    double* buffer = getBuffer();
    benchmark::DoNotOptimize(buffer);
  }
}

// BENCHMARK(baseline);

// 32-bit unsigned both for the size and the counter.
// I see nothing preventing the compiler from producing
// very good code for this since ++index cannot wrap since
// it stops when it reaches size, which is must do.
__attribute((forceinline))
double sum(double* data, unsigned int size)
{
    double s {0.0};
    for (unsigned int index = 0; index < size; ++index)
    {
        s += data[index];
    }
    return s;
}

static void SumUIntUInt(benchmark::State& state)
{
  for (auto _ : state)
  {
    double s = sum(getBuffer(), static_cast<unsigned int>(N));
    benchmark::DoNotOptimize(s);
  }
}

BENCHMARK(SumUIntUInt);





// 32-bit signed both for the size and the counter.
// I see nothing preventing the compiler from producing
// very good code for this.
__attribute((forceinline))
double sum(double* data, int size)
{
    double sum {0.0};
    for (int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

static void SumIntInt(benchmark::State& state)
{
  for (auto _ : state)
  {
    double s = sum(getBuffer(), static_cast<int>(N));
    benchmark::DoNotOptimize(s);
  }
}

BENCHMARK(SumIntInt);





// 64-bit signed for the size, 32-bit signed for the counter.
// The compiler can assume that size isn't very large since
// if it were then ++index would overflow. It can chose to
// use either 64-bit or 32-bit instructions.
__attribute((forceinline))
double sum(double* data, std::ptrdiff_t size)
{
    double sum {0.0};
    for (int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

static void SumPtrdiffInt(benchmark::State& state)
{
  for (auto _ : state)
  {
    double s = sum(getBuffer(), static_cast<std::ptrdiff_t>(N));
    benchmark::DoNotOptimize(s);
  }
}

BENCHMARK(SumPtrdiffInt);





// 64-bit unsigned for the size, 32-bit unsigned for  the counter.
// The compiler must ensure that ++index wraps properly.
__attribute((forceinline))
double sum(double* data, std::size_t size)
{
    double sum {0.0};
    for (unsigned int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

static void SumSizeUInt(benchmark::State& state)
{
  for (auto _ : state)
  {
    double s = sum(getBuffer(), static_cast<std::size_t>(N));
    benchmark::DoNotOptimize(s);
  }
}

BENCHMARK(SumSizeUInt);
```


If the compiler can know that a variable will not overflow because it is a signed integer then it can apply optimizations that depend on regular repeated arithmetic operations behave "as expected" [(74)](https://www.linkedin.com/pulse/int-uint-question-alex-dathskovsky-).
With unsigned integers that is not possible since the value is allowed to wrap, wrecking havoc with any numerical analysis.
Here is an example of a loop that has a closed form solution if we know that all operations will be performed "as expected".
```cpp

__attribute((noinline))
std::ptrdiff_t sum_range(std::ptrdiff_t n)
{
    std::ptrdiff_t sum {0};
    for (std::ptrdiff_t i = 1; i <= n; ++i)
    {
        sum += i;
    }
    return sum;
}

__attribute((noinline))
std::size_t sum_range(std::size_t n)
{
    std::size_t sum {0};
    for (std::size_t i = 1; i <= n; ++i)
    {
        sum += i;
    }
    return sum;
}
```

```S
sum_range(long):
        testq   %rdi, %rdi
        jle     .LBB50_1
        leaq    -1(%rdi), %rax
        leaq    -2(%rdi), %rcx
        mulq    %rcx
        shldq   $63, %rax, %rdx
        leaq    (%rdx,%rdi,2), %rax
        decq    %rax
        retq
.LBB50_1:
        xorl    %eax, %eax
        retq

sum_range(unsigned long):
        testq   %rdi, %rdi
        je      .LBB51_1
        incq    %rdi
        cmpq    $3, %rdi
        movl    $2, %ecx
        cmovaeq %rdi, %rcx
        leaq    -2(%rcx), %rax
        leaq    -3(%rcx), %rdx
        mulq    %rdx
        shldq   $63, %rax, %rdx
        leaq    (%rdx,%rcx,2), %rax
        addq    $-3, %rax
        retq
.LBB51_1:
        xorl    %eax, %eax
        retq
```

(
The unsigned variant doesn't get a loop anymore, which it used to [(74)](https://www.linkedin.com/pulse/int-uint-question-alex-dathskovsky-).
Not sure what changed.
More analysis needed.
Clang 13 produces a loop, but Clang 14 does not.
How does the non-loop variant work?
)




What happens when we multiply a value by some constant and then divide by that same c constant [(75)](https://youtu.be/82jVpEmAEV4?t=696)?
The mathematical result is that the multiply and the divide cancels out and we get back the original value.
This is not the case with limited precision numbers since the multiply may overflow or wrap, causing the number we then divide not actually being the expected multiple of the original value.
For unsigned integers this is well defined behavior and the standard specifies what the result of the multiplication and the following division should be.
We may therefore get a value smaller than we expect.
For signed values we as programmers must guarantee that the multiplication doesn't overflow since that would be undefined behavior.
We are therefore guaranteed that, for any valid program, we will get back the original value.
The compiler may optimize based on this, meaning that the multiplication and division may be removed.

Example using the constant 7 and `INT_MAX` for the value.
```cpp
volatile int si1 = INT_MAX;
volatile unsigned int ui1 = INT_MAX;
printf("si1 * 7 / 7 = %d\n", si1 * 7 / 7);
printf("ui1 * 7 / 7 = %d\n", ui1 * 7 / 7);
```
The `si1 * 7`computation will overflow and since `si1` is a signed integer this is undefined behavior.
This means that the compiler is allowed to do anything.
For example, it can chose to look at the `7 / 7` part and simplify that away.
So the value printed may be `INT_MAX`.
It may print anything else, doesn't have to be a number, or nothing at all.
Or the program may do something completely different, all bets are off.
Don't do this, not even accidentally.
Undefined behavior is bad.

The `ui1 * 7` computation will wrap and the compiler is required to follow that.
In this case, where all values are know at compile time, the compile can precompute the value, but if the value was unknown at compile time then the compiler would be required to put in the multiplication and the division.

Compiled with Clang 18.1.
```cpp
__attribute((noinline))
unsigned useless(int value)
{
    return value * 7 / 7;
}

__attribute((noinline))
unsigned useless(unsigned int value)
{
    return value * 7 / 7;
}
```

```S
useless(int):
        movl    %edi, %eax
        retq

useless(unsigned int):
        leal    (,%rdi,8), %eax
        subl    %edi, %eax
        imulq   $613566757, %rax, %rcx
        shrq    $32, %rcx
        subl    %ecx, %eax
        shrl    %eax
        addl    %ecx, %eax
        shrl    $2, %eax
        retq
```





Though there are some cases where unsigned provides better optimization opportunities.
For example division [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).
See _Advantages Of Unsigned_ > _More Compiler Optimization Opportunities In Some Cases_.
(
TODO Update the  section reference above.
)
(
TODO List cases where this leads to a measurable performance improvement.
)
(
TODO Are there cases where the same is true also for unsigned integers?
)



## Dividing By A Power Of Two

For example when an expression contains a division or reminder by a power of two [(73)](https://news.ycombinator.com/item?id=2364065), [(74)](https://www.linkedin.com/pulse/int-uint-question-alex-dathskovsky-).
```cpp
__attribute((noinline))
int mod(int b)
{
    return b % 16;
}

__attribute((noinline))
int mod(unsigned b)
{
    return b % 16;
}
```

Clang 18.1 produces
```S
mod(int):
        movl    %edi, %eax
        leal    15(%rax), %ecx
        testl   %edi, %edi
        cmovnsl %edi, %ecx
        andl    $-16, %ecx
        subl    %ecx, %eax
        retq

mod(unsigned int):
        movl    %edi, %eax
        andl    $15, %eax
        retq
```

The signed version has a lot more instructions in order to handle the cases where `b` is negative.


## Algebraic Expression Manipulation

Unsigned integers does not follow the regular algebraic rules [(47)](https://blog.libtorrent.org/2016/05/unsigned-integers/).
Consider the following function where the returned value is computed from a runtime parameter and a few compile-time constants.
```cpp
int32_t work(uint32_t b)
{
	int32_t a = -2;
	int32_t c = 2;
	int32_t d = (a - b) / c + 10;
	return d;
}
```
It would be good for performance if the compiler had been able to rewrite the expression as follows:
- `(a - b) / c + 10`
- `(-2 - b) / 2 + 10`
- `(-2 / 2) - (b / 2) + 10`
- `-1 + 10 - (b >> 1)`
- `9 - (b >> 1)`

Fewer operations and the division, which is fairly expensive, has been replaced with a right-shift.
Alas, this transformation is not legal when `b` is unsigned due to the possibility of wrapping.
This is the assembly code produced by Clang 18.1 [(52)](https://godbolt.org/z/41MazoGW7):
```S
work(unsigned int):
movl    $-2, %eax   # eax = -2.   eax = a.
subl    %edi, %eax  # eax -= edi. eax = a - b.
shrl    %eax        # eax >>= 1.  eax = (a - b) / 2.
addl    $10, %eax   # eax += 10.  eax = ((a - b) / 2) + 10.
retq
```
The above assembly code implements the C++ expression pretty much as written.


Slightly more complicated example:
```cpp
__attribute((noinline))
int32_t wrap_optimization_test(uint32_t b)
{
    int32_t a = -2;
	int32_t c = 2;
	int32_t d = (a - b) / c + 10;
    return d;
}

__attribute((noinline))
int32_t wrap_optimization_test(int32_t b)
{
    int32_t a = -2;
	int32_t c = 2;
	int32_t d = (a - b) / c + 10;
    return d;
}
```
Clang 18.1 produces [(51)](https://godbolt.org/z/41MazoGW7):
```S
wrap_optimization_test(unsigned int):
        movl    $-2, %eax
        subl    %edi, %eax
        shrl    %eax
        addl    $10, %eax
        retq

wrap_optimization_test(int):
        movl    $-2, %ecx
        subl    %edi, %ecx
        movl    %ecx, %eax
        shrl    $31, %eax
        addl    %ecx, %eax
        sarl    %eax
        addl    $10, %eax
        retq
```

The unsigned version contains fewer instructions because division by a power of two is a single shift-right with unsigned integers, while with signed integers it requires some additional work.


```cpp
int32_t work(int32_t b)
{
    int32_t a = -2;
	int32_t c = 2;
	int32_t d = (a - b) / c + 10;
    return d;
}
```
```S
work(int):
movl    $-2, %ecx   # ecx = -2.   ecx = a.
subl    %edi, %ecx  # ecx -= edi. ecx = a - b.
movl    %ecx, %eax  # eax = ecx.  eax = a - b.
shrl    $31, %eax   # eax >>= 31. eax = sign(a - b)
addl    %ecx, %eax  # eax += ecx. eax = (a - b) + sign(a - b).
sarl    %eax        # eax >>= 1.  eax = ((a - b) + sign(a - b)) / 2.
addl    $10, %eax   # eax += 10.  eax = (((a - b) + sign(a - b)) / 2) + 10.
retq
```
The compiler can't do much even when all values are signed because division by a power-of-two isn't just a shift in this case, depending on whether the dividend is positive or negative we may need to adjust the result of the shift in order to get correct rounding towards zero instead of rounding towards negative infinity, which is what a simple shift would result in [(86)](https://stackoverflow.com/questions/39691817/divide-a-signed-integer-by-a-power-of-2).

An even more complicated example [(87)](https://youtu.be/Iz2UOgLMj58?t=1803) that sums the integers from 1 to some given upper limit.
```cpp
integer_t arithmetic_sum(integer_t n)
{
	integer_t sum = 0;
	for (integer_t i = 1; i <= n; i++)
	{
		sum += i;
	}
	return sum;
}
```

This has a closed form solution: `n * (n + 1) / 2`.
Or rather, it would have if `integer_t` had infinite range.
In our limited space world the closed form solution is only valid as long as `sum` doesn't overflow.
Since signed integer overflow is undefined behavior the compiler is allowed to assume that won't happen and can replace the loop with just a few register manipulation instructions.
With unsigned integers the compiler cannot make this assumption and must handle the wrapping case.
Thus the closed form solution cannot be used and the compiler must emit instructions for the loop.
That means that for any non-tiny `n` the signed integer version will be much faster.


## Type Replacement

(
TODO: Find the text talking about 32-bit signed arithmetic being done using 64-bit instructions and move it here.
)

## There is an x86-64 Instruction For Converting A Signed Integer To Floating-Point

Consider a typical average calculation:
```cpp
template <typename T>
double average(Container<T>& container)
{
	T sum {0};
	const integer_t num_elements = container.size();
	for (integer _t index = 0; index < num_elements; ++index)
	{
		sum += container[index];
	}
	return static_cast<double>(sum) / static_cast<double>(num_elements);
}
```

When `integer_t` is a signed integer type the compiler can use the `cvtsi2sd` instruction to convert `num_elements` to a floating-point number.
When `integer_t` is unsigned it takes more work, here converting the integer in `rsi` to a floating-point value in `xmm2` [(45)](https://stackoverflow.com/a/78132178):
```S
movq      %rsi, %xmm1
punpckldq .LCPI12_0(%rip), %xmm1
subpd     .LCPI12_1(%rip), %xmm1
movapd    %xmm1, %xmm2
unpckhpd  %xmm1, %xmm2
addsd     %xmm1, %xmm2
```

`.LCPI12_0` `.LCPI12_1` are a pair of constants describing the bit layout of the floating point type.
I have no idea if this has any impact on real-world performance or not.
If we want to avoid the potentially more costly conversion and are allowed to assume that `num_elements` can never be larger than `std::numeric_limits<std::ptrdiff_t>::max()` , for example if we are using `std::vector`, then we can cast it first:
```cpp
return /* as before */ / static_cast<double>(static_cast<std::ptrdiff_t>(num_elements));
```

# When To Use Signed And When To Use Unsigned

In some cases it is clear which variant should be used.
If you need modular arithmetic and don't need negative numbers then use unsigned.
Example use-cases include cryptography, check sums, hash functions, timers.
If the value doesn't represent a number but a bit field, flags, a hash, or an ID then also use unsigned.

The determining factor is whether the values will be used for arithmetic beyond bit operations.
A topic of contention is what to use for values that should never be negative but are used in arithmetic expressions.
For example counts and array indices.
One recommendation is to use unsigned to signal that the value should never be negative.
Another recommendation is to always use signed even in this case.
- [_Google C++ Style Guide_ > _Integers_](https://google.github.io/styleguide/cppguide.html#Integer_Types)
- [_Learn C++_ > _4.5 — Unsigned integers, and why to avoid them_ @ learncpp.com](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/)

## Reasons For Using Unsigned

Most Values Are Positive And Positive Values Rarely Mix With Negative Values [(6)](https://www.reddit.com/r/cpp_questions/comments/1ej5mo0/comment/lgcbrh0/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button), [25](https://graphitemaster.github.io/aau/).
A better way to put it is that there are domains in which negative values simply doesn't show up, and in those domains it is safe to use an unsigned integer type [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
(
TODO List a few such domains.
)

The Type Used By Real Programmers
Unsigned unsigned integers is absolutely safe if you know what you are doing and make no mistakes [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
If you get burned by any of the pitfalls associated with using unsigned integers then you are a bad programmer [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
We should not allow programmers to switch their brains off because signed overflow is "unlikely" to happen [(62)](https://news.ycombinator.com/item?id=29766658).
Good tools create weak programmers, programming should be tough [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
It is what God intended [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).

## Reasons For Using Signed

An appeal to authority argument but I include it anyway to counter the "the standard library designers chose unsigned and they are experts" argument in favor of unsigned integers.
The relevant factual takeaways from the C++ Core Guidelines has been incorporated in the rest of the _Advantages Of Signed_ subsections. The same is true for the Google C++ Style Guide and other documents listed below.

See
- [_C++ Core Guidelines_ > _Arithmetic_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#arithmetic)
- [_Google C++ Style Guide_ > _Integers_](https://google.github.io/styleguide/cppguide.html#Integer_Types)


# Recommendations

What can a programmer do today to avoid as many pitfalls as possible?

- Don't use unsigned for quantities [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
- Don't do math on unsigned types [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
- Don't mix signed and unsigned types [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
- Use unsigned only for:
	- bitmasks [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
	- modular arithmetic 

Don't use unsigned when you need:
- mathematical operations [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
- compare magnitudes [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

A quote from Bjarne Stroustrup [(15)](https://youtu.be/Puio5dly9N8?t=2644):

> There are far too many integer types, there are far too lenient rules for mixing them together, and it's a major bug source, which is why I'm saying stay as simple as you can, use (signed) integers till you really need something else.

To make it clear when a conversion is happening, always use `static_cast` instead of relying on implicit conversions.

Build with `-Wconversion`.
Use a library for sane signed+unsigned comparisons [(70)](http://ithare.com/c-thoughts-on-dealing-with-signedunsigned-mismatch), [(71)](https://en.cppreference.com/w/cpp/utility/intcmp).

Use a signed type and use one with more range than you think you need [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
Then most of the disadvantages of signed integers are removed.
Not sure what to do if currently using `int64_t` and maybe possibly need a larger type.
When would that ever be the case?

It doesn't seem possible to avoid the issues of mixing integers types with different signedness or sizes.
Instead we should select our types to minimize the damage.
A small negative value converted to an unsigned type causes the receiver to see a large number.
A large unsigned value converted to signed produces a negative value.
Proponents of signed integers claim that getting an unexpected negative value is better than getting an unexpected large value because we can identify the negative value as erroneous.

Error conditions can be prevented or detected using:
- Ranged types [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
- Preconditions [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
- Postconditions [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
- Invariants [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
- Plenty of asserts [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).


## Alternatives To Indexing

Don't use explicit indices at all, do something else instead.

- Range based for loops.
	- Less flexible than index-based loops.
- The ranges library.
- Named algorithms.
- Iterators.

A strategy for avoiding many of the above bugs is to not use indices at all and instead prefer range based loops, named algorithms, iterators, the ranges library, ... (TODO More tips?).
A drawback of iterators is that for some containers, such as `std::vector`, they can become invalidated at times where an index would not.


Where possible, instead of passing an array and a size, pass a view and use ranged based loop instead of an indexing loop:
```cpp
// Old:
void work(byte* data, int size)
{
	for (int i = 0; i < size; ++i)
	{
		// Do work with data[i].
	}
}

// New:
void work(std::span<byte> data)
{
	for (byte& b : data)
	{
		// Do work with b.
	}
}
```


Rust has a range based loop construct that has built-in support for backwards looping [25](https://graphitemaster.github.io/aau/).
```rust
fn work(Container container)
{
	for index in (0..container.size()).rev()
	{
		// Woth with container[index].]
	}
}
```


# Alternative Integer Representations And Semantics

## Bignum

Bignum, i.e. an arbitrary sized integer that allocates more space when needed [(27)](https://blog.regehr.org/archives/1401).
Fixed size integer should only be used where needed, which is rare.

## Change The Conversion Rules

It is problematic that operators require that the two operands have the same type.
It would help if `<`, and the other operators, could treat mixed signedness like `std::cmp_less`, and the other functions in the `std::cmp_*` family of functions.
Not sure how will this would work with arithmetic operators such as `+` and `*`.
Convert to a larger signed type?
What about the largest unsigned type?
Require that `sizeof(intmax_t)` be larger than `sizeof(uintmax_t)`?
That sounds... bad.

Integer promotion and implicit conversions should be removed.

## Overflow Bit

It would help to have some way to detect that an overflow happened, either as part of a type's representation or within the CPU.

For example, we could dedicate one bit of the value representation as a poison bit to indicated that this value had an under- or overflow in its calculation [(62)](https://news.ycombinator.com/item?id=29766658), [(63)](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2868.pdf).
Whenever an under- or overflow happens the poison bit is set, and whenever a poisoned value is passed to an operator the result also has the poisoned bit set.
This way we can do computations as usual, get the same result as we do today, but also check is if an overflow happened after the fact.
```cpp
std::size_t base = /* Some value. */;
std::size_t block_index = /* Some value. */;
std::size_t block_size = /* Some value. */;
std::size_t index = base + block_index * block_size;
if (is_under_or_overflow(index))
{
	log_error("Overflow detected in index calculation.");
	return;
}
```

Variant using a CPU flag instead, similar to what we already have for floating-point operations [(64)](https://man7.org/linux/man-pages/man3/fenv.3.html).
```cpp
clear_cpu_flag(UNDEROVERFLOW_BIT);
std::size_t base = /* Some value. */;
std::size_t block_index = /* Some value. */;
std::size_t block_size = /* Some value. */;
std::size_t index = base + block_index * block_size;
if (test_cpu_flag(UNDEROVERFLOW_BIT))
{
	log_error("Overflow detected in index calculation.");
	return;
}
```

## New Integer Type Designed For Sizes And Indexing

Not sure what the semantics for this type should be.
- Overflow behavior?
- Signed or unsigned?
- Unsigned when declared but signed in arithmetic involving subtraction?
Should carry valid range information, and do runtime checks to enforce it [(68)](https://github.com/ericniebler/stl2/issues/182).


# Signed Overloads In The Standard Library

A suggestion is to add
- `std::ptrdiff_t ssizeof`
- `std::ptrdiff_t std::vector<T>::ssize()`
- `T& std::vector<T>::operator[](ptrdiff_t)`

A similar change is [advised against](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post3) for Eigen by Benoit:

>The only thing that would be really bad would be to try to make both
>signednesses fully supported. That would mean keeping only the intersection
>of the sets of advantages of either signedness, having to deal with the
>union of the sets of constraints.

(
I'm not sure that the "intersection of advantages" and "union of constraints" are.
I should make a list here.
)

An alternative is to wrap the standard library containers in views that provide a signed interface.
Not sure that this is a good idea.

# References


- 1:  [_Should I use unsigned types? Or should I turn off Wconversion?_ @ reddit.com/cpp 2024](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/should_i_use_unsigned_types_or_should_i_turn_off/)
- 2: [_C++ Core Guidelines_ > _ES.100: Don’t mix signed and unsigned arithmetic_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es100-dont-mix-signed-and-unsigned-arithmetic)
- 3: [_C++ Core Guidelines_ > _ES.102: Use signed types for arithmetic_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic)
- 4: [_C++ Core Guidelines_ > _ES.106: Don’t try to avoid negative values by using unsigned_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es106-dont-try-to-avoid-negative-values-by-using-unsigned)
- 5: [_C++ Core Guidelines_ > ES.107: Don’t use unsigned for subscripts, prefer gsl::index](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es107-dont-use-unsigned-for-subscripts-prefer-gslindex)
- 6: [_Why are there no signed overloads of operator[](size_type index) in the standard library containers?_ @ reddit.com/cpp_questions 2024](https://www.reddit.com/r/cpp_questions/comments/1ej5mo0/why_are_there_no_signed_overloads_of_operatorsize/)
- 7: [_Google C++ Style Guide_ > _Integers_](https://google.github.io/styleguide/cppguide.html#Integer_Types)
- 8: [_Learn C++_ > _4.5 — Unsigned integers, and why to avoid them_ @ learncpp.com](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/)
- 9: [_Learn C++_ > _16.3 — std::vector and the unsigned length and subscript problem_](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/)
- 10: [_Learn C++_ > _16.7 — Arrays, loops, and sign challenge solutions_ @ learncpp.com](https://www.learncpp.com/cpp-tutorial/arrays-loops-and-sign-challenge-solutions/)
- 11: [_C++23: Literal suffix for (signed) size_t_ by Sandor Dargo @ sandordargo.com 2023](https://www.sandordargo.com/blog/2022/05/25/literal_suffix_for_signed_size_t)
- 12: [_How to compare signed and unsigned integers in C++20?_ by Sandor Dargo @ sandordargo.com](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities)
- 13: [_My battle against signed/unsigned comparison: the most usual violations_ by Sandor Dargo @ sandordargo.com 2023](https://www.sandordargo.com/blog/2023/10/18/signed-unsigned-comparison-the-most-usual-violations)
- 13: [P1428R0 _Subscripts and sizes should be signed_ by Bjarne Stroustrup @ open-std.org 2018](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf)
- 14: [_eigen Signed or unsigned indexing_ @ eigen.tuxfamily.narkive.com 2017](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing)
- 15: [_Interactive Panel: Ask Us Anything_ 42:41 Q: Elaborate on not using unsigned int unless you want to do bit arithmetic. Answer by Bjarne Stroustrup, Herb Sutter, Chander Charruth @ youtube.com](https://youtu.be/Puio5dly9N8?t=2561)
- 16: [_CppCon 2016: Michael Spencer “My Little Optimizer: Undefined Behavior is Magic"_ by Michael Spencer, CppCon @ youtube.com 2016](https://www.youtube.com/watch?v=g7entxbQOCc)
- 17: [_CppCon 2016: Chandler Carruth “Garbage In, Garbage Out: Arguing about Undefined Behavior..."_ by Chandler Carruth, CppCon @ youtube.com 2016](https://youtu.be/yG1OZ69H_-o?t=2357)
- 18: [_size_t or int for dimensions, index, etc_ @ stackexchange.com 2016](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc)
- 19: [_CppCon 2016: Jon Kalb “unsigned: A Guideline for Better Code"_ by Jon Kalb, CppCon @ youtube.com](https://www.youtube.com/watch?v=wvtFGa6XJDU)
- 20: [_Iteration Over std::vector: Unsigned vs Signed Index Variable_ by @ geeksforgeeks.org 2024](https://www.geeksforgeeks.org/iteration-over-vector-unsigned-vs-signed-index-variable-cpp/)
- 21: [_Subscripts and sizes should be signed_ by T4r4sB et.al. @ rust-lang.org 2022](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699)
- 22: [_Signed & unsigned integer multiplication_ by phkahler et.al. @ stackoverflow.com 2013](https://stackoverflow.com/questions/16966636/signed-unsigned-integer-multiplication)
- 23: [_Signed to unsigned conversion in C - is it always safe?_ by cwick et.al @ stackoverflow.com 2008](https://stackoverflow.com/questions/50605/signed-to-unsigned-conversion-in-c-is-it-always-safe)
- 24: [_Is it a good practice to use unsigned values ?_ by \[deleted\] et.al @ reddit.com/cpp 2018](https://www.reddit.com/r/cpp/comments/7y0o6r/is_it_a_good_practice_to_use_unsigned_values/)
- 25: [_Almost Always Unsigned_ by Dale Weiler @ graphitemaster.github.io 2022](https://graphitemaster.github.io/aau/)
- 26: [_Amost Always Unsigned_ @ reddit.com/cpp 2022](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned/)
- 27: [_Solutions to Integer Overflow_ by regehr @ regehr.org 2016](https://blog.regehr.org/archives/1401)
- 28: [_a praise of size_t and other unsigned types_ by Jens Gustedt @ gustedt.wordpress.com 2013](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/)
- 29: [_Why are unsigned integers error prone?_ by Destructor et.al. @ stackoverflow.com 2015](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone)
- 30: [_Signed and Unsigned Types in Interfaces_ by Scott Meyers @ aristeia.com 1995](https://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf)
- 31: [_Danger – unsigned types used here!_ by David Crocker @ critical.eschertech.com 2010](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/)
- 32: [_Should signed or unsigned integers be used for sizes?_ by Martin Ueding et.al. @ stackoverflow.com 2017](https://stackoverflow.com/questions/47283449/should-signed-or-unsigned-integers-be-used-for-sizes)
- 33: [_Signed vs. unsigned integers for lengths/counts_ by user1149224 et.al @ stackoverflow.com 2012](https://stackoverflow.com/questions/10040884/signed-vs-unsigned-integers-for-lengths-counts)
- 34: [_Signed Integers Considered Harmful - Robert Seacord - NDC TechTown 2022_ by Robert C. Seacord @ youtube.com 2022](https://www.youtube.com/watch?v=Fa8qcOd18Hc)
- 35: [_Unsigned vs. signed integer arithmetic_ by Daniel Lemire @ lemire.me 2017](https://lemire.me/blog/2017/05/29/unsigned-vs-signed-integer-arithmetic/)
- 36: [_SEI CERT C++ Coding Standard_ > _CTR50-CPP. Guarantee that container indices and iterators are within the valid range_ by Justin Picar, Jill Britton @ sei.cmu.edu 2023](https://wiki.sei.cmu.edu/confluence/display/cplusplus/CTR50-CPP.+Guarantee+that+container+indices+and+iterators+are+within+the+valid+range)
- 37: [_SEI Cert C Coding Standard_ > _INT30-C. Ensure that unsigned integer operations do not wrap_ by Robert Seacord, Michal Rozenau @ sei.cmu.edu 2023](https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap)
- 38: [_Firefox 2.0 SVG "_cairo_pen_init" Heap Overflow_ by tommy @ bugzilla.mozilla.org 2006](https://bugzilla.mozilla.org/show_bug.cgi?id=360645)
- 39: [_SEI CERT C Coding Standard_ > _INT01-C. Use rsize_t or size_t for all integer values representing the size of an object_ by Rober C. Seacord, David Svoboda @ sei.cmu.edu 2023](https://wiki.sei.cmu.edu/confluence/display/c/INT01-C.+Use+rsize_t+or+size_t+for+all+integer+values+representing+the+size+of+an+object)
- 40: [_P1227: Signed ssize() functions, unsigned size() functions (Revision 2)_ by Jorg Brown @ open-std.org 2019](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1227r2.html)
- 41 [_Sizes Should Only span Unsigned_ by Robert Douglas, Nevin Liber, Marshall Clow @ open-std.org 2018](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1089r2.pdf)
- 42:  [_N1967: Field Experience With Annex K - Bounds Checking Interfaces_ by Carlos O'Donell, Martin Sebor @ open-std.org 2015](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1967.htm)
- 43: [_Reducing Signed and Unsigned Mismatches with std::ssize()_ by Bartlomiej Filipek @ cppstories.com 2022](https://www.cppstories.com/2022/ssize-cpp20/)
- 44: [_Unsigned int considered harmful for Java_ by Nayuki @ nayuki.io 2018](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java)
- 45: [_Why prefer signed over unsigned in C++_ by Mordachai et.al @ stackoverflow.com 2013](https://stackoverflow.com/questions/18795453/why-prefer-signed-over-unsigned-in-c)
- 46: [_Why does this if condition fail for comparison of negative and positive integers_ by manav m-n et.al. 2013](https://stackoverflow.com/questions/18247919/why-does-this-if-condition-fail-for-comparison-of-negative-and-positive-integers/18249553#18249553)
- 47: [_unsigned integers_ by arvid @ blog.libtorrent.org 2016](https://blog.libtorrent.org/2016/05/unsigned-integers/)
- 48: [_Usual arithmetic conversions_ @ cppreference.com](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions)
- 49: [_Expressions_ > _Usual arithmetic conversions_ @ eel.is/c++draft](https://eel.is/c++draft/expr.arith.conv)
- 50: [_Expressions_ > _Standard conversions_ > _Integral promotions_ @ eelis/c++draft](https://eel.is/c++draft/conv.prom)
- 51: [_SEI CERT C Coding Standard_ > _INT02-C. Understand integer conversion rules_ by Rober C. Seacord, Jill Britton @ sei.cmu.edu](https://wiki.sei.cmu.edu/confluence/display/c/INT02-C.+Understand+integer+conversion+rules)
- 52: [Compiler Explorer experiments @ godbolt.org](https://godbolt.org/z/41MazoGW7)
- 53: [_Building better software with better tools: sanitizers versus valgrind_ by Daniel Lemire @ lemire.me 2019](https://lemire.me/blog/2019/05/16/building-better-software-with-better-tools-sanitizers-versus-valgrind/)
- 54: [_GCC Command Options_ > _3.17 Options for Code Generation Conventions_ @ gcc.gnu.org](https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html)
- 55: [_Clang 20.0.0git documentation_ > _UndefinedBehaviorSanitizer_ @ clang.llvm.org](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
- 56: [_Do not use unsigned for non-negativity_ by Eugene Homyakov @ hamstergene.github.io 2021](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity/)
- 57: [_Almost Always Unsigned_ by graphitemaster et.al. @ reddit.com/cpp 2022](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned/)
- 58: [_Torsors Made Easy_ by John Baez @ math.ucr.edu/ 2009](https://math.ucr.edu/home/baez/torsors.html)
- 59: [_What is size_t in C?_ by Vijay. Alok Singhal, Jason Oster, et.al @ stackoverflow.com 2010](https://stackoverflow.com/questions/2550774/what-is-size-t-in-c/2551647#2551647)
- 60: [_Choosing the type of Index Variables_ by sarat et.al @ stackoverflow.com 2011]((https://softwareengineering.stackexchange.com/questions/104591/choosing-the-type-of-index-variables)
- 61: [_Extra, Extra - Read All About It: Nearly All Binary Searches and Mergesorts are Broken_ by Joshua Bloch @ research.google.com/blog](https://research.google/blog/extra-extra-read-all-about-it-nearly-all-binary-searches-and-mergesorts-are-broken/)
- 62: [_Almost Always Unsigned_ comments @ news.ycombinator.com 2022](https://news.ycombinator.com/item?id=29766658)
- 63: [_Supplemental Integer Safety_ by David Svoboda @ open-std.org 2021](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2868.pdf)
- 64: [_fenv_ @ man7.org](https://man7.org/linux/man-pages/man3/fenv.3.html)
- 65: [_Built-in Functions to Perform Arithmetic with Overflow Checking_ @ gcc.gnu.org](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html)
- 66: [Comment on _Unsigned integers, and why to avoid them_ by Jubatian @ learncpp.com 2020](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/#comment-487024)
- 67: [Comment on _Unsigned integers, and why to avoid them_ by faskldj 2020](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/#comment-474548)
- 68: [_Kill unsigned integers throughtout STL2_ by Eric Nieler @ github.com/ericniebler](https://github.com/ericniebler/stl2/issues/182)
- 69: [_Use unsigned integers instead of signed integers where appropriate_ by Johannes Vollmer @ github.com/kryptan/rect_packer et. al. 2019](https://github.com/kryptan/rect_packer/issues/3)
- 70: [_C++: Thoughts on Dealing with Signed/Unsigned Mismatch_ by "No Bugs" Hare @ ithare.com 2018](http://ithare.com/c-thoughts-on-dealing-with-signedunsigned-mismatch/)
- 71: [_std::cmp_equal, cmp_not_equal, cmp_less, cmp_greater, cmp_less_equal, cmp_greater_equal_ @ cppreference.com](https://en.cppreference.com/w/cpp/utility/intcmp)
- 72: [_They forked this one up: Microsoft modifies open-source code, blows hole in Windows Defender_ by Shaun Nichols @ theregister.com 2018](https://www.theregister.com/2018/04/04/microsoft_windows_defender_rar_bug/)
- 73: [_A fifteen year old TCP bug?_ comments @ news.ycombinator.com 2011](https://news.ycombinator.com/item?id=2364065)
- 74: [_To Int or To UInt, This Is The Question_ by Alex Dathskovsky @ linkedin.com 2022](https://www.linkedin.com/pulse/int-uint-question-alex-dathskovsky-)
- 75: [_Integer Type Selection in C++: in Safe, Secure and Correct Code - Robert Seacord - CppNow 2023_ by Rober Seacord, CppNow @ youtube.com 2023](https://www.youtube.com/watch?v=82jVpEmAEV4)
- 76: [_Keynote: Safety and Security: The Future of C and C++ - Robert Seacord. - NDC TechTown 2023_ by Rober Seacord, NDC Conferences @ youtube.com 2023](https://www.youtube.com/watch?v=DRgoEKrTxXY)
- 77: [_C compilers may silently discard some wraparound checks_ by Chad R Dougherty,  Robert C Seacord @ kb.cert.org 2008](https://www.kb.cert.org/vuls/id/162289/)
- 78: [_Does size_t have the same size and alignment as ptrdiff_t?_ by cigien, Nathan Oliver, Bathsheba, et.al. @ stackoverflow.com 2020](https://stackoverflow.com/questions/61935876/does-size-t-have-the-same-size-and-alignment-as-ptrdiff-t)
- 79: [_What does the expression "Fail Early" mean, and when would you want to do so?_ by Andrew Grimm, Bert F et.al. @ stackoverflow.com 2010](https://stackoverflow.com/questions/2807241/what-does-the-expression-fail-early-mean-and-when-would-you-want-to-do-so)
- 80: [_Fail Fast_ by Jim Shore @ martinfowler.com 2004](https://www.martinfowler.com/ieeeSoftware/failFast.pdf)
- 81: [_Fail-fast system_ @ wikipedia.com](https://en.wikipedia.org/wiki/Fail-fast_system)
- 82: [_The Lakos Rule_ by Arthur O'Dwyer @ quuxplueone.github.io 2018](https://quuxplusone.github.io/blog/2018/04/25/the-lakos-rule/)
- 83: [_Conservative use of noexcept in the Library_ by Alisdair Meredith, John Lakos @ open-std.org 2011](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3279.pdf)
- 84: [_A Guide to Undefined Behavior in C and C++_ by John Regehr @ regehr.org 2010](https://blog.regehr.org/archives/213)
- 85: [_P2809R2 Trivial infinite loops are not Undefined Behavior_ by JF Bastien @ open-std.org 2023](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2809r2.html)
- 86: [_Divide a signed integer by a power of 2_ by Gustavo Blehart, aka.nice @ stackoverflow.com 2016](https://stackoverflow.com/questions/39691817/divide-a-signed-integer-by-a-power-of-2)
- 87: [_Alex Dathskovsky :: To Int or to Uint, This is the Question_ by Alex Dathskovsky, CoreCppIL @  youtube.com 2023](https://www.youtube.com/watch?v=Iz2UOgLMj58)

