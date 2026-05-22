#include <algorithm>
#include <vector>

// A sentinel value is used to determine when 'work' should stop.
const int SENTINEL = 0;

void collect_work(std::vector<int>& container)
{
	// Add the next batch of work to the container
	// and sort by priority.
	container.push_back(9);
	//std::sort(container.begin(), container.end());
}

void work(std::vector<int>& container)
{
	// Add a sentinel value so we can know when to stop.
	container.push_back(SENTINEL);
	// Keep an iterator to the sentinel value's initial position
	auto sentinel_it = --container.end();
	collect_work(container);
	// At this point 'sentinel_it' may have become invalidated.
	while (*sentinel_it != SENTINEL)
	{
		// Do work on 'container'.
        container.pop_back();
	}
}


int main()
{
    std::vector<int> container {1, 2, 3};
    container.reserve(10); // This is needed to prevent dangling pointer.
    work(container);
}
