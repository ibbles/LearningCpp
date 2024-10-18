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

void test_update()
{
	// clang-format off
	std::map<int, char> table {
        {1, 'a'},
        {2, 'b'},
        {3, 'c'}
    };
	// clang-format on

	print(table);
	update(table, [](int key) { return 'A' + key; });
	print(table);
}

int main()
{
	test_update();
}
