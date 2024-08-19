(
TODO
Rewrite the sectioning to not be pros/cons signed/unsigned since there is too much repetition.
Instead describe one use-case / concept at the time and evaluate the consequences with signed and unsigned integers together.
This makes it easier to place considerations that doesn't fit neatly into a pro or con for either of them, such as the `auto mid = (low + high) / 2`  discussion in [25](https://graphitemaster.github.io/aau#computing-indices-with-signed-arithmetic-is-safer), and others in the same text.
)


(
TODO 
Find where to put the following loop, demonstrating division in the index calculation.
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
)

(
TODO
Find where to put the following loop, demonstrating iterating over a sub-range [(28)](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/).
```cpp
void work(Container& container, std::ptrdiff_t start_skip, std::ptrdiff_t end_skip)
{
	for (integer_t index = start; index < container.size() - end_skip; ++index)
	{
		// Work with contianer[index].
	} 
}
```

The above fails for unsigned index when `end_skip > container.size()` because `container.size() - end_skip` underflows, producing a very large number, and the loop will visit not only container elements at the end that should be skipped,
it also buffer overruns.
With an unsigned index we must first check that we shouldn't skip the entire loop.
```cpp
void work(Container& container, integer_t start_skip, integer_t end_skip)
{
	if (end_skip >= container.size())
		return;

	for (integer_t index = start; index < container.size() - end_skip; ++index)
	{
		// Work with contianer[index].
	} 
}
```

The initial `std::ptrdiff_t` loop fails if `start_skip` or `end_skip` is negative,
since that will cause it to index out of bounds of the container.

An even simpler, and I guess more common, variant is to visit all but the last element of a container.
```cpp
void work(Container& container)
{
	for (integer_t index = 0; index < container.size() - 1; ++index)
	{
		// Work with container[index].
	}
]
```

This fails with unsigned integers for empty containers.
`container.size() - 1` becomes `0 - 1` which wraps around to a very large number,
which causes the loop to run too many iterations.

A signed `integer_t` doesn't save us here if `Container` is an `std::vector` since the problem is because `std::vector::size` has an unsigned return type.
We must either explicitly check for the empty container case or make sure the end-of-range computation is done using a signed type.
```cpp

// Handle empty container separately before the loop.
template <typename T>
void work(std::vector<T>& container)
{
	if (container.empty())
		return;

	for (integer_t index = 0; index < container.size() - 1; ++index)
	{
		// Work with container[index].
	}
}

// Use std::ssize instead of .size().
template <typename T>
void work(std::vector<T>& container)
{
	for (std:ptrdiff_t index = 0; index < std::ssize(container) - 1; ++index)
	{
		// Work with container[index].
	}
}

// Cast the size before the subtraction.
template <typename T>
void work(std::vector<T>& container)
{
	for (std:ptrdiff_t index = 0;
		index < static_cast<std::ptrdiff_t>(container.size()) - 1;
		++index)
	{
		// Work with container[index].
	}
}
```

Another variant is to not do the subtraction at all, and instead do addition on the other size of the less-than operator.
```cpp
void work(Container& container)
{
	for (integer_t index = 0; index + 1 < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```
)


