A signed integer type is one that can represent both positive and negative numbers, and zero.
An unsigned integer is one that can only represent positive numbers, and zero.
The purpose for this note is to explore the pros and cons of using each in cases where it is not obvious, such as for indexing.
There is no universally agreed upon choice.
The options are permutations of:
- Integer signedness.
	- Use unsigned wherever a value should never be negative.
	- Use signed even when a value should never be negative, including indices.
- Whether to use `-Wsign-conversion` or not.
- Whether to cast signed → unsigned every time we use a standard library container.
- Whether to wrap standard library containers in sign-ness-changing views.

We can have different priorities when making this decision, and put different weight one each from project to project.
- Robustness: It should be difficult to write bugs and easy to identify them when they happen
- Readability: The code should makes sense. What ever that means.
- Runtime performance: The code should execute as fast as possible.
- Memory usage: Always use the smallest type possible.
- Avoid undefined behavior.
- Undefined behavior one error: It is diagnosable with an undefined behavior sanitizer.

(
TODO For each advantage / disadvantage describe how it affects the priorities listed above.
)

In this note `integer_t` is an integer type that is an alias for either `std::size_t` or `std::ptrdiff_t` depending on if we use signed or unsigned indexing.
In this note `unsigned_integer_t` is an alias for `std::size_t`.
In this note `signed_integer_t` is an alias for `std::ptrdiff_t`.
In this note `Container` represents any container type that has `std::size_t size() const` and `T& operator[](size_t)` member function, for example `std::vector`.
In this note, `isValidIndex` is the following overloaded function, unless context makes it clear that something else is meant:
```cpp
template <typename Container>
bool isValidIndex(const Container& container, std::size_t index)
{
	return index < container.size();
}

template <typename Container>
bool isValidIndex(const Container& container, std::ptrdiff_t index)
{
	// Negative indices are never valid.
	if (index < 0)
	{
		return false;
	}

	// Sanity check of the container, _probably_ not necessary.
	// Can the below error report ever trigger?
	const std::size_t max_possible_index =
		static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());
	if (container.size() > max_possible_index)
	{
		report_error("Container too large for signed indexing");
	}

	return index < std::ssize(container);
}
```

In this note "arithmetic underflow" refers to an arithmetic operation that should produce a result that is smaller than the smallest value representable by the return type.
For a signed type that is some large negative value.
For an unsigned type that is 0.

"Arithmetic overflow" refers to an arithmetic operation with an unrepresentable result, just like underflow, but at the high end of the integer type's range instead of the low end.

Signed numbers are not allowed to overflow or underflow, that is considered [[Undefined Behavior]].
Unsigned numbers are allowed to overflow and underflow and wrap around.
This is called modular arithmetic.

Because overflow of signed integers is undefined behavior the compiler does not need to consider this case when generating machine code.
This can in some cases produce more efficient code (`citation needed`).

Since there are a number of bugs stepping from unintended conversions between signed and unsigned integer types many compilers include warnings for this, often enabled with `-Wconversion`, `-Wsign-conversion`, and/or `-Wsign-compare`.

It has been difficult to find or come up with good illustrative examples that demonstrates the various problems described in this note.
Examples are often trivial and hard to map to real-world production code.

# Common Bugs

## Incorrect Type Of Loop Control Variable

