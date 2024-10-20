Used with [[Variadic Templates]] and [[Template Parameter Pack]]s.
Provides a way to write expression that applies a binary operator on a variable number of function parameters without having to write explicit base cases for zero and one parameter [(2)](https://youtu.be/fI2xiUqqH3Q?t=1130).
A fold expression is characterized by the `...` token and is always wrapped in parenthesis.

A fold expression can be either unary or binary [(2)](https://youtu.be/fI2xiUqqH3Q?t=1172).
A unary fold expression does not mean that it uses a unary operator.
An unary fold does not take an initial value, the first or last element of the [[Template Parameter Pack]] becomes the first operand.
A binary fold expression does take an initial value that is separate from the [[Template Parameter Pack]].
A binary fold expression given an empty [[Template Parameter Pack]] will return the initial value.
A unary fold expression given an empty [[Template Parameter Pack]] will result in a compiler error except when the operator is 
- `&&`, for which the return value is true.
- `||`, for which the return vale is false.
- `,`, for which the return value is `void()`.


A fold expression can be either left or right[(2)](https://youtu.be/fI2xiUqqH3Q?t=1172).
A left fold expression evaluates the left-most operation first.
A right fold expression evaluates right-most operation first.
We differentiate between left and right fold expressions by where the `...` is placed relative to the [[Template Parameter Pack]].
When the `...` is placed to the right of the [[Template Parameter Pack]] we have a right fold.
When the `...` is placed to the left of the [[Template Parameter Pack]] we have a left fold.

With `E` being a [[Template Parameter Pack]] with elements \[0, N-1\], `op` being a binary operator, and `I` being the initial value, either a variable or a literal:
- Unary right fold.
	- `(E op ...)`
	- `E[0] op (... op (E[N-2] op E[N-1]))`
	- The operator is first applied to the second-to-last and last elements of the [[Template Parameter Pack]]. The to the third-to-last element and the result so far. Then the operator is applied to the fourth-to-last element and the result so far. And so on.
- Unary left fold.
	- `(... op E)`
	- `((E[0] op E[1]) op ...) op E[N-1]`
	- The operator is first applied to the first two elements of the [[Template Parameter Pack]]. The to the result so far and the third element. Then the result so far and the fourth element. And so son.
- Binary right fold.
	- `(E op ... op I)`
	- `E[0] op (... op (E[N-2] op (E[N-1] op I)))`
	- Binary fold right works just like unary fold right except that the right-most element isn't from the [[Template Parameter Pack]] but instead provided separately.
- Binary left fold.
	- `(I op ... op E)`
	- `(((I op E[0]) op E[1]) op ...) op E[N-1]`
	- Binary left fold works just like unary fold left except that the left-most element isn't from the [[Template Parameter Pack]] but instead provided separately.


Example to add numbers [(1)](https://www.youtube.com/watch?v=QpFjOlzg1r4) that uses a a unary right fold where `E` is `params` and `op` is `+`:
```cpp
template <typename ... T>
auto add(T const& ... params)
{
	return (params + ...);
}

int work()
{
	return add(1, 2, 3, 4, 5);
}
```
The above will evaluate `(1 + (2 + (3 + (4 + 5))))`.

A variant that uses a left fold instead:
```cpp
template <typename ... T>
auto add(T const& ... params)
{
	return (... + params);
}

int work()
{
	return add(1, 2, 3, 4, 5);
}
```
The above will evaluate `(((((1 + 2) + 3) + 4) + 5)`.

To help remember the evaluation order of unary folds think of the `...` as a recursive function call that must be evaluated before the final operator application can take place.

A variant that uses a binary right fold [(2)](https://youtu.be/fI2xiUqqH3Q?t=1130):
```cpp
template <typename... Ns>
auto sum(Ns... ns)
{
	return (ns + ... + 0);
}

int work()
{
	return sum(1, 2, 3, 4, 5);
}
```
The above will evaluate `(1 + (2 + (3 + (4 + (5 + 0)))))`.

To help remember the evaluation order of binary folds think of the extra operand `I` as the starting value, the one that will be used by the operator application that is most deeply nested, i.e. the one that is applied first.

# Boolean Operator

A fold expression can use Boolean values and operators [(2)](https://youtu.be/fI2xiUqqH3Q?t=1254).
```cpp
template <typename... Bools>
bool all(Bools... bools)
{
	return (... && bools);
}
```

This is a left fold, i.e. it will evaluate from left to right.
This is appropriate for the and-operator since it uses left to right evaluation and short-circuting.
Consider the following common idiom:
```cpp
void work(Widget* widget)
{
	if (widget != nullptr && widget->is_valid())
	{
		// Work with 'widget'.
	}
}
```

It is important that `widget != nullptr` is evaluated before `widget->is_valid()`, and that the second sub-expression is not evaluated if the first is false.
A left-fold on a set of Boolean values has this effect since it evaluates
```
((E[1] op E[2]) op ...) op E[N]
```
and the language rules will ensure that any `(E[k-1] op E[k])` sub-expression will not evaluate `E[k]` if `E[k-1]` is false.

(
NOTE: I don't get short-circuting to work.
```cpp
#include <iostream>

struct BoolLog
{
	bool value;
	std::string name {""};
};

std::ostream& operator<<(std::ostream& stream, BoolLog const& rhs)
{
	stream << rhs.name << "=" << std::boolalpha << rhs.value;
	return stream;
}

BoolLog operator&&(BoolLog const& lhs, BoolLog const& rhs)
{
	std::cout << lhs << " && " << rhs << " = " << (lhs.value && rhs.value) << '\n';
	return {lhs.value && rhs.value, lhs.name + rhs.name};
}

BoolLog operator&&(BoolLog const lhs, bool rhs)
{
	std::cout << lhs << " && " << rhs << " = " << (lhs.value && rhs) << '\n';
	return {lhs.value && rhs, lhs.name + (rhs ? "t" : "f")};
}

BoolLog operator&&(bool lhs, BoolLog const rhs)
{
	std::cout << lhs << " && " << rhs << " = " << (lhs && rhs.value) << '\n';
	return {lhs && rhs.value, (lhs ? "t" : "f" + rhs.name)};
}

template <typename... Bools>
auto all(Bools... bools)
{
	return (... && bools);
}

int main()
{
	std::cout << "\nLeft fold bools, middle false:\n"
			  << all(BoolLog(true, "t1"), BoolLog(false, "f2"), BoolLog(true, "t3")) << '\n';
	return 0;
}
```

The above prints
```shell
Left fold bools, middle false:
t1=true && f2=false = false
t1f2=false && t3=true = false
t1f2t3=false
```

The programs represents `true && false && true`.
This should short-circut after the first operation, `true && false`, since it is false.
The second operation, `false && true`, shouldn't need to be evaluated.

The first print, `t1=true && f2=false = false`, is for the first evaluation.
The second print, `t1f2=false && t3=true = false`, is for the second evaluation.
Why is the second print happening?

But in the following setup short-circuting does happen.
```cpp
#include <iostream>

template <typename... Bools>
auto all(Bools... bools)
{
	return (... && bools);
}

struct CheckThatReturnsFalse
{
	operator bool() const
	{
		return false;
	}
};

struct OperationThatMayNotBePerformed
{
	operator bool() const
	{
		std::cout << "FATAL ERROR\n";
		return false;
	}
};

int main()
{
	std::cout << "\nLeft fold operations:\n"
			  << all(CheckThatReturnsFalse(), OperationThatMayNotBePerformed()) << '\n';
	return 0;
}
```

`FATAL ERROR` is not printed.

What's the difference?
Is short-circuting only for `bool`, and not for `operator&&` with user-defined types?
Yes, that seems to be the case.
From cppreference.com [(3)](https://en.cppreference.com/w/cpp/language/operator_logical)

> Builtin operators `**&&**` and `**||**` perform short-circuit evaluation (do not evaluate the second operand if the result is known after evaluating the first), but overloaded operators behave like regular function calls and always evaluate both operands.

and [(4)](https://en.cppreference.com/w/cpp/language/operators)

> The overloads of operators `**&&**` and `**||**` lose short-circuit evaluation.

)

# Per Element Expression

We need not use the [[Template Parameter Pack]] as-is, we can perform computations on each element before it is passed to the binary operator.

```cpp
template <typename T>
T add_one(T const& t)
{
	return t + T(1);
}

template <typename... Params>
auto sum(Params const&... params)
{
	return (add_one(params) + ...);
}
```

# Print

Since most overloads of `operator<<` with the left hand side being an output stream return that stream we can chain multiple prints [(2)](https://youtu.be/fI2xiUqqH3Q?t=1317).

```cpp
template <typename... Ts>
void print(Ts... ts)
{
	(std::cout << ... << std::forward<Ts>(ts)) << '\n';
}
```

This is a binary fold since we provide an initial value.
In this case the initial value is `std::cout`.
It is a left fold since `...` is to the left of the [[Template Parameter Pack]] name `ts`.
The operator is `<<`.
The per element expression `std::forward<Ts>(ts)` is used to avoid unnecessary copying when calling `operator<<(std::ostream&, Ts[k])`.

Since this is a left fold the first step is `std::cout << ts[0]`.
The return value of this expression is `std::cout`.
The second step takes the return value of the first step, `std::cout`, and the next element in the [[Template Parameter Pack]] and thus evaluates `std::cout << ts[1]`.
The pattern continues over all elements of `ts`.

So the full expansion for a parameter list with tree elements is
```cpp
template <typename T0, typename T1, typename T2>
void print(T0 t0, T1 t1, T2 t2)
{
	std::cout
		<< std::forward<T0>(t0)
		<< std::forward<T1>(t1)
		<< std::forward<T3>(t3)
		<< '\n';
}
```


# Call A Function Multiple Times

We can write a function that takes a function that can be called with a single parameter and a list of parameters to call that function with [(2)](https://youtu.be/fI2xiUqqH3Q?t=1347).
```cpp
template <typename Function, typename... Args>
void for_each_arg(Function function, Args&&... args)
{
	(function(std::forward<Args>(args)), ...);
}
```

When called as
```cpp
for_each_arg(log, 1, 'a', 1.0);
```
the function template (almost) expands to
```cpp
template <typename Function>
void for_each_arg(Function function, int&& args0, char&& args1, double&& args2)
{
	(
		function(std::forward<int&&>(args0)),
		function(std::forward<char&&>(args1)),
		function(std::forward<double&&>(args2))
	);
}
```

I write "almost" because it is actually
```cpp
template <typename Function>
void for_each_arg(Function function, int&& args0, char&& args1, double&& args2)
{
	(
		(function(std::forward<int&&>(args0)),
		(function(std::forward<char&&>(args1)),
		function(std::forward<double&&>(args2))))
	);
}
```

(
TODO: I'm not sure about the following. Is any of this guaranteed?

There is an important difference between `operator+` and `operator,` in how parenthesized operands are evaluated.
In `a + (b + c)` the `b + c` part must be evaluated before the add with `a`.
This means that `b` and `c` is evaluated before `a`. (Does it really?).
In `a, (b, c)` there is no dependency between the two `,`s.
Instead they operands are evaluated from left to right.
So first `a` and then `(b, c)`.
When `(b, c)` is evaluated we do the same, evaluate from left to right.
For first `b` and then `c`.
So even though `(function(args), ...)` is a right fold, the right-most elements of `args` is most deeply nested, `function` is still called with `args[0]` first and `args[N-1]` last.
)


I never got `for_each_arg` to work with function overloading.
The following gives a compiler error.
```cpp
id name_type(int)
{
	std::cout << "int\n";
}

void name_type(char)
{
	std::cout << "char\n";
}

void name_type(double)
{
	std::cout << "double\n";
}

template <typename T>
void name_type_dispatch(T t)
{
	name_type(t);
}

template <typename Function, typename... Ts>
void for_each_arg(Function function, Ts... args)
{
	(function(args), ...);
}

int main()
{
	// Doesn't matter if I use 'name_type' or 'name_type_dispatch'.
	for_each_arg(name_type, 1, 'a', 1.0);
	for_each_arg(name_type_dispatch, 1, 'a', 1.0);
	return 0;
}
```

The error is `couldn’t deduce template parameter ‘Function’`.
This is because `function` in `for_each_arg` is a specific function, not a function overload set.
Making a function template, `name_type_dispatch` in this example, doesn't help because it isn't the function template that is passed to `for_each_arg` but a specific function template instantiation.
And there is no function template instantiation, i.e. type to assign to `T` in `name_type_dispatch<T>(T t)` that will match all types in `Ts`.
I don't know what to do about this.

# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
- 2: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
- 3: [_Logical operators_ @ cppreference.com 2024](https://en.cppreference.com/w/cpp/language/operator_logical)
- 4: [_operator overloading_ @ cppreference.com 2024](https://en.cppreference.com/w/cpp/language/operators)

