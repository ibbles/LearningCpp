A signed integer type is one that can represent both positive and negative numbers, and zero.
An unsigned integer is one that can only represent positive numbers, and zero.

In this note `integer_t` is an integer type that is either `std::size_t` or `std::ptrdiff_t` depending on if we use signed or unsigned indexing.
Or some other pair of signed/unsigned integers.
In this note `Container` represents any container type that has `std::size_t size() const` and `T& operator[](size_t)` member function, for example `std::vector`.

In this note "arithmetic underflow" refers to an arithmetic operation that should produce a result that is smaller than the smallest value representable by the return type.
For a signed type that is some large negative value.
For an unsigned type that is 0.

"Arithmetic overflow" refers to an arithmetic is just like underflow, but at the high end of the integer type's range instead of the low end.

Signed numbers are not allowed to overflow or underflow, that is considered [[Undefined Behavior]].
Unsigned numbers are allowed to overflow and underflow and wrap around.
This is called modular arithmetic.

Because overflow of signed integers is undefined behavior the compiler does not need to consider this case when generating machine code.
This can in some cases produce more efficient code (citation needed).

Since there are a number of bugs stepping from unintended conversions between signed and unsigned integer types many compilers include warnings for this, often enabled with `-Wconversion`, `-Wsign-conversion`, and/or `-Wsign-compare`.


# Common Bugs


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

There is `std::ssize(const T& container)` that returns the size of the container as a signed integer.

I don't know of any way to index with a signed index.
This is a major problem.
The only recommendation I've seen is to not do direct indexing and instead use ranged base for loops,  iterators, named algorithms, ranges, ... (TODO More to add here?).

`std::next` and `std::prev` uses a signed type.


# When To Use Signed And When To Use Unsigned

In some cases it is clear which variant should be used.
If you need modular arithmetic and don't need negative numbers then use unsigned.
If the value doesn't represent a number but a bit field, flags, a hash, or an ID then also use unsigned.
The determining factor is whether the values will be used for arithmetic beyond bit operations.
A topic of contention is what to use for values that should never be negative but are used in arithmetic expressions.
For example counts and array indices.
One recommendation is to use unsigned to signal that the value should never be negative.
Another recommendation is to always use signed even in this case

# Advantages Of Unsigned

Using unsigned is a natural choice when working with non-negative quantities such as indices and counts.

If you don't need a sign then don't spend a bit on it.
You get double the range of positive values compared to an equally sized signed type.

[The idea that 'signed types are safer' is nonsense](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/comment/lfzonsz/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button).

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


# Advantages Of Signed

## Less Surprising Behavior

Signed integer behaves as most people expect most of the time.
`x - y` is negative when `y > x`.

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

One source of bugs is when signed and unsigned values are mixed in an expression.
This leads to conversions and results that are difficult to predict for many programmers.
Assuming we are required to use signed values for some variables, some data is inherently signed, it follows that we want to also use a singed type for any value that is used together with the inherently signed data.
This process repeats and a front of the signed-ness spreads like a virus across the code base until everything, or at least most things, are signed.
Every time we introduce a variable with an unsigned integer type we risk introduce mixing with the must-be-signed values and thus introduce bugs.

Unfortunately the reverse of this is also true since the standard library uses a lot of unsigned types in its container.
This means that a similar front of unsigned-ness spreads in the opposite direction.
Trouble and headaches happen where these fronts meet.
Unless we chose to use a different set of containers that use a signed integer type instead.

A goal should be to keep this front-of-mixed-signed-ness as small as possible.


# Disadvantages Of Unsigned

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

Signed types also has the same problem, but it is less frequent in practice (citation needed) since the underflow happens a much farther away from commonly used numbers.
For unsigned types the underflow happens near 0 and a lot of real-world arithmetic is done near 0.
Also, underflow with signed integer types is undefined behavior.

The result is that using a signed integer type is similar to setting `VERY_LARGE_NUMBER` to the halfway-point between 0 and the maximum representable value.
So when using unsigned integers for indices we have the flexibility to set `VERY_LARGE_NUMBER` larger than that and thereby support larger containers.

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

The standard library uses singed integer types for many thing, in particular `size_t size() const` and `T& operator[](size_t index)`.
This makes working with the standard library with `integer_t` being signed difficult since we are forced to mix signed and unsigned values in expressions and get sign-conversion warnings all over the place.

The C++ Core Guidelines are conflicted on this issue.
[ES.100](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es100-dont-mix-signed-and-unsigned-arithmetic) says that we should not mix signed and unsigned integers and [ES.102](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic) says that we should use signed integers for indices.
This is a contradiction since the singed indices will be mixed with unsigned standard library containers.
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

In the above the expressions `index < data.size()` mixes signed and unsigned integers, and `data[index]` passes a singed value to an operator expecting an unsigned argument.
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
# References

- 1:  [_Should I use unsigned types? Or should I turn off Wconversion?_ @ reddit.com/cpp](https://www.reddit.com/r/cpp_questions/comments/1ehc50j/should_i_use_unsigned_types_or_should_i_turn_off/)
- 2: [_C++ Core Guidelines_ > _ES.100: Don’t mix signed and unsigned arithmetic_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es100-dont-mix-signed-and-unsigned-arithmetic)
- 3: [_C++ Core Guidelines_ > _ES.102: Use signed types for arithmetic_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic)
- 4: [_C++ Core Guidelines_ > _ES.106: Don’t try to avoid negative values by using unsigned_](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es106-dont-try-to-avoid-negative-values-by-using-unsigned)
- 5: [_C++ Core Guidelines_ > ES.107: Don’t use unsigned for subscripts, prefer gsl::index](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es107-dont-use-unsigned-for-subscripts-prefer-gslindex)
