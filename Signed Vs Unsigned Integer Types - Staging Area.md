This note is a copy of [[Signed Vs Unsigned Integer Types - Source Notes]].
The intention is to move piece by piece from this note to [[Signed Vs Unsigned Integer Types]] until this note is empty.











# Advantages Of Signed

## Can Detect Unintended Negative Values

When a signed expression unintentionally produces a negative values we can detect that by simply checking if the result is smaller than 0 [(30)](https://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf).
That makes error reporting easier, and thus more prevalent [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned).
With unsigned integers we can't do that since there can't be any negative values [(29)](https://stackoverflow.com/questions/30395205/why-are-unsigned-integers-error-prone), [(62)](https://news.ycombinator.com/item?id=29766658).
That means bad values can more easily pass deeper into our code while doing something wrong [(57)](https://www.reddit.com/r/cpp/comments/rtsife/almost_always_unsigned), possibly leading to security vulnerabilities.
The original bug is the same, but the impact is different.

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
This works with unsigned integers as long as the computation only involves operations that work as intended under modular arithmetic, e.g. additions, subtractions and multiplications [(44)](https://www.nayuki.io/page/unsigned-int-considered-harmful-for-java).
Not division and reminder.
Also, for unsigned it is required that all variables have the same bit-width, since the usual arithmetic conversions don't do sign-extension for unsigned integers [(73)](https://news.ycombinator.com/item?id=2364065).
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

## Less Mixing Of Signed And Unsigned

One source of bugs is when signed and unsigned values are mixed in an expression [(7)](https://google.github.io/styleguide/cppguide.html#Integer_Types), [(12)](https://www.sandordargo.com/blog/2023/10/11/cpp20-intcmp-utilities), [(15)](https://youtu.be/Puio5dly9N8?t=2561), [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU), [(41)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1089r2.pdf). [(45)](https://stackoverflow.com/a/18796546), [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity).
This leads to implicit conversions and results that are difficult to predict for many programmers.

The problem is that C++ doesn't use the values two variables have when an operator is given variables of different types.
Instead integer promotion [(50)](https://eel.is/c++draft/conv.prom) and the usual arithmetic conversions happen [(49)](https://eel.is/c++draft/expr.arith.conv).
An example making the effect obvious [(19)](https://www.youtube.com/watch?v=wvtFGa6XJDU):
```cpp
signed int a {-1};
unsigned int b {1};
if (a < b)
{
	// Not executed.
}
else
{
	// Is executed.
}
```

So why is -1 not less than 1?
What happens, in this case, is that the signed variable, which has a low value, is converted to the type of the unsigned variable [(49)](https://eel.is/c++draft/expr.arith.conv).
See _Dangers_ > _Implicit Conversions_.
Since the signed variable has a value not representable in unsigned type we get "garbage".
(Not actual garbage, the value we get is well defined, but in many cases it is indistinguishable for garbage since we got a completely different value.)
This is surprising, and arguably bad language design.

For example, the following two expressions, that use the same values, produce different results [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity).
```cpp
int base = 12;
unsigned short offset = 25;
int size = 32;
if (base - offset < size)
{
	// This is true since 12 - 25 = -13, which is less than 32.
}
```
The above behaves as one would expect, but change a single type to a wider one and the semantics changes completely, due to the usual arithmetic conversions, [(48)](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions) [(49)](https://eel.is/c++draft/expr.arith.conv):
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
This variant is particularly insidious if the `size` value is a function since in that case ALL values we declare are signed and a signed left hand side is computed for the comparison, but then the value is implicitly converted to unsigned and destroyed in the process.
```cpp
int base = 12;
int offset = 25;
if (base - offset < container.size())
{
	// This is often false since, surprisingly,
	// 12 - 25 = 4294967283
	// which is greater than most container sizes.
}
```
To avoid this problem use `std::ssize`, which returns a singed integer, instead of `Container::size`.

Make one more type change and we get the expected result again:
```cpp
int64_t base = 12;
int offset = 25;
unsigned int size = 32;
if (base - offset < size)
{
	// This is true since 12 - 25 = -13, which is less than 32.
}
```
In this case it is the unsigned right hand side that is converted to the type of the left hand side instead, signed, of vice versa because the left hand side has a larger bit width.

A real-world example may be a line like the following [(56)](https://hamstergene.github.io/posts/2021-10-30-do-not-use-unsigned-for-nonnegativity):
```cpp
if (source_index - findTargetIndex() < getSize())
```
It is not obvious what the types are here, or what implicit conversions will happen.

This type of implicit conversion become even more difficult to keep track of when using typedef'd integer types and the application support multiple  platforms possibly with different integer sizes.

See _Dangers_ > _Implicit Conversions_ > _Usual Arithmetic Conversions_.

Assuming we are required to use signed values for some variables, some data is inherently signed [(13)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf), it follows that we want to also use a singed type for any value that is used together with the inherently signed data.
This process repeats and a front of the signed-ness spreads like a virus across the code base until everything, or at least most things, are signed.
Every time we introduce a variable with an unsigned integer type we risk introduce mixing with the must-be-signed values and thus introduce bugs.

Unfortunately the reverse of this is also true since the standard library uses a lot of unsigned types in its container.
This means that a similar front of unsigned-ness spreads in the opposite direction.
Trouble and headaches happen where these fronts meet.
Unless we chose to use a different set of containers that use a signed integer type instead.

A goal should be to keep this front of mixed signed-ness as small and manageable as possible.

Since we cannot, in many applications, avoid negative, and thus signed, integers it is not unreasonable to confine the unsigned types as much as possible.
As soon as we are given an unsigned value we check that it isn't too large and then cast it to signed.
As soon as we need to supply an unsigned value somewhere we check that the signed value we have isn't negative and then cast it to unsigned [(31)](https://critical.eschertech.com/2010/04/07/danger-unsigned-types-used-here/).

A counter-point is that if a function is so large that it becomes difficult to track which variables are signed and which are unsigned then the function should be split anyways.
Trying to force everything to the same signedness is a false sense of security.

A counter-point to that is that as programmers we absolutely do not want to discard whatever we currently have in our own working memory, for example while debugging, to start analyzing the impacts of possible implicit conversions, sign extensions, and wrapping behavior.
We want code to do what it looks like the code is doing.
We cannot memorize an ever-growing number of tricks and idioms to make loops using unsigned counters work correctly in all edge and corner cases.

One reason for why mixing signed and unsigned integers in C++ is because of unfortunate implicit conversion rules [(46)](https://stackoverflow.com/a/18248537).
Consider
- `unsigned - unsigned` which produces an unsigned result, and
- `signed + unsigned` which also produces an unsigned result (except if the signed integer type has higher range than the unsigned type).

If we take "unsigned" to mean "cannot be negative", i.e. "is positive", we can write
`positive - positive is positive` and `signed + unsigned is unsigned`.
Both of these are logically false.
Subtracting two positive values can produce a negative value,
and adding a positive value to a negative value may result in another negative value.

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

## Mostly Safe Conversions To Unsigned

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

## More Opportunities For Compiler Optimizations

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



### There is an x86-64 Instruction For Converting A Signed Integer To Floating-Point

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

### /

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

