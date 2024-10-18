A switch-init expression is part of a switch statement.
It allows for the declaration and initialization of one or more variables within a switch statement [(2)](https://youtu.be/fI2xiUqqH3Q?t=629).

Syntax:
```cpp
switch (init; cond)
{
	case a: statement1;
	case b: statement2;
	// ...
}
```

Example:
```cpp
class Context
{
	void setResult(int);
};

enum class Operator { PLUS, MINUS };

struct Node {
	Context getContext();
	Operator getOperator() const;
};

void evaluate(int lhs, int rhs, Node node)
{
	switch (Context context = node.getContext(); node.getOperator())
	{
		case Operator::PLUS:
			context.setResult(lhs + rhs);
			break;
		case Operator::MINUS:
			context.setResult(lhs - rhs);
			break;
	}
}
```
# References

- 1: [_CppCon 2017: Bryce Adelstein Lelbach “C++17 Features (part 1 of 2)”_ by Bryce Adelstein Lelbash, CppCon @ youtube.com 2017](https://youtu.be/fI2xiUqqH3Q)
