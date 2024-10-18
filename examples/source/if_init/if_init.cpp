#include <iostream>
#include <mutex>
#include <numeric>
#include <vector>

int sum(std::vector<int>& container, std::mutex& mutex)
{
	int sum = 0;

	if (std::lock_guard lock(mutex); !container.empty())
	{
		sum = std::accumulate(container.begin(), container.end(), 0, std::plus());
	}

	return sum;
}

int main()
{
	std::vector<int> data {1, 2, 3, 4};
	std::mutex mutex;
	std::cout << sum(data, mutex) << '\n';
}
