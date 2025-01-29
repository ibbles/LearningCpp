A single-element container that might hold an element, or it might not.
Often used for return values from functions that may fail or for optional parameters.

```cpp
enum class Item
{
	CARROT,
	TOMATO
};

enum class Shelf
{
	TOP,
	BOTTOM
};

std::unordered_map<Item, Shelf> inventory;

std::optional<Shelf> get_shelf(Item item, std::optional<Item> fallback_item)
{
	if (auto it = inventory.find(item); it != inventory.end())
	{
		return it->second;
	}

	if (fallback_item)
	{
		if (auto it = inventory.find(*fallback_item); it != inventory.end())
		{
			return it->second;
		}
	}

	return {};
}

void add_ingredient(Item item, Item fallback_item)
{
	std::optional<Shelf> shelf_maybe = get_shelf(item, fallback_item);
	if (shelf_maybe)
	{
		std::cout << "The ingredient is on shelf " << *shelf_maybe << '\n';
	}

	// If we didn't have the wanted ingredient, use whatever is on the top shelf.
	Shelf shelf = shelf_maybe.value_or(Shelf::TOP);
	std::cout << "Using ingredient on shelf" << shelf << '\n';
}
```

# References

- 1: [_std::optional_ @ cppreference.com](https://en.cppreference.com/w/cpp/utility/optional)
