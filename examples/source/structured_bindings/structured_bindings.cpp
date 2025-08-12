#include <cstdint>
#include <iostream>
#include <map>

template <typename Key, typename Value, typename Function>
void update(std::map<Key, Value>& table, Function getNewValueForKey)
{
	for (auto& [key, value] : table)
	{
		value = getNewValueForKey(key);
	}
}

template <typename Map>
void print(Map const& table)
{
	std::cout << "Table: \n";
	for (auto [key, value] : table)
	{
		std::cout << " " << key << " -> " << value << '\n';
	}
}

void testUpdate()
{
	std::cout << "\n# " << __FUNCTION__ << '\n';

	// clang-format off
	std::map<int, char> table {
        {1, 'a'},
        {2, 'b'},
        {3, 'c'}
    };
	// clang-format on

	print(table);
	std::cout << "Make upper-case.\n";
	update(table, [](int key) { return 'A' + key - 1; });
	print(table);
}

class Person
{
public:
	std::uint64_t& getId()
	{
		return m_id;
	};
	std::string& getName()
	{
		return m_name;
	};
	std::uint16_t& getAge()
	{
		return m_age;
	};

private:
	std::uint64_t m_id;
	std::string m_name;
	std::uint16_t m_age;
};

template <std::size_t I>
auto& get(Person& person)
{
	// This line is optional.
	// See block comment 'testPerson' below.
	static_assert(I <= 2);

	if constexpr (I == 0)
	{
		return person.getId();
	}
	else if constexpr (I == 1)
	{
		return person.getName();
	}
	else if constexpr (I == 2)
	{
		return person.getAge();
	}

	// No if-less else or final return because written
	// like this the return type would be deduced as
	// void&, which is illegal and won't compile.
}

void testPerson()
{
	std::cout << "\n# " << __FUNCTION__ << '\n';

	Person person;
	person.getId() = 0;
	person.getName() = "Alice";
	person.getAge() = 18;

	std::cout << get<0>(person) << ", " << get<1>(person) << ", " << get<2>(person) << '\n';

	// The following does not compile.
	/*
	Without 'static_assert(I <= 2)' in get(Person&):
	  In instantiation of ‘auto& get(Person&) [with long unsigned int I = 3]’:
	  error: no return statements in function returning ‘auto&’
	  note: only plain ‘auto’ return type can be deduced to ‘void’
	  warning: no return statement in function returning non-void [-Wreturn-type]

	With 'static_assert(I <= 2)' in get(Person&):
	  In instantiation of ‘auto& get(Person&) [with long unsigned int I = 3]’:
	  error: static assertion failed
	  static_assert(I <= 2);
	*/
#if 0
	std::cout << get<3>(person) << '\n';
#endif
}

int main()
{
	testUpdate();
	testPerson();
}