(
TODO
Find where to put the following, demonstrating unexpected widening [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
This is similar to _Impossible To Detect Underflow_, but we compute the large value instead of being given it.
```cpp
std::uint32_t a {10};
std::uint32_t b {11};
std::int64_t c = a - b;
// c is now std::numeric_limits<std::uint32_t>::max() despited being stored
// in a signed variable. The problem is that we change the size. Had both
// been 32-bit or 64-bit then it would have been fine.
```
)


(
TODO
Find where to put the following, demonstrating that we cannot compute distances between values like normally would when using unsigned integers.

This looks like it would give us the distance between `v1` and `v2`,
and in some sense it does,
but it doesn't give the shortest distance and the `std::abs` will trigger a compiler error [(32)](https://stackoverflow.com/questions/47283449/should-signed-or-unsigned-integers-be-used-for-sizes).
```cpp
void work(std::size_t v1, std::size_t v2)
{
	std::size_t distance = std::abs(v1 - v2);
}
```
The problem is that `v1 - v2`, with unsigned `v1` and `v2`, produces an unsigned result.
If `v1 < v2` then we wrap around and get a very large positive value.
The distance from `v2` to `v1` if we start walking away from `v1`, hit the upper bound, wrapped back down to zero, and finally continued on to `v1`.
This is probably not what was intended.
The `std::abs` doesn't save us because by the time the argument has been computed we already have a too large value, since the expression cannot ever be negative.
Also, `std::abs` doesn't make any sense for an unsigned type since if we were to implement it it would be the identity function.
For this reason the standard library designers opted to not provide that overload,
giving a compiler error to inform us that we are probably not doing what we thing we are doing.
One way to compute the distance is `std::max(v1, v2) - std::min(v1, v2)`.

With signed integers the original computation works as intended.
```cpp
void work(std::ptrdiff_t v1, std::ptrdiff_t v2)
{
	std::ptrdiff_t distance = std::abs(v1 - v2);
```
)


The purpose of this note is to evaluate the advantages and disadvantages of using signed or unsigned integers, mainly for indexing operations.
Both variants work and all problematic code snippets can be fixed using either type.
They are what multiple authors and commentators could call "bad code, bad programmer".
But both families of types have problem areas and errors can occur with both [(34)](https://youtu.be/Fa8qcOd18Hc?t=3080).
The aim has been to find examples where the straight-forward way to write something produces unexpected and incorrect behavior.
If what the code says, or at least implies after a quick glance, isn't what the code does then there is a deeper problem than just "bad code, bad programmer".
Real-world programmers writing real-world programs tend to have other concerns in mind than the minutiae of integer implicit conversion rules and overflow semantics.
We want tools, language and compiler included, that help us prevent errors [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).

For this discussion we assume a 64-bit machine with 32-bit `int`.
Some results are different on other machines.

Fixed sized integer types, which all primitive integer types are, have the drawback that they can only represent a limited range of values.
When we try to use them outside of that range they wrap, trap, saturate, or trigger undefined behavior [(27)](https://blog.regehr.org/archives/1401).
In most cases none of these produce the result we intended, and often lead to security vulnerabilities [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc), with exceptions for algorithms that explicitly need wrapping behavior such as some encryption algorithms.
A signed integer type is one that can represent both positive and negative numbers, and zero.
An unsigned integer is one that can only represent positive numbers, including zero.
The purpose for this note is to explore the pros and cons of using each in cases where it is not obvious, such as for sizes and indexing.
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
- Undefined behavior on error: It is diagnosable with an undefined behavior sanitizer.
	- Though being undefined behavior means the compiler may do unexpected transformations to our code, meaning that the overflow might not actually happen at runtime. Instead the application does something completely different.

(
TODO For each advantage / disadvantage describe how it affects the priorities listed above.
)

In this note `integer_t` is an integer type that is an alias for either `std::size_t` or `std::ptrdiff_t` depending on if we use signed or unsigned indexing.
`std::size_t` is an unsigned integer large enough to hold the size of any object,
including heap allocated buffers.
`std::ptrdiff_t` is a signed type used to represent the difference between two pointers.
It is "meant" to be large enough to hold the difference between any two pointers pointing within the same object, but that isn't necessarily true since `std::size_t` and `std::ptrdiff_t` typically have the same size and if we have an object with size `std::numeric_limits<std::size_t>::max()` and a pointer to the first byte and a pointer to the last byte then `std::numeric_limits<std::ptrdiff_t>::max()`  is smaller than the difference between them.
(
Is there any difference between `intptr_t` and `std::ptrdiff_t`?
On some machines it is, such as those with segmented memory.
Most modern machine have a flat memory address space.
)
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

A problem is that signed and unsigned expresses multiple properties and it is not always possible to get the exact combination that we want.
- Modular arithmetic / wrapping.
- Value may only be positive.
- Value can be negative.
- Over / underflow not possible / not allowed.

# Dangers

- Implicit type conversions with unexpected results happening in unexpected places.
- Under- or overflow resulting in undefined behavior.
- Unexpected or unintended wrapping.
- Comparing signed and unsigned.


# Operations

## Loop Over Container

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

Since `container.size()` is often unsigned, specifically `std::size_t`, which is an indication that `integer_t` should be `std::size_t` as well.


## Test If Index In Range

[(28)](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/)

A common range is from zero to some positive number, such as the size of a container.
With a signed integer type we must check both the lower and upper end of the range.
With an unsigned integer we only need to check the upper range since the lower bound is built into the type.

```cpp
bool work(Container& container, std::ptrdiff_t index)
{
	if (index < 0 || index >= container.size())
		return false;

	// Work with container[index].

	return true;
}

bool work(Container& container, std::size_t index)
{
	if (index >= container.size())
		return false;

	// Work with container[index].

	return true;
```


## Iterate Backwards

When iterating backwards we need to use different loop iteration conditions.
With a signed loop counter we stop when the index becomes negative,
because that means that we have gone past the start of the container.
With an unsigned loop counter we stop when the index becomes larger than the container size,
because that means that the counter wrapped around at zero.

```cpp
// Signed loop counter.
void work(Container& container)
{
	for (std::ptrdiff_t index = container.size() - 1; index >= 0; --index)
	{
		// Workd with container[index].
	}
}

// Unsigned loop counter.
void work(Container& container)
{
	for (std::size_t index = container.size() - 1; index < container.size(); --index)
	{
		// Work with container[index].
	}
}
```

The signed variant checks for, to me at least, more intuitive condition since the intention when the code was written was to loop from the size down to, and including, zero.
The unsigned variant has a more familiar condition since it uses the same one for both forwards and backwards loops.
To reverse the loop direction with an unsigned loop counter we simply start at the other end instead of the  beginning and step the other direction.
To reverse the loop direction with a signed loop counter we must edit all three parts of the loop header.

We cannot use the `index >= 0` condition with an unsigned counter because that expression is always true, for any value of `index`.
We cannot use the `index < container.size()`  condition with a signed counter because it won't wrap at zero and negative indices will be passed to `container[index]`. Not good.

Another variant that works with unsigned indices is the following [(33)](https://stackoverflow.com/questions/10040884/signed-vs-unsigned-integers-for-lengths-counts).
```cpp
void work(Container& container)
{
	std::size_t index {container.size());
	while (index-- > 0)
	{
		// Work with container[index].
	}
}
```

Here we use the post-fix decrement operator within the loop condition.
This means that index starts one-past the actual index we want to work on, but it is decremented to a valid index in the loop header before it is first used to index into the container.
If there is no valid index, i.e. the container is empty, then `index` starts at 0 and the condition, which sees the initial value of 0, ends the loop immediately since 0 isn't larger than 0.
If the container is non-empty then we first get the size of the container, check it against 0 and find that we should enter the loop, the index is decrement to the last valid index, and then we use that index to access an element in the container.
Then the once-decremented index is tested against 0 and if still larger then we do another round in the loop.
It some point `index` becomes 1, which means that we are about the enter the last loop iteration.
The condition tests `1 > 0`, `index` is decremented to 0 and we access `container[0]`.
Then we do the last condition check with `index` being zero, which evaluates to false and the loop ends.
The final decrement still happens so at the end of the loop `index` is `std::numeric_limits<std::size_t::max()`.


# Common Bugs

## Incorrect Type Of Loop Control Variable

Often `int` [(13)](https://www.sandordargo.com/blog/2023/10/18/signed-unsigned-comparison-the-most-usual-violations).
This is not terribly bad as long as the container isn't very large.
The counter will only ever be positive and a positive `int` can always be implicitly converted to `std::size_t` without loss.
This will work as expected until the container holds more elements than the largest possible `int`.
At that point an undefined behavior sanitizer will report the overflow,
but it is unlikely that data sets of that size are included in unit tests run with sanitizers.

```cpp
void work(Container& container)
{
	for (int index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

Here is a variant that will loop forever if the container size is larger than the largest `unsigned int`,
visiting each element again and again a indefinite number of times.
A sanitizer typically don't report this because the this behavior might be intended.

```cpp
void work(Container& container)
{
	for (unsigned int index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```
## Implicit Conversion From Signed To Unsigned

The compiler will implicitly convert signed values to unsigned when a signed and an unsigned operand of the same size are passed to an operator without checking that the signed value is even representable in the unsigned type [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
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

Without warnings is can be difficult to find all the places where changing the type of a variable or function's return type changes the behavior of the code that uses it.
IDE's and other tools can help within the same project but projects that use our code is more difficult.
The developers on that project may not even be aware that the type changed.

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


# Illegal Operations

## Signed

## Unsigned

- Division by zero.
- Shift by more than the bit width.
- Multiplying two unsigned integers with a size smaller than `int` with a result out of range for `int`.
	- This is because integers smaller than `int` are promoted to `int` before use, which means that the operation is subjected to all the limitations of signed integer arithmetic, including overflow being undefined behavior. This is problematic with multiplication because, on a platform where `int` is 32-bit, there are `uint32_t` values that when multiplied produces a result larger than `std::numeric_limits<int>>::max`, and thus overflow, and thus undefined behavior.


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

Using unsigned is a natural choice when working with non-negative quantities such as indices and counts [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), [25](https://graphitemaster.github.io/aau/).

## The Type Used By Real Programmers

Unsigned unsigned integers is absolutely safe if you know what you are doing and make no mistakes [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
If you get burned by any of the pitfalls associated with using unsigned integers then you are a bad programmer [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
Good tools create weak programmers, programming should be tough [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).

## Makes Invalid Values Unrepresentable

Restricting what values can be passed to a function through the type system is a good way to communicate how the function is meant to be used to both programmers and the compiler.
It simplifies the written documentation required.

At least it would be good if we didn't have implicit signed → unsigned conversions.
And if arithmetic over- and underflow was an error instead of wrapping.
Passing a negative signed value into a function taking an unsigned parameter is a common source of bugs [(15)](https://youtu.be/Puio5dly9N8?t=2561),
making the parameter an unsigned integer type doesn't protect us from that unfortunately.

## Integrates Well With The Standard Library Containers

Since the standard library containers use `std::size_t`, it is not unreasonable that our code using then also should.
By making our own container classes similar to the standard library containers we make them familiar to other C++ programmers [(18)](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc).


## The Type Returned By `sizeof()`

By definition, `std::size_t` is a type large enough to hold the size of any object.
Therefore it is the type returned by `sizeof()`.
Therefore, any time you wish to work with the size of objects in bytes, you should use `std::size_t` [(18)](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc).
For consistency, we should also use `std::size_t` for other sizes, such as the number of elements in a container.


## Larger Positive Range

An unsigned integer can address twice as many container elements as an equally-sized signed integer can [(33)](https://stackoverflow.com/questions/10040884/signed-vs-unsigned-integers-for-lengths-counts).
If you don't need a sign then don't spend a bit on it.
When no negative numbers are required, unsigned integers are well-suited for networking and systems with little memory, because unsigned integers can store more positive numbers without taking up extra memory.
This may be important when the index type is small [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), [(18)](https://softwareengineering.stackexchange.com/questions/338088/size-t-or-int-for-dimensions-index-etc), such as 16 or possibly even 32-bit in some cases.
On 32-bit machines we don't want to be limited to 2 GiB `std::byte` buffers since we have up to 4 GiB of address space.
This is not a problem for element types larger than a single byte since by then even a signed integer is able to address all of memory due to the size multiplication.
I'm not sure when this will become a restriction for 64-bit signed indices, the largest `std::ptrdiff_t` value is 9'223'372'036'854'775'807.
For most modern applications on modern hardware the extra bit is not necessary [(15)](https://youtu.be/Puio5dly9N8?t=2561), does not come up in practice very much.
The limitation only comes into effect when the container contains single-byte elements such as char,
with any larger type with run out of addressable memory for the data before we run out of index values in a signed integer.

If we chose a signed integer instead then we need to motivate the loss of maximum size.

However, having the extra bit does not mean it is actually used [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).
At least [GCC's standard library implementation of `vector` is limited](https://github.com/gcc-mirror/gcc/blob/releases/gcc-11.4.0/libstdc%2B%2B-v3/include/bits/stl_vector.h#L1776) to the range of `std::ptrdiff_t`, and the same is stated under _Note_ on [cppreference.com/vector/max_size](https://en.cppreference.com/w/cpp/container/vector/max_size).

## Single-Comparison Range Checks

Only need to check one side of the range for indexing.

```cpp
// Unsigned index.
if (index >= container.size())

// Signed index.
if (index < 0 || index >= container.size())
```

This assumes the computation of `index` didn't underflow before we got here, which is impossible to detect after the fact.
See _Disadvantages Of Unsigned_ > _Impossible To Detect Underflow_ for a longer discussion on this.

This may come with a performance improvement dues to the smaller number of instructions,
but that is unlikely on a modern computer in most cases since the number of loads is the same and ALU saturation is rarely the limiting factor for execution speed [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf).

## Under- And Overflow Is Not Undefined Behavior

It's not too bad to have under- or overflow in our loop iterations because at least it isn't undefined behavior  [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).

A counter-point is that even if the computation of the bad index isn't undefined behavior,
using it may be, depending on what it is being used for.
Indexing into an array or `std::vector` would be undefined behavior.

Another counter-point is that by making under- and overflow undefined behavior we allow tools, such as undefined behavior sanitizers to find them.

## Underflow In Index Calculations Are Obvious

If an application occasionally miscalculates an index to be negative that might not be noticed if using signed integer for indexing other than difficult-to-diagnose bugs.
With unsigned integers for indexing the negative value becomes a very large value and likely a segmentation fault on the first use.

## Bit With Conversions Cheaper

If you mix values with different bit width then unsigned is more efficient because the conversion is a no-op.
With signed values one must perform sign extension when going from e.g. 32-bit to 64-bit [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post19).

On the other hand, with signed values the compiler may not need to do any conversion at all.
In some cases it can transform the code to use the target type form the start [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post23).
See _Advantages Of Signed_ > _More Opportunities For Compiler Optimizations_.

## Most Values Are Positive And Positive Values Rarely Mix With Negative Values

[25](https://graphitemaster.github.io/aau/)

I'm not sure this is true.
[Link](https://www.reddit.com/r/cpp_questions/comments/1ej5mo0/comment/lgcbrh0/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button).

# Advantages Of Signed

## Less Surprising Behavior

Signed integer behaves as most people expect number to behave most of the time.
`x - y` is negative when `y > x`.
`x - 1` is less than `x`  for pretty much every value that will show up in practice,
but it is not true for the most common unsigned number: 0.

Using the type that is more similar to our intuition of how numbers work makes it faster to teach new programmers, and even intermediate programmers will make fewer mistakes (citation needed).

Small negative numbers are more common than very large positive numbers.

Mixing signed and unsigned numbers adds even more surprising behavior.

## Can Detect Unintended Negative Values

When an expression unintentionally produces a negative values we can detect that by simply checking if the result is smaller than 0 [(30)](https://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf).
With unsigned integers we can't do that since there can't be any negative values [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone).
In the following the `Container::Container(std::size_t size)` constructor creates a container with `size` elements.
```cpp
std::ptrdiff_t f();
std::ptrdiff_t g();
Container container(f() - g());
```
If for some reason `f()` returns a values smaller than `g()` we get a negative value that is implicitly converted to the unsigned `std::size_t`.

There is no way to detect this after the fact, the only thing we can do is detect unexpectedly large positive values.
But where do we draw the line?
See _Disadvantages Of Unsigned_ > _Impossible To Detect Underflow_ for a longer discussion on this.

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

## Underflow Is Farther Away From Common Numbers

With signed integers we have a larger margin from commonly used values to the underflow edge [25](https://graphitemaster.github.io/aau/).
Signed values are well-behaved around zero, a common value [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
Small mistakes are unlikely to cause and underflow [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).
For unsigned types, that boundary point at the busiest spot, zero  [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).

Tough beware of malicious inputs that intentionally bring the program close to the domain boundaries [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
Just because the limits are far away from the common values doesn't mean that we can ignore them.
That is something that is easier to do, accidentally or not, if we don't expect them to ever appear.

Relying on the negative domain of signed integers to skip bounds checking is an appeal to luck, which is not something we should do.

## Backwards Loops Easier To Write

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

### Walking Image Rows Backwards

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

## Can Have Negative Intermediate Values

(TODO Find a good example of this.)

In some cases the index computation may produce intermediate negative values.
This is OK as long as we ultimately end up with a positive value being passed to `operator[]`.
This works with unsigned integers as long as the computation only involves operations that work as intended under modular arithmetic, additions and subtractions.
(Does multiplication work as well?)
The modular arithmetic ensures that the correct number of steps is taken in both directions regardless of any wrapping at either side.
It may make debugging more difficult since printing values won't show the "semantically correct" value but instead the wrapped value.
However, if we do divides on a supposedly negative, but actually very large, values then we will get an incorrect result.

```cpp
void work(
	Container& container,
	std::size_t base,
	std::size_t bonus,
	std::size_t penalty,
	std::size_t attenuation)
{
	std::size_t index = base + (bonus - penaly) / attenuation;
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

## Tools Can Report Underflow And Overflow

Tools, such as sanitizers, can report underflow and underflow on signed arithmetic operations.
In most cases the user did not intend for the operation to under- or overflow.
Tools cannot do this for unsigned operations in general since the behavior is defined in those cases, it is not an error.

I imagine we can have tools that can be configured to report unsigned wrapping if we want to,
but since there are cases where we actually want wrapping this is not something that can be done in general.
We may want the check in some parts of the code but not others.

## Less Mixing Of Signed And Unsigned

One source of bugs is when signed and unsigned values are mixed in an expression [(7)](https://google.github.io/styleguide/cppguide.html#Integer_Types), [(12)](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities), [(15)](https://youtu.be/Puio5dly9N8?t=2561).
This leads to implicit conversions and results that are difficult to predict for many programmers.
Assuming we are required to use signed values for some variables, some data is inherently signed [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), it follows that we want to also use a singed type for any value that is used together with the inherently signed data.
This process repeats and a front of the signed-ness spreads like a virus across the code base until everything, or at least most things, are signed.
Every time we introduce a variable with an unsigned integer type we risk introduce mixing with the must-be-signed values and thus introduce bugs.

Unfortunately the reverse of this is also true since the standard library uses a lot of unsigned types in its container.
This means that a similar front of unsigned-ness spreads in the opposite direction.
Trouble and headaches happen where these fronts meet.
Unless we chose to use a different set of containers that use a signed integer type instead.

A goal should be to keep this front-of-mixed-signed-ness as small as possible.

Since we cannot, in many applications, avoid negative, and thus signed, integers it is not unreasonable to confine the unsigned types as much as possible.
As soon as we are given an unsigned value we check that it isn't too large and then cast it to signed.
As soon as we need to supply an unsigned value somewhere we check that the signed value we have isn't negative and then cast it to unsigned [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).

A counter-point is that if a function is so large that it becomes difficult to track which variables are signed and which are unsigned then the function should be split anyways.
Trying to force everything to the same signedness is a false sense of security.

A counter-point to that is that as programmers we absolutely do not want to discard whatever we currently have in our own working memory, for example while debugging, to start analyzing the impacts of possible implicit conversions, sign extensions, and wrapping behavior.
We want code to do what it looks like the code is doing.
We cannot memorize an ever-growing number of tricks and idioms to make loops using unsigned counters work correctly in all edge and corner cases.

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
	for (ptrdiff_t index = 0; index < container.size(); ++index)
	{
		// Work with container[index].
	}
}
```

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

Martin Beeger summarizes the mixing issue well  [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post15)

> ... the unsigned-signed promotion and comparison rules are just a as
> big hazard. We should never be forced to really think about them.
> This is possible through consistent use of one type of indexes, sizes,
> slices etc. And negative slices and strides have well-defined meaning
> and should be allowed, so they must be signed. Deciding on a
> case-by-case basis forces peoples into all the nitty gritty details of
> integer promotion and comparision, which is exactly what we should IMHO
> protect users from.


## Mostly Safe Conversions To Unsigned

It is usually safe to pass a signed integer value in a call to a function taking an unsigned integer parameter.
This includes `operator[]` in the standard library containers.
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

## More Opportunities For Compiler Optimizations

Since signed integer under- and overflow is undefined behavior the compiler can optimize based on this knowledge.
For example it can assume that `index + 1` is greater than `index` and that `container[index]` is next to both `container[index - 1]` and `container[index + 1]`.
This is not true for unsigned types since both `- 1` and `+ 1` can cause the value to wrap around.
This can make auto-vectorization more difficult [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).

With signed integers the compiler can simplify `10 * k / 2` to `5 * k`.
This is not possible with unsigned since the `10 * k` part can wrap.

Loops with fixed but unknown number of iterations can be optimized better with signed integers [(16)](https://www.youtube.com/watch?v=g7entxbQOCc).

The compiler can chose to use a larger sized signed integer type if it believes it will make the loop faster since it knows that the smaller sized integer won't overflow and the larger sized integer can hold all values that the smaller can hold [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing#post23), [(17)](https://youtu.be/yG1OZ69H_-o?t=2357).
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
The programmer can do this transformation itself when using unsigned integer, e.g. instead of using` std::uint32_t` use `std::uint64_t` on a 64-bit machine, or `std::size_t` to work on "any" machine [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
("any"in quotes because there may be machines where `std::size_t` is different from the native arithmetic type. You may consider using one of the `std::uint_fast*_t` types.)

Though there are some cases where unsigned provides better optimization opportunities.
For example division [(14)](https://eigen.tuxfamily.narkive.com/ZhXOa82S/signed-or-unsigned-indexing).



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


## Can Use Negative Values As Error Codes

E.g.
```cpp
void work(Container& container, T key)
{
	std::ptrdiff_t index = find(container, key);
	if (index < 0)
	{
		// switch block with a bunch of negateive cases.
		return;
	}

	// Work on container[index].
}
```

I don't like this much.
It overloads the semantics of the return value of `find`.
Better to keep the error reporting separate.
Either with an output parameter, returning a tuple or a struct, returning an optional (when we don't need multiple error values), or `std::expected`.

# Disadvantages Of Unsigned

## Unsigned Integer Does Not Model Natural Numbers

They model modular arithmetic.[(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf)
Can have surprising conversions to/from signed integers, see _Advantages Of Signed_ > _Less Mixing Of Signed And Unsigned_.


## Easy To Accidentally Write Conditions That Are Always True Or Always False

Such as `unsigned_value >= 0` [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
For example
```cpp
for (std::size_t i = container.size(); i >= 0; --i) { /* Do stuff. */
```

This is not illegal per the language, this code should compile without error.
You may get a warning, but that is just the compiler trying to be helpful.

See also _Advantages Of Signed_ > _Backwards Loops Easier To Write_.

## The Underflow Edge Is Close To Common Numbers

Programs very often deal with with zero and nearby values.
That is right at the edge of underflow [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).
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


## Impossible To Reliably Detect Underflow

It is impossible to detect earlier arithmetic underflow, other than with heuristics.
By the time we get to the `work` function the damage has already been done.
This is a common source of vulnerabilities and memory safety issues  [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc).

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

Signed types also has the same problem, but it is less frequent in practice (`citation needed`) since the underflow happens much farther away from commonly used numbers.
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

## Under- And Overflow Is Not An Error

And thus not reported by error detection tools such as undefined behavior sanitizers, unless explicitly enabled with `-fsanitize=unsigned-integer-overflow` but beware that this may trigger on intentional wrapping.

```cpp
std::size_t x = /* Expression. */;
std::size_t y = /* Expression. */;
container[x - y];
```

## Implicit Conversion Of Signed Values Leads To Bugs

[(10)](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/)

For more on this, see _Advantages Of Signed_ > _Less Mixing Of Signed And Unsigned_.

## Backwards Loops Non-Trivial To Write

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


# Disadvantages Of Signed

## Difficult To Interact With The Standard Library

The standard library uses unsigned integer types for many things, in particular `size_t size() const` and `T& operator[](size_t index)`.
This makes working with the standard library with `integer_t` being signed difficult since we are forced to mix signed and unsigned values in expressions and get sign-conversion warnings all over the place if `-Wsign-conversion` is enabled,
or explicitly cast our index variables from signed to unsigned on every container access.
This creates verbose, noisy, and difficult to read code, and we risk incorrect behavior if we ever encounter a `std::size_t`  value larger than `std::numeric_limits<std::ptrdiff_t>::max()` [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc)****.
It is unclear how such a case should be handled.
The same is true for interactions with much of the C standard library, `strlen`, `memcpy`, and such.
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

Since unsigned integers cannot be negative there is no need to test whether a given index is less than zero.
So it is enough to test the index against one end of the range,
since the other end is built into the type.

With a signed type, since it can contain negative values, we must also check the lower end of the range, i.e. 0.

```cpp
// Unsigned index.
if (index >= container.size())
	return false;

// Signed index.
if (index < 0 || index >= container.size())
	return false;
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
Which is true for e.g. `std::vector`.

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


## Small Index Calculation Errors Causes Misbehaving Programs

With unsigned indexing if you accidentally compute a "negative" value and use that to index into a container you will often get a very obvious error since it will try to read very far off into memory.
Likely a crash.
With unsigned indexing a slight miscalculation resulting in a small negative number will cause indexing into a container to access memory  close to the elements and likely valid accessible memory [(21)](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699).
Memory that is used for something else.
This can lead to hard to diagnose errors since the bug manifests itself elsewhere, possibly far from where the actual error, the bad container access, really is.
Debug builds, asserts, and possibly memory sanitizers, can help identify such problems.

## Under- And Overflow Is Undefined Behavior

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

## Must Use A Larger Type To Store Unsigned Input

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

## Safe Arithmetic Takes More Instructions

By "safe" I mean guaranteed to have produced the mathematical result,
the result we would have gotten with infinite precision, otherwise we are give a false return or similar.
It is easier to check for unsigned wrap around than signed overflow.
With signed integers safe addition requires up to three of branches and an extra subtraction.
With unsigned it is a single add instruction and a read from the status flags registry [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2910).
Similar for other operators as well.

## The Modulus Operator Behavior Can Be Unexpected For Negative Numbers

A common way to detect odd numbers is `n % 2 == 1`.
This does not work for negative `n` since the result is `-1` instead of `1`.
Use `n % 2 != 0` instead.

## Shift Operators Behavior Can Be Unexpected For Negative Numbers

TODO
Description and examples here.

## Invites To Doing Inline Error Reporting

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

## More Operators With Problematic Semantics

Given a listing of all operators on signed and unsigned integers a larger fraction of can produce undefined behavior when operating on signed integers compared to the fraction of operators that can produce wrap around when operating on unsigned integers [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2643).
There are more things that can go wrong with them.
This is because when using two's complement representation of signed integers we have one more negative number than we have positive ones.
We can't negate the most negative value because there is no corresponding positive value.
The same is true for dividing by or taking the remainder of that number and -1.

# Recommendations

What can a programmer do today to avoid as many pitfalls as possible?

## Alternatives To Indexing

Don't use explicit indices at all, do something else instead.

- Range based for loops.
	- Less flexible than index-based loops.
- The ranges library.
- Named algorithms.
- Iterators.

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

I'm not sure that the "intersection of advantages" and "union of constraints" are.
I should make a list here.

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
- 25: [_Almost Always Unsiged_ by Dale Weiler @ graphitemaster.github.io 2022](https://graphitemaster.github.io/aau/)
- 26: [_Amost Always Unsigned_ @ reddit.com/cpp 2022](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned/)
- 27: [_Solutions to Integer Overflow_ by regehr @ regehr.org 2016](https://blog.regehr.org/archives/1401)
- 28: [_a praise of size_t and other unsigned types_ by Jens Gustedt @ gustedt.wordpress.com 2013](https://gustedt.wordpress.com/2013/07/15/a-praise-of-size_t-and-other-unsigned-types/)
- 29: [_Why are unsigned integers error prone?_ by Destructor et.al. @ stackoverflow.com 2015](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone)
- 30: [_Signed and Unsigned Types in Interfaces_ by Scott Meyers @ aristeia.com 1995](https://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf)
- 31: [_Danger – unsigned types used here!_ by David Crocker @ critical.eschertech.com 2010](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/)
- 32: [_Should signed or unsigned integers be used for sizes?_ by Martin Ueding et.al. @ stackoverflow.com 2017](https://stackoverflow.com/questions/47283449/should-signed-or-unsigned-integers-be-used-for-sizes)
- 33: [_Signed vs. unsigned integers for lengths/counts_ by user1149224 et.al @ stackoverflow.com 2012](https://stackoverflow.com/questions/10040884/signed-vs-unsigned-integers-for-lengths-counts)
- 34: [_Signed Integers Considered Harmful - Robert Seacord - NDC TechTown 2022_ by Robert C. Seacord @ youtube.com 2022](https://www.youtube.com/watch?v=Fa8qcOd18Hc)


