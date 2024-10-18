An if-init expression is an if-statement that has two statements in the if-header.
The first statement, called the if-init expression, declares and initializes one or more variables.
The second statement is evaluated as a Boolean and is used to determine if the if-block should be executed or not.
Variables declared in the if-init expression have their lifetime tied to the if-block, i.e. they are not accessible before or after the if-block.

```cpp

void work(Widget widget)
{
	if (Widget parent = widget.getParent(); parent.isVaid())
	{
		// Work with 'parent'.
	}
}
```


# References

- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)

