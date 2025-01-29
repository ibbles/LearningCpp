Also called an [[Anonymous Function]].

They are created with the `[](){}` syntax.
- The `[]` is the capture list. It contains the variables that the function should have access to.
- The `()` is the parameter list. It contains variables that the caller of the function provides at each call site.
- The `{}` is the statement list. It contains the statements that should be executed when the function is called.

Anonymous functions are often used as configuration points for library functions.
For example to provide a comparison function to a sort function.
```cpp
#include "Data.h"

void work(
	Container<Data>& container,
	double small_scale, double large_scale,
	double scale_threshold)
{
	std::sort(
		container.begin(), container.end(),
		[smallScale, largeScale, scaleThreshold]
		(Data const& lhs, Data const& rhs)
		{
			double const lhs_scale =
				lhs.value < scale_threshold ? small_scale : large_scale;
			double const scaled_lhs = lhs_scale * lhs.value;
			
			double const rhs_scale =
				rhs.value < scale_threshol ? small_scale : large_scale;
			double const scaled = rhs_scale * rhs.value;
			
			return scaled_lhs < scale_rhs;
		});
}
```

Anonymous functions can be stored in a variable.
That makes it possible to reference the function using the variable name.
The function is still nameless though, it is the variable that has a name.
We cannot know the name of the type of the anonymous function, only the compiler does, so we have to use `auto`.

```cpp
auto function = [capture_variable](int parameter) { body(); };
```

Instead of explicitly listing variables to capture we can put an `=` to capture all variables.
We can also put an `&` instead of `=` capture by reference instead of value.

```cpp
void work(
	Container<Data>& container,
	double small_scale, double large_scale,
	double scale_threshol)
{
	auto scale = [=](double value) {
		double const scale = // TODO Find name not shadowing outer 'scale'.
			value < scale_threshold ? small_scale : large_scale;
		double const scaled = scale * value;
		return scaled;
	];

	std::sort(
		container.begin(), container.end(),
		[scale] (Data const& lhs, Data const& rhs)
		{
			double const scaled_lhs = scale(lhs.value);
			double const scaled_rhs = scale(rhs.value);
			return scaled_lhs < scaled_rhs;
		});
}
```


# `mutable` Lambda Expression

# `constexpr` Lambda Expression

A lambda expression can be [[constexpr]] in two ways.
The first is to make the call operator [[constexpr]], which makes it possible to call in a [[constexpr]] context.
The second is to make the lambda object itself [[constexpr]], including constructor and copy constructor

## `constexpr` Call Operator

To make a lambda that can be used  in a [[constexpr]] context add `constexpr` between the parameter list the the statement list [(1)](https://youtu.be/fI2xiUqqH3Q?t=2236).
This is implicit, i.e. not required, if the compiler can see both the `constexpr` context where the lambda is called and the lambda expression at the same time.
By adding `constexpr` you are guaranteed by the compiler that the lambda is [[constexpr]] valid even if it isn't used in a `constexpr` context yet.
[[constexpr]] lambdas was not possible before C++17.

```cpp
auto add = [](int lhs, int rhs) constexpr { return lhs + rhs; };
```

## `constexpr` Constructors

A lambda with [[constexpr]] constructors can be created by marking the variable holding the lambda with `constexpr` [(1)](https://youtu.be/fI2xiUqqH3Q?t=2390).
```cpp
constexpr auto add = [](int lhs, int rhs) { return lhs + rhs; };
```

This is relevant if you have captures and your constructors are not trivial.


## Example

```cpp
// A function template with a value template parameter must
// have the value being known at compile time.
template <int N>
int value()
{
    return N;
}

int works()
{
	// A lambda marked constexpr can be used to select a
	// function template instantiation.
    constexpr auto add = [](int lhs, int rhs) constexpr { return lhs + rhs; };
    return value<add(5, 5)>();
}

int does_not_work(int num)
{
	// We cannot use a non-constexpr value in a constexpr lambda.
	// error: 'num' is not a constant expression
    constexpr auto add = [num](int lhs, int rhs) constexpr { return lhs + rhs + num; };
    return value<add(5, 5)>();
}
```


# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
