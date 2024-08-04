#include <iostream>
#include <limits>
#include <vector>

void report_error(const char* message)
{
	std::cout << "  Error: " << message << '\n';
}


template <typename Container>
bool isValidIndex(const Container& container, std::size_t index)
{
	return index < container.size();
}

template <typename Container>
bool isValidIndex(const Container& container, std::ptrdiff_t index)
{
	// Negative indices are never valid.
	if (index < 0)
	{
		return false;
	}

	// Sanity check of the container.
	// Can the below error report ever trigger?
	const std::size_t max_possible_index =
		static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());
	if (container.size() > max_possible_index)
	{
		report_error("Container too large for signed indexing");
	}

	return index < std::ssize(container);
}


namespace mock
{
	class UnsignedVector
	{
	public:
		explicit UnsignedVector(std::size_t size)
			: m_size(size)
			, m_capacity(size)
		{
		}

		std::size_t size() const
		{
			return m_size;
		}

		std::size_t capacity() const
		{
			return m_capacity;
		}

		int operator[](std::size_t index)
		{
			if (!isValidIndex(*this, index))
			{
				report_error("Invalid index passed to mock::UnsignedVector::operator[].");
				return -1;
			}

			return index;
		}

	private:
		std::size_t m_size;
		std::size_t m_capacity;
	};
}


namespace is_valid_index
{
	void too_large_container_for_signed()
	{
		const std::size_t max_signed_index =
			static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max());
		const std::size_t too_large {max_signed_index + 1};
		bool is_valid {isValidIndex(mock::UnsignedVector(too_large), 5l)};
		std::cout << "  Too large for signed: " << too_large << ": " << is_valid << '\n';
	}

	void negative_offset_unsigned(std::size_t base, std::ptrdiff_t offset, std::size_t stride)
	{
		const std::size_t index = base + offset * stride;
		std::cout << "  strided offset = " << offset << " * " << stride << " = " << offset * stride << '\n';
		std::cout << "  index = " << base << " + " << offset << " * " << stride << " = " << index << '\n';
	}

	void run()
	{
		std::cout << "isValidIndex:\n";
		too_large_container_for_signed();
		negative_offset_unsigned(100, -10, 5);
	}
}


namespace signed_to_unsigned_conversion
{
	void simple_arithmetic(signed int lhs, unsigned int rhs)
	{
		std::cout << "  signed * unsigned multiply: " << lhs << " * " << rhs << " = " << (lhs * rhs) << '\n';
	}

	template <typename C>
	auto access(C& container, signed int index) -> typename C::value_type&
	{
		const auto i = static_cast<typename C::size_type>(index);
		return container[i];
	}

	void use_std_vector(signed int index)
	{
		std::vector<int> data {0, 1, 2, 4, 5, 6, 7, 8, 9};

		// Since index is signed and the parameter of operator[] is unsigned
		// we get an implicit conversion and a warning if -Wsign-conversion
		// is enabled.
		std::cout << "  Element at index " << index << ": " << data[index] << '\n';

		// To suppress the warning we can be explicit about the conversion.
		// Many considers this to be distractingly verbose. I have seen the word
		// "insane" being used.
		std::cout << "  Element at index " << index << ": " << data[static_cast<size_t>(index)] << '\n';

		// Another option is to work with iterators instead of indices since
		// random access iterators can be moved both forwards and backwards they
		// must support signed arithmetic.
		//
		// I find neither option to be as readable as data[index].
		std::cout << "  Element at index " << index << ": " << *(data.begin() + index) << '\n';
		std::cout << "  Element at index " << index << ": " << *std::next(data.begin(), index) << '\n';

		// Another option is to provide wrapper function for every function
		// we want to use with signed parameters.
		std::cout << "  Element at index " << index << ": " << access(data, index) << '\n';

		// Another option is to not use the standard library container at all
		// and instead opt for a library that uses signed indices. The only one
		// I know of are the ones built into Unreal Engine.
	}

	void run()
	{
		// We expect 2 * -3 to be -6. But unfortunately the -3 is passed to an
		// unsigned type and becomes a very large number: 4294967293. The very
		// large number is multiplied by 2 and we wrap around by almost the
		// entire value range of unsigned int and end up at 4294967290.
		// We get a warning both on the function call and the multiply inside
		// the function.
		simple_arithmetic(2, -3);

		use_std_vector(3);
	}

}

int main()
{
	is_valid_index::run();
	signed_to_unsigned_conversion::run();
	return 0;
}