Often `int` [(13)](https://www.sandordargo.com/blog/2023/10/18/signed-unsigned-comparison-the-most-usual-violations).
This is not terribly bad as long as the container isn't very large.
The counter will only ever be positive and a positive `int` can always be implicitly converted to `std::size_t` without loss.
This will work as expected until the container holds more elements than the largest possible `int`.

```cpp
void work(Container& container)
{
	for (int index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

## Implicit Conversion From Signed To Unsigned

The compiler will implicitly convert signed values to unsigned when a signed and an unsigned operands of the same size are passed to an operator.
When `integer_t` is a signed type in the below example the operator is `>=` and the signed `index` function parameter is converted to the same unsigned type as `data.size()`.
When `index` has a negative value the result of the conversion is a large negative value, which often will be larger than the size of the container and we enter the error reporting code block.

When `integer` is a unsigned type the conversion from a negative value to a positive value happens in the initialization of `index` in `work_on_element` instead so the effect is the same.
The difference is the lie in the signature of `work` in the singed `integer_t` case.
The function claims to handle negative values when in reality it does not.

```cpp
void work(Container& data, integer_t index)
{
	if (index >= data.size())
	{
		report_error("Index passed to work out of range.");
		return;
	}

	// Work on data[index].
}

void work_on_element(Container& data)
{
	const integer_t index {-1};

	// This will log the error message even though index is not larger
	// than data.size().
	work(data, index);
}
```

If we want to use a signed type then we can use `std::ssize(container)` instead of `container.size()` to get the size, which will give us the size as a signed type.
In that case there will be no sign conversion and the comparison will work as expected, i.e. `-1 >= std::ssize(data)` will always evaluate to false.

When using singed `integer_t` the range check in `work` should check both sides:
```cpp
if (index < 0 || index >= data.size())
```

[Some say](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/comment/lfymu23/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button) , including the C++ Core Guidelines [(3)](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic) that this is not a problem, that we should pass `-Wno-sign-conversion` to the compiler, ignore the problem, and hope that it never happens.
This is OK if we take care to never let a negative value be converted to an unsigned integer type.
I don't know how to ensure that.
I would prefer to keep the sign-conversion warning enabled, but I'm not sure that is realistic with an signed `integer_t` type.

## Changing The Type Of A Variable Or Return Value


## /


To make it clear when a conversion is happening, always use `static_cast`.

A strategy for avoiding many of the above bugs is to not use indices at all and instead prefer range based loops, named algorithms, iterators, the ranges library, ... (TODO More tips?).
A drawback of iterators is that for some containers, such as `std::vector`, they can become invalidated at times where an index would not.

Instead of passing an array and a size, pass a view and use ranged based loop instead of an indexing loop:
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

# Standard Library

Many containers in the standard library use `size_t`, which is an unsigned type, for indexing and counts.
I assume that the standard library designers know what they are doing.
It is a sign indicating that we should do the same, and that we should use unsigned integer when working with the standard library.
Not everyone agrees [(9)](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/)[(7)](https://google.github.io/styleguide/cppguide.html#Integer_Types), not even some of the standard library maintainers (`citation needed`) and committee members [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf).
It may be that it was the correct decision at the time, but things changed (e.g. 16-bit to 32-bit to 64-bit CPUs.) and now we are stuck with what we have for legacy / consistency / backwards compatibility reasons.
The design of any API that is difficult to change by definition happens before widespread usage of that API, and it is not until widespread usage we we discover real-world implications that wasn't though of beforehand.

There is `std::ssize(const T& container)` that returns the size of the container as a signed integer.

I don't know of any way to index with a signed index, other than passing in the signed index and letting it implicit convert to the unsigned equivalent or do the cast explicitly.
I've seen the following recommendations:
- Don't do direct indexing and instead use ranged base for loops,  iterators, named algorithms, ranges, ... (TODO More to add here?).
- Pass the signed value to `operator[]`.
	- This will work as long as the value is non-negative, so make sure you have a check for that or some other way to guarantee it.
	- Disable the `sign-conversion` warning since every indexing will trigger it.
- Provide a short-named conversion function.
	- Such as `std::size_t toUZ(std::ptrdiff_t)`.
	- Use as `container[toUZ(index)]`.
- Use a view that provide signed size and element access.
	- Possibly using `toUZ` and `fromUZ` internally.

`std::next` and `std::prev` uses a signed type.

The standard containers will assert (Or throw, no sure which.) if an out-of-bounds index is passed to `operator[]` in debug builds.
If you want bounds checking in release builds then use `at` instead of `operator[]`.



# When To Use Signed And When To Use Unsigned

In some cases it is clear which variant should be used.
If you need modular arithmetic and don't need negative numbers then use unsigned.
If the value doesn't represent a number but a bit field, flags, a hash, or an ID then also use unsigned.
The determining factor is whether the values will be used for arithmetic beyond bit operations.
A topic of contention is what to use for values that should never be negative but are used in arithmetic expressions.
For example counts and array indices.
One recommendation is to use unsigned to signal that the value should never be negative.
Another recommendation is to always use signed even in this case.
- [_Google C++ Style Guide_ > _Integers_](https://google.github.io/styleguide/cppguide.html#Integer_Types)
- [_Learn C++_ > _4.5 — Unsigned integers, and why to avoid them_ @ learncpp.com](https://www.learncpp.com/cpp-tutorial/unsigned-integers-and-why-to-avoid-them/)


# Advantages Of Unsigned

[The idea that 'signed types are safer' is nonsense](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/comment/lfzonsz/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button).

## The Natural Type To Use

Using unsigned is a natural choice when working with non-negative quantities such as indices and counts [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf).

## Makes Invalid Values Unrepresentable

Restricting what values can be passed to a function through the type system is a good way to communicate how the function is meant to be used to both programmers and the compiler.
At least it would be if we didn't have implicit signed → unsigned conversions.
And if arithmetic over- and underflow was an error instead of wrapping.

## Larger Positive Range

An unsigned integer can address twice as many container elements as an equally-sized signed integer can.
If you don't need a sign then don't spend a bit on it.
When no negative numbers are required, unsigned integers are well-suited for networking and systems with little memory, because unsigned integers can store more positive numbers without taking up extra memory.
You get double the range of positive values compared to an equally sized signed type.
This may be important when the index type is small [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), such as 16 or possibly even 32-bit in some cases.
I'm not sure when this will become a restriction for 64-bit signed indices, the largest `std::ptrdiff_t` value is 9'223'372'036'854'775'807.

If we chose a signed integer instead then we need to motivate the loss of maximum size.

## Single-Comparison Range Checks

Only need to check one side of the range for indexing.

```cpp
// Unsigned index.
if (index >= container.size())

// Signed index.
if (index < 0 || index >= container.size())
```

This assumes the computation of `index` didn't underflow, which is impossible to detect after the fact.
See _Disadvantages Of Unsigned_ > _Impossible To Detect Underflow_ for a longer discussion on this.

## Most Values Are Positive And Positive Values Rarely Mix With Negative Values

I'm not sure this is true.
[Link](https://www.reddit.com/r/cpp_questions/comments/1ej5mo0/comment/lgcbrh0/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button).

# Advantages Of Signed

## Less Surprising Behavior

Signed integer behaves as most people expect number to behave most of the time.
`x - y` is negative when `y > x`.
`x - 1` is less than `x`  for pretty much every value that will show up in practice,
but it is not true for the most common unsigned number: 0.

Using the type that is more similar to our intuition of how numbers work makes it faster to teach new programmers, and even intermediate programmers will make fewer mistakes (citation needed).

## Can Detect Unintended Negative Values

When an expression unintentionally produces a negative values we can detect that by simply checking if the result is smaller than 0.
With unsigned integers we can't do that since there can't be any negative values.
The only thin we can do is detect unexpectedly large positive values.
But where do we draw the line?
See _Disadvantages Of Unsigned_ > _Impossible To Detect Underflow_ for a longer discussion on this.

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
	work(100000, 100);
}
```

In this case `work` will never return because `x`, being a `short`, can never be equal or larger to `100000`.
Debugging will required to find the cause for the freeze.
Had this been done with signed values instead then an undefined behavior sanitizer would have detected and reported the overflow and we would immediately know what the problem was.

For a motivation for why `max` and `x` has different sizes consider a scenario where `max` is actually `std::vector<VertexPosition>::size()` and `x` is the start index in a collection of meshlets each with 100 vertices that are being batched processed, and where it is assumed that each meshlets collection doesn't have more than a few thousands of vertices (So that `unsigned short` is large enough and we want to keep the type small since there may be a large number of meshlets.), and `work`, below `process_meshlets` is responsible for processing a consecutive subset of those meshlets for a limited amount of time:
```cpp
#include <chrono>
using namespace std::chrono;

constexpr unsigned short VERTICES_PER_MESHLET {100};

unsigned short process_meshlets(
	const std::vector<VertexPosition>& positions,
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
		start_index += VERTICES_PER_MESHLET;
	}
	return start_index;
}
```

## The Underflow Is Farther Away From Common Numbers

With a signed integers we have a larger margin from commonly used values to the underflow edge.
Small mistakes are unlikely to cause and underflow.



## Backwards Loops Easier To Write

The following intuitive loop only works with signed integers since the `index >= 0` condition only makes sense for signed integer..

```cpp
void work(const Container& container)
{
	for (integer_t index = container.size() - 1; index >= 0; index--)
	{
		// Work on container[index];
	}
}
```

The following strided loop that handles the condition problem above fails half of the time, when `i` goes from `1` to `18446744073709551615`, with unsigned integers, while with signed integers the loops works just fine.
```cpp
void work(const Container& container)
{
	for (integer_t i = container.size(); i > 0; i -= 2)
	{
		const integer_t index = i - 1
		// Work on container[index];
	}
}
```

## Tools Can Report Underflow And Overflow

Tools, such as sanitizers, can report underflow and underflow on signed arithmetic operations.
In most cases the user did not intend for the operation to under- or overflow.
Tools cannot do this for unsigned operations in general since the behavior is defined in those cases, it is not an error.


## Less Mixing Of Signed And Unsigned

One source of bugs is when signed and unsigned values are mixed in an expression [(7)](https://google.github.io/styleguide/cppguide.html#Integer_Types), [(12)](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities).
This leads to implicit conversions and results that are difficult to predict for many programmers.
Assuming we are required to use signed values for some variables, some data is inherently signed [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), it follows that we want to also use a singed type for any value that is used together with the inherently signed data.
This process repeats and a front of the signed-ness spreads like a virus across the code base until everything, or at least most things, are signed.
Every time we introduce a variable with an unsigned integer type we risk introduce mixing with the must-be-signed values and thus introduce bugs.

Unfortunately the reverse of this is also true since the standard library uses a lot of unsigned types in its container.
This means that a similar front of unsigned-ness spreads in the opposite direction.
Trouble and headaches happen where these fronts meet.
Unless we chose to use a different set of containers that use a signed integer type instead.

A goal should be to keep this front-of-mixed-signed-ness as small as possible.

A counter-point is that if a function is so large that it becomes difficult to track which variables are signed and which are unsigned then the function should be split anyways.
Trying to force everything to the same signedness is a false sense of security.

Example error case:
```cpp
void work(Container& container, std::ptrdiff_t index)
{
	if (index < container.size())
	{
		report_error("Invalid index passed to work.");
		return;
	}

	// Work with container[index].
}
```

This will fail to reliably report an error for negative `index` even though `container.size()` can never be less than 0 so ALL negative `index` is smaller than ALL `container.size()`.
This is because the comparison happens after implicit conversion, and the conversion is signed → unsigned [(12)](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities). This means that `index < container.size()` compares the container size against some potentially very large positive number.

Less trivial variant:
```cpp
void work(
	Container& container,
	unsigned int base_index, signed int block_offset, unsigned int block_size)
{
	signed int offset {block_offset * block_size};
	if (offset < base_index)
	{
		// Code for processing elements before base_index.
	}
	else
	{
		// Code for processing elements at or after base_index.
	}
}
```

The if statement condition is essentially the same as the common `index < container.size()` that we know to be weary of when `index` is a signed integer, but the problem is easier to miss since the pattern is unfamiliar.

One place where the type is not visible is in functions calls.
At the call site we cannot see the type of the parameter.
This means that we can unknowingly pass a signed integer that is implicitly converted to an unsigned one.
It is is difficult for the called function to detect if large positive values are also valid use cases.
We can chose to enable compiler warnings for this, e.g. `-Wsign-conversion`, but some advocates for using signed integers advice against enabling this warning in order to make interacting with the standard library easier.
They want to following code, where `container[index]` contains an implicit signed → unsigned conversion, to be warning-free:
```cpp
void work(Container& container)
{
	for (integer_t index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

## Mostly Safe Conversions To Unsigned

It is usually safe to pass a signed integer value in a call to a function taking an unsigned integer parameter.
This includes `operator[]` in the standard library containers.
We need to ensure that the value is in range of the container, but that is true regardless.
We can enable warnings for when we do this if we want, `-Wsign-conversion`, but we can also chose not to enable those and just let the conversion happen.

This is fine even with a negative offset:
```cpp
bool work(
	Container& container,
	signed int base, signed int offset, signed int stride
)
{
	signed int index = base + offset * stride;
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
	unsigned int base, signed int offset, unsigned int stride
)
{
	// Here be footguns.
	unsigned int index = base + offset * stride;
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

## More Opportunities For Compiler Optimizations

Since signed integer under- and overflow is undefined behavior the compiler can optimized 
based on this knowledge.



(
TODO List cases where this leads to a measurable performance improvement.
)
(
TODO Are there cases where the same is true also for unsigned integers?
)


## Recommended By The C++ Core Guidelines And Other "Authorities"

An appeal to authority argument but I include it anyway to counter the "the standard library designers chose unsigned and they are experts" argument in favor of unsigned integers.
The relevant factual takeaways from the C++ Core Guidelines has been incorporated in the rest of the _Advantages Of Signed_ subsections. The same is true for the Google C++ Style Guide and other documents listed below.

See
- [_C++ Core Guidelines_ > _Arithmetic_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#arithmetic)
- [_Google C++ Style Guide_ > _Integers_](https://google.github.io/styleguide/cppguide.html#Integer_Types)

# Disadvantages Of Unsigned

## Unsigned Integer Does Not Model Natural Numbers

They model modular arithmetic.[(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf)
Can have surprising conversions to/from signed integers, see _Advantages Of Signed_ > _Less Mixing Of Signed And Unsigned_.

## The Underflow Edge Is Close To Common Numbers

Programs very often deal with with zero and nearby values.
That is right at the edge of underflow.
Even a small mistake is enough to fall over the edge.
This makes reverse loops non-trivial to write, the following classical example does not work:
```cpp
void work(const Container& container)
{
	for (unsigned int index = container.size() - 1; index >= 0; ++index)
	{
		// Work with container[index];
	}
}
```

This is better:
```cpp
void work(const Container& container)
{
	for (unsigned int i = container.size(); index > 0; ++i)
	{
		const unsigned index = i - 1;
		// Work with container[index];
	}
}
```

Another option is something based on [`std::reverse_iterator`](https://en.cppreference.com/w/cpp/iterator/reverse_iterator) or [`std::ranges::reverse_view`](https://en.cppreference.com/w/cpp/ranges/reverse_view).

## Impossible To Detect Underflow

It is impossible to detect earlier arithmetic underflow, other than with heuristics.
By the time we get to the `work` function the damage has already been done.
In the code below the programmer decided that the container passed to `work` should never contain more than `VERY_LARGE_NUMBER`, some constant defined somewhere, and any index argument larger than that is a sign that we had an underflow in the computation of that argument.

```cpp
void work(const Container& container, integer_t index)
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
The larger we set it the bigger containers we support, but we risk missing underflows that wrap past the boundary and incorrectly enter into the legal range again.

Signed types also has the same problem, but it is less frequent in practice (`citation needed`) since the underflow happens a much farther away from commonly used numbers.
For unsigned types the underflow happens near 0 and a lot of real-world arithmetic is done near 0.
Also, underflow with signed integer types is undefined behavior.

The result is that using a signed integer type is similar to setting `VERY_LARGE_NUMBER` to the halfway-point between 0 and the maximum representable value.
So when using unsigned integers for indices we have the flexibility to set `VERY_LARGE_NUMBER` larger than that and thereby support larger containers.

A signed integer underflow is just as difficult to test for as an unsigned one.
The difference is that the edge where the underflow happens is much farther away from common values for signed integers than it is for unsigned integers.
For unsigned integers the edge is at 0, for signed integers it is at some large negative number.

## Implicit Conversion Of Signed Values Leads To Bugs

[(10)](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/)

For more on this, see _Advantages Of Signed_ > _Less Mixing Of Signed And Unsigned_.

## Backwards Loops Non-Trivial To Write

An operation that is often written incorrectly with unsigned types is backwards loops.
```cpp
void work(const Container& container)
{
	for (integer_t index = container.size() - 1; index >= 0; index--)
	{
		// Work on container[index];
	}
}
```

`index >= 0` will always be true when `index_t` is unsigned.

A less obvious variant is
```cpp
void work(const Container& container)
{
	for (integer_t i = container.size(); i > 0; i -= 2)
	{
		const integer_t index = i - 1
		// Work on container[index];
	}
}
```

Here the condition is `i > 0` so we will stop when we reach 1, which works when walking with stride 1.
But here we work with stride 2 and may miss 0 and instead wrap back up the very large values when using unsigned integer.
There error will happen half of the time, on average, assuming the contains has a random number of elements each time.
With a singed integer we instead compare `-1 > 0` and things work as we intended.


# Disadvantages Of Signed

## Difficult To Interact With The Standard Library

The standard library uses unsigned integer types for many things, in particular `size_t size() const` and `T& operator[](size_t index)`.
This makes working with the standard library with `integer_t` being signed difficult since we are forced to mix signed and unsigned values in expressions and get sign-conversion warnings all over the place if `-Wsign-conversion` is enabled,
or explicitly cast our index variables from signed to unsigned on every container access.
This creates verbose, noisy, and difficult to read code.
```cpp
void work(Container& container)
{
	for (std::ptrdiff_t index = 0; index < container.size(); ++index)
	{
		// Work with container[static_cast<std::size_t>(index)].
	}
}
```

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


## Require Two Comparisons For Range Checks

```cpp
// Signed index.
if (index < 0 || index >= container.size())

// Unsigned index.
if (index >= container.size())
```

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


## Under- And Overflow Is Undefined Behavior

This means that we must do all checks before any arithmetic operation.
We cannot detect errors by checking the result.
The following cannot be used to detect overflow:
```cpp
bool work(Container& container, signed int base, signed int offset)
{
	signed int index = base + offset;
	if (index < base)
	{
		report_error("Overflow in work.");
		return false;
	}

	// Work on container[index].
}
```



## The Modulus Operator Behavior Can Be Unexpected For Negative Numbers

A common way to detect odd numbers is `n % 2 == 1`.
This does not work for negative `n` since the result is `-1` instead of `1`.
Use `n % 2 != 0` instead.

# Alternatives To Indexing

- Range based for loops.
- The ranges library.
- Named algorithms.
- Iterators.
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
