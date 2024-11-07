#include <iostream>
#include <variant>
#include <vector>

volatile int sink_int {0};
volatile double sink_double {0.0};

__attribute((noinline)) void consume(int i)
{
	sink_int = i;
}

__attribute((noinline)) void consume(double i)
{
	sink_double = i;
}

template <int flag>
__attribute((noinline)) void place_flag()
{
	consume(flag);
}

int main()
{
	using IntOrDouble = std::variant<int, double>;
	using IntOrDoubles = std::vector<IntOrDouble>;

	std::cout << "sizeof(IntOrDouble) = " << sizeof(IntOrDouble) << '\n';

	IntOrDoubles int_or_doubles {1, 2.0, 3, 4.0, 5, 6, 7, 8.0};

	struct Process
	{
		void operator()(int i)
		{
			consume(i);
		}

		void operator()(double d)
		{
			consume(d);
		}
	};

	place_flag<1>();
	for (IntOrDouble& int_or_double : int_or_doubles)
	{
		std::visit(Process(), int_or_double);

		// std::visit([](auto& value) { Process()(value); }, int_or_double);
	}
	place_flag<2>();

	return 0;
}
