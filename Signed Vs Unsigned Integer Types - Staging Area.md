This note is a copy of [[Signed Vs Unsigned Integer Types - Source Notes]].
The intention is to move piece by piece from this note to [[Signed Vs Unsigned Integer Types]] until this note is empty.












# Disadvantages Of Unsigned

## Unsigned Integer Does Not Model Natural Numbers

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

Unsigned integers have surprising conversions to/from signed integers, see _Advantages Of Signed_ > _Less Mixing Of Signed And Unsigned_.


## Easy To Accidentally Write Conditions That Are Always True Or Always False

Such as `unsigned_value >= 0` [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc) or the opposite `unsigned_value < 0` [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).
For example
```cpp
for (std::size_t i = container.size(); i >= 0; --i)
{
	/* Do stuff. */
}
```

This is not illegal per the language, this code should compile without error.
You may get a warning, but that is just the compiler trying to be helpful.

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

See also _Advantages Of Signed_ > _Backwards Loops Easier To Write_.

## The Underflow Edge Is Close To Common Numbers

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

It is impossible to detect earlier arithmetic underflow, other than with heuristics [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), [(62)](https://news.ycombinator.com/item?id=29766658), [(73)](https://news.ycombinator.com/item?id=2364065).
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

## Under- And Overflow Is Not An Error

And thus not reported by error detection tools such as undefined behavior sanitizers, unless explicitly enabled with `-fsanitize=unsigned-integer-overflow` but beware that this may trigger on intentional wrapping [(55)](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).

```cpp
std::size_t x = /* Expression. */;
std::size_t y = /* Expression. */;
container[x - y];
```

## Implicit Conversion Of Signed Values Leads To Bugs

[(10)](https://www.learncpp.com/cpp-tutorial/stdvector-and-the-unsigned-length-and-subscript-problem/)

For more on this, see _Advantages Of Signed_ > _Less Mixing Of Signed And Unsigned_.

## Impossible To Have Negative Intermediate Result

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

## Forced To Mix Signed And Unsigned

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


# Disadvantages Of Signed

## Difficult To Interact With The Standard Library

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


## Require Two Comparisons For Range Checks

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


## Small Index Calculation Errors Causes Misbehaving Programs

With unsigned indexing if you accidentally compute a "negative" value and use that to index into a container you will often get a very obvious error since it will try to read very far off into memory.
Likely a crash.
With unsigned indexing a slight miscalculation resulting in a small negative number will cause indexing into a container to access memory  close to the elements and likely valid accessible memory [(21)](https://internals.rust-lang.org/t/subscripts-and-sizes-should-be-signed/17699).
Memory that is used for something else.
This can lead to hard to diagnose errors since the bug manifests itself elsewhere, possibly far from where the actual error, the bad container access, really is.
Debug builds, asserts, and possibly memory sanitizers, can help identify such problems.


## False Sense Of Safety

The fact that signed integers are well behaved around zero can lead developers to forget about the limited range entirely and not consider the lower and upper bounds that still do exist for signed integers just as they do for unsigned integer [(75)](https://youtu.be/82jVpEmAEV4?t=3739).
May of the problems that exist with unsigned integers exists for signed integers as well, assuming adversarial inputs.
If a programmer cannot reason about unsigned integers not be able to take on negative values, then that programmer also likely won't be able to reason about large negative or positive signed integers.
Using a signed integer type will make bad code produce the correct result a larger number of times, but it doesn't offer complete protection.


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
The presence of possible undefined behavior can cause the compiler to do unexpected transformations to our code [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), [(68)](https://github.com/ericniebler/stl2/issues/182).

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

## Using Signed Return Values Is Misleading

Having a signed return value signals that one should expect negative values to be returned.
In scenarios where only positive values are expected the user may be confused as to during what circumstances the return value could be negative.
Maybe the documentation explains the situation, but maybe it doesn't.

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

Given a listing of all operators on signed and unsigned integers a larger fraction of them can produce undefined behavior when operating on signed integers compared to the fraction of operators that can produce wrap around when operating on unsigned integers [(34)](https://www.youtube.com/watch?v=Fa8qcOd18Hc?t=2643).
There are more things that can go wrong with signed integers.
This is because when using two's complement representation of signed integers we have one more negative number than we have positive ones.
We can't negate the most negative value because there is no corresponding positive value.
Which means that we also can't take its absolute value, i.e. `std::abs(singed)` is not a safe operation.
The same is true for dividing by or taking the remainder of the most negative number and -1.

The following identities does not hold with signed integers [(35)](https://lemire.me/blog/2017/05/29/unsigned-vs-signed-integer-arithmetic):
- `a/b = sign(b) * a / abs(b)`
- `a * b = sign(a) * sign(b) * abs(a) * abs(b)`


# Recommendations

What can a programmer do today to avoid as many pitfalls as possible?

Don't use unsigned for quantities [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

Don't do math on unsigned types [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

Don't mix signed and unsigned types [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

Use unsigned only for:
- bitmasks [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

Don't use unsigned when you need:
- mathematical operations [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).
- compare magnitudes [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU).

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

