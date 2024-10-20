#include <iostream>

struct AddLog
{
	int value;
};

int add_one(int i)
{
	return i + 1;
}

AddLog add_one(AddLog add_log)
{
	return {add_log.value + 1};
}

std::ostream& operator<<(std::ostream& stream, AddLog const& add_log)
{
	stream << add_log.value;
	return stream;
}

AddLog operator+(AddLog const& lhs, AddLog const& rhs)
{
	std::cout << lhs << " + " << rhs << " = " << (lhs.value + rhs.value) << '\n';
	return {lhs.value + rhs.value};
}

AddLog operator+(AddLog const& lhs, int const rhs)
{
	std::cout << lhs << " + " << rhs << " = " << (lhs.value + rhs) << '\n';
	return {lhs.value + rhs};
}

AddLog operator+(int const lhs, AddLog const& rhs)
{
	std::cout << lhs << " + " << rhs << " = " << (lhs + rhs.value) << '\n';
	return {lhs + rhs.value};
}

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

template <typename... Params>
auto unary_fold_right(Params... params)
{
	return (add_one(params) + ...);
}

template <typename... Params>
auto unary_fold_left(Params... params)
{
	return (... + add_one(params));
}

template <typename... Params>
auto binary_fold_right(Params... params)
{
	return (add_one(params) + ... + 0);
}

template <typename... Params>
auto binary_fold_left(Params... params)
{
	return (0 + ... + add_one(params));
}

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

void print(int i)
{
	std::cout << "int i = " << i << '\n';
}

void name_type(int)
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
	AddLog one(1);
	AddLog five(5);
	std::cout << "\nUnary fold right:\n" << unary_fold_right(one, 2, 3, 4, five) << '\n';
	std::cout << "\nUnary fold left:\n" << unary_fold_left(one, 2, 3, 4, five) << '\n';
	std::cout << "\nBinary fold right:\n" << binary_fold_right(one, 2, 3, 4, five) << '\n';
	std::cout << "\nBinary fold left:\n" << binary_fold_left(one, 2, 3, 4, five) << '\n';

	std::cout << "\nUnary fold right on int:\n" << unary_fold_right(1, 2, 3, 4, 5) << '\n';

	// std::cout << "\nUnary fold with empty parameter list:\n" << unary_fold_right() << '\n';
	std::cout << "\nBinary fold with empty parameter list:\n" << binary_fold_right() << '\n';

	std::cout << "\nLeft fold bools, all true:\n"
			  << all(BoolLog(true, "t1"), BoolLog(true, "t2"), BoolLog(true, "t3")) << '\n';

	std::cout << "\nLeft fold bools, middle false:\n"
			  << all(BoolLog(true, "t1"), BoolLog(false, "f2"), BoolLog(true, "t3")) << '\n';

	std::cout << "\nLeft fold operations:\n"
			  << all(CheckThatReturnsFalse(), OperationThatMayNotBePerformed()) << '\n';

	for_each_arg(print, 1, 2, 3);

	for_each_arg(name_type, 1, 'a', 1.0);

	std::cout << '\n';
}
