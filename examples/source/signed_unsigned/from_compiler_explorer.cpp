#include <cstddef>
#include <cstdint>

#include <array>
#include <iostream>
#include <limits>
#include <vector>

std::ostream &operator<<(std::ostream &o, const __int128 &x)
{
    if (x == std::numeric_limits<__int128>::min())
        return o << "-170141183460469231731687303715884105728";
    if (x < 0)
        return o << "-" << -x;
    if (x < 10)
        return o << (char)(x + '0');
    return o << x / 10 << (char)(x % 10 + '0');
}

template <typename T>
volatile T dont_optimize{};

template <typename T>
__attribute((noinline))
T
consume(T value)
{
    dont_optimize<T> = value;
    std::cout << value << '\n';
    return value;
}

bool add(int lhs, int rhs)
{
    consume(lhs);
    consume(rhs);
    consume(lhs + rhs);
    return lhs + rhs < 0;
}

bool add(std::int64_t lhs, std::int64_t rhs)
{
    consume(lhs);
    consume(rhs);
    consume(lhs + rhs);
    return lhs + rhs < 0;
}

bool add(__int128 lhs, __int128 rhs)
{
    consume(lhs);
    consume(rhs);
    consume(lhs + rhs);
    return lhs + rhs < 0;
}

bool divide(std::ptrdiff_t lhs, std::ptrdiff_t rhs)
{
    consume(lhs);
    consume(rhs);
    consume(lhs / rhs);
    return lhs / rhs > 10;
}

bool divide(std::size_t lhs, std::size_t rhs)
{
    consume(lhs);
    consume(rhs);
    consume(lhs / rhs);
    return lhs / rhs > 10;
}

void implicit_unsigned(
    std::vector<int> &container,
    std::ptrdiff_t a,
    std::ptrdiff_t b)
{
    auto index = container.size() - (a + b);
    consume(index);
}

template <typename T>
__attribute((noinline)) float to_float(T v)
{
    return static_cast<float>(v);
}

template <typename T>
__attribute((noinline)) double to_double(T v)
{
    return static_cast<double>(v);
}

double average(int64_t *data, int64_t num)
{
    int64_t sum{0};
    for (int64_t i = 0; i < num; ++i)
    {
        sum += data[i];
    }
    return static_cast<double>(sum) / static_cast<double>(num);
}

double average(uint64_t *data, uint64_t num)
{
    uint64_t sum{0};
    for (uint64_t i = 0; i < num; ++i)
    {
        sum += data[i];
    }
    return static_cast<double>(sum) / static_cast<double>(num);
}

double average_small(uint64_t *data, uint64_t num)
{
    uint64_t sum{0};
    for (uint64_t i = 0; i < num; ++i)
    {
        sum += data[i];
    }
    return static_cast<double>((int64_t)sum) / static_cast<double>((int64_t)num);
}

void to_float_and_double()
{
    consume(to_float(int32_t(1)));
    consume(to_float(uint32_t(-1)));
    consume(to_float(int64_t(1)));
    consume(to_float(uint64_t(-1)));

    consume(to_double(int32_t(1)));
    consume(to_double(uint32_t(-1)));
    consume(to_double(int64_t(1)));
    consume(to_double(uint64_t(-1)));
}

struct Pixel
{
    float red;
    float green;
    float blue;
};

struct Image
{
    std::size_t width{10};
    std::size_t height{10};
    Pixel pixels[65535];
};

void work(
    const Pixel *const pixels,
    std::size_t const num_rows,
    std::size_t const num_columns,
    std::int32_t const stride)
{
    for (unsigned int i = 0; i < num_rows; ++i)
    {
        // std::cout << "  i * stride = " << i * stride << '\n';
        const Pixel *row = pixels + i * stride;
        consume(row);
        // Work with row[0] to row[num_columns - 1].
        // std::cout
        // << "Working with row starting at index "
        // << (row - pixels)
        // << ".\n";
    }
}

void work_forwards(const Image &image)
{
    const Pixel *first_row = image.pixels;
    work(first_row, image.height, image.width, image.width);
}

void work_backwards(const Image &image)
{
    const Pixel *last_row = image.pixels + (image.height - 1) * image.width;
    work(last_row, image.height, image.width, -image.width);
}

void multiply(
    unsigned int base,
    unsigned int block_size,
    signed int stride)
{
    consume(base);
    consume(block_size);
    consume(stride);
    consume(block_size / stride);
    consume(base + block_size / stride);
}

void multiply_test()
{
    multiply(1000, 100, -2);
}

template <typename integer_t>
void byte_offset(
    const std::vector<double> &data,
    integer_t base_index,
    const std::vector<std::ptrdiff_t> &byte_offsets,
    integer_t element_size)
{
    for (integer_t byte_offset : byte_offsets)
    {
        integer_t index = base_index + (byte_offset / element_size);
        consume(index);
    }
}

void byte_offset_test_signed()
{
    std::cout << "byte_offset_test_signed\n";
    std::vector<double> data{
        0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    std::ptrdiff_t base_index{3};
    std::vector<std::ptrdiff_t> byte_offsets{
        0, 8, -8, 16, -16};
    std::ptrdiff_t element_size{8};
    byte_offset(data, base_index, byte_offsets, element_size);
}

void byte_offset_test_unsigned()
{
    std::cout << "byte_offset_test_unsigned\n";
    std::vector<double> data{
        0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    std::size_t base_index{3};
    std::vector<std::ptrdiff_t> byte_offsets{
        0, 8, -8, 16, -16};
    std::size_t element_size{8};
    byte_offset(data, base_index, byte_offsets, element_size);
}

void byte_offset_test()
{
    byte_offset_test_signed();
    byte_offset_test_unsigned();
}

bool safe_add(std::size_t lhs, std::size_t rhs, std::size_t *result)
{
    *result = lhs + rhs;
    return *result < lhs;
}

__attribute((noinline))
int32_t
wrap_optimization_test(uint32_t b)
{
    int32_t a = -2;
    int32_t c = 2;
    int32_t d = (a - b) / c + 10;
    return d;
}

__attribute((noinline))
int32_t
wrap_optimization_test(int32_t b)
{
    int32_t a = -2;
    int32_t c = 2;
    int32_t d = (a - b) / c + 10;
    return d;
}

/*
The sum functions test if if the compiler can generate
better code with a signed loop counter compared to an
unsigned one.
*/

// 32-bit unsigned both for the size and the counter.
// I see nothing preventing the compiler from producing
// very good code for this.
__attribute((noinline)) double sum(double *data, unsigned int size)
{
    double sum{0.0};
    for (unsigned int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

// 32-bit signed both for the size and the counter.
// I see nothing preventing the compiler from producing
// very good code for this.
__attribute((noinline)) double sum(double *data, int size)
{
    double sum{0.0};
    for (int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

// 64-bit signed for the size, 32-bit signed for the counter.
// The compiler can assume that size isn't very large since
// if it were then ++index would overflow. It can chose to
// use either 64-bit or 32-bit instructions.
__attribute((noinline)) double sum(double *data, std::ptrdiff_t size)
{
    double sum{0.0};
    for (int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

// 64-bit unsigned for the size, 32-bit unsigned for  the counter.
// The compiler must ensure that ++index wraps properly.
__attribute((noinline)) double sum(double *data, std::size_t size)
{
    double sum{0.0};
    for (unsigned int index = 0; index < size; ++index)
    {
        sum += data[index];
    }
    return sum;
}

class Unsigned
{
public:
    Unsigned() : m_value(0) {}
    Unsigned(std::uint8_t value) : m_value(value) {}
    Unsigned(std::uint16_t value) : m_value(value) {}
    Unsigned(std::uint32_t value) : m_value(value) {}
    Unsigned(std::uint64_t value) : m_value(value) {}

    Unsigned &operator=(Unsigned other) volatile { m_value = other.m_value; }
    operator std::size_t() const { return m_value; }

    Unsigned(std::int8_t) = delete;
    Unsigned(std::int16_t) = delete;
    Unsigned(std::int32_t) = delete;
    Unsigned(std::int64_t) = delete;

private:
    std::size_t m_value;
};

void unsigned_work(Unsigned value)
{
    consume(value);
}

void unsigned_test()
{
    // These do not compile.
#if 0
    std::int8_t s8;
    unsigned_work(s8);
    std::int16_t s16;
    unsigned_work(s16);
    std::int32_t s32;
    unsigned_work(s32);
    std::int64_t s64;
    unsigned_work(s64);
#endif

    std::uint8_t u8;
    unsigned_work(u8);
    std::uint16_t u16;
    unsigned_work(u16);
    std::uint32_t u32;
    unsigned_work(u32);
    std::uint64_t u64;
    unsigned_work(u64);
}

void subtract_negative_to_signed()
{
    std::size_t large = 1000;
    std::size_t small = 400;
    std::size_t udiff = small - large;
    consume(udiff);
    std::ptrdiff_t sdiff = static_cast<std::ptrdiff_t>(udiff);
    consume(sdiff);
}

void index_calc_with_multiply()
{
    {
        std::cout << "  neg_step positive.\n";
        std::size_t base{4};
        std::size_t neg_step{2};
        std::size_t num_neg_step{4};
        std::size_t pos_step{3};
        std::size_t num_pos_step{3};
        consume(
            base - (neg_step * num_neg_step) + (pos_step * num_pos_step));
    }

    {
        std::cout << "  neg_step negative.\n";
        std::size_t base{4};
        std::size_t neg_step(-2);
        std::size_t num_neg_steps{4};
        std::size_t pos_step{3};
        std::size_t num_pos_steps{3};
        consume(
            base + (neg_step * num_neg_steps) + (pos_step * num_pos_steps));
    }
}

void index_calc_with_divide()
{
    {
        std::cout << "  unsigned div.\n";
        std::size_t base{4};
        std::size_t neg_step{2};
        std::size_t num_neg_step{4};
        std::size_t div{2};
        std::size_t num_pos_step{7};
        std::size_t corrected = base - (neg_step * num_neg_step);
        consume(corrected / div);
        consume((corrected / div) + num_pos_step);
    }

    {
        std::cout << "  signed div.\n";
        std::ptrdiff_t base{4};
        std::ptrdiff_t neg_step{2};
        std::ptrdiff_t num_neg_step{4};
        std::ptrdiff_t div{2};
        std::ptrdiff_t num_pos_step{7};
        std::ptrdiff_t corrected = base - (neg_step * num_neg_step);
        consume(corrected / div);
        consume((corrected / div) + num_pos_step);
    }
}

__attribute((noinline)) int mod(int b)
{
    return b % 16;
}

__attribute((noinline)) int mod(unsigned b)
{
    return b % 16;
}

__attribute((noinline))
std::ptrdiff_t
sum_range(std::ptrdiff_t n)
{
    std::ptrdiff_t sum{0};
    for (std::ptrdiff_t i = 1; i <= n; ++i)
    {
        sum += i;
    }
    return sum;
}

__attribute((noinline))
std::size_t
sum_range(std::size_t n)
{
    std::size_t sum{0};
    for (std::size_t i = 1; i <= n; ++i)
    {
        sum += i;
    }
    return sum;
}

__attribute((noinline)) unsigned sum_range(unsigned n)
{
    unsigned sum{0};
    for (unsigned i = 1; i <= n; ++i)
    {
        sum += i;
    }
    return sum;
}

__attribute((noinline)) unsigned useless(int value)
{
    return value * 7 / 7;
}

__attribute((noinline)) unsigned useless(unsigned int value)
{
    return value * 7 / 7;
}

__attribute((noinline)) void multiply_small()
{
    uint16_t small = std::numeric_limits<uint16_t>::max();
    consume(small * small);
    consume(static_cast<uint32_t>(small) * small);
}

__attribute((noinline))
int32_t
add_trunc_32(int32_t lhs, int32_t rhs)
{
    return lhs + rhs;
}

__attribute((noinline))
int16_t
add_trunc_16(int16_t lhs, int16_t rhs)
{
    return lhs + rhs;
}

__attribute((noinilne)) void test_add_trunc()
{
    {
        int64_t lhs{(1l << 33) + 5};
        int64_t rhs{(1l << 33) + 10};
        int64_t sum = lhs + rhs;
        int64_t added = add_trunc_32(lhs, rhs);
        std::cout << "\ntest_add_trunc\n";
        std::cout << "sum:   " << sum << '\n';
        std::cout << "added: " << added << '\n';
    }

    {
        int32_t lhs{(1l << 17) + 5};
        int32_t rhs{(1l << 17) + 10};
        int32_t sum = lhs + rhs;
        int32_t added = add_trunc_16(lhs, rhs);
        std::cout << "\ntest_add_trunc\n";
        std::cout << "sum:   " << sum << '\n';
        std::cout << "added: " << added << '\n';
    }
}

template <typename T, typename IndexType>
__attribute((noinline)) bool max_size_check()
{
    size_t constexpr max_allowed_size = static_cast<size_t>(std::numeric_limits<IndexType>::max());
    static_assert(std::vector<T>().max_size() <= max_allowed_size);
    return std::vector<T>().max_size() <= max_allowed_size;
}

template <typename Container>
bool continueReverseLoop(size_t index, Container const &container)
{
    return index < container.size();
}

template <typename Container>
bool continueReverseLoop(ptrdiff_t index, Container const &container)
{
    return index > 0;
}

template <typename Container>
__attribute((noinline)) void reverse_loop_template(Container &container)
{
    for (typename Container::size_type index = container.size() - 1;
         continueReverseLoop(index, container);
         --index)
    {
        consume(container[index]);
    }
}

template <typename SizeType>
struct Container
{
    using size_type = SizeType;

    Container(SizeType size) : m_size(size)
    {
    }

    SizeType size() const
    {
        return m_size;
    }

    int operator[](int index) const
    {
        return index;
    }

    SizeType m_size;
};

__attribute((noinline)) void reverse_loop_signed()
{
    Container<ptrdiff_t> container(100);
    reverse_loop_template(container);
}

__attribute((noinline)) void reverse_loop_unsigned()
{
    Container<size_t> container(100);
    reverse_loop_template(container);
}

__attribute((noinline)) void reverse_loop_vector()
{
    std::vector<int> container(100);
    reverse_loop_template(container);
}

volatile uint64_t bit{1};

__attribute((noinline))
uint64_t
get_bit()
{
    return bit;
}

__attribute((noinline))
uint64_t
mask_lsb(uint64_t value)
{
    uint32_t v1 = value;
    uint32_t v2 = get_bit();
    v2 &= v1;
    return v2;
}

int main()
{
    // std::cout << "Forwards:\n";
    // work_forwards(Image());
    // std::cout << "Backwards:\n";
    // work_backwards(Image());

    multiply_test();

    std::cout << "Distance\n";
    std::size_t top{10};
    std::size_t bottom{5};
    std::size_t distance = std::distance((char *)top, (char *)bottom);
    consume(distance);

    int64_t i{0};
    unsigned int u{0};
    static_assert(
        std::is_same_v<
            decltype(i + u),
            decltype(i)>);

    byte_offset_test();

    std::cout << "unsigned short - unsigned  short\n";
    unsigned short a{1};
    unsigned short b{2};
    consume(a - b);
    std::cout << "16-bit: " << (a - b < 0) << '\n';
    unsigned int a32{1};
    unsigned int b32{2};
    std::cout << "32-bit: " << (a32 - b32 < 0) << '\n';

    // Unsigned to larger signed.
    {
        std::cout << "Unsigned subtraction to larger signed.\n";
        std::uint32_t a{10};
        std::uint32_t b{11};
        std::int64_t c = a - b;
        consume(c);
    }

    std::cout << "sizeof(std::intmax_t): " << sizeof(std::intmax_t) << '\n';
    std::cout << "sizeof(__int128): " << sizeof(__int128) << '\n';

    __int128 largest_integer = std::numeric_limits<__int128>::max();
    std::cout << "max __int128: " << largest_integer << '\n';
    std::cout << "max unsiged long long: " << std::numeric_limits<std::uint64_t>::max() << '\n';

    __int128 large_number = __int128(100000) * __int128(100000);

    std::size_t v{10};
    // std::abs(v);

    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int *ptr_4 = a + 4;
        int *ptr_3 = ptr_4 + (-6);
        unsigned int offset = -1;
        int *ptr_2 = ptr_4 + offset;
        std::cout
            << " ptr_4=" << (ptr_4 - a)
            << " ptr_3=" << (ptr_3 - a)
            << " ptr_2=" << (ptr_2 - a)
            << '\n';
        std::cout << "*ptr_3=" << *ptr_3 << '\n';
    }

    {
        std::ptrdiff_t large_signed = std::numeric_limits<std::ptrdiff_t>::max();
        std::size_t large_unsigned = static_cast<std::size_t>(large_signed);
        large_unsigned += 1000;
        std::ptrdiff_t larger_signed = static_cast<std::ptrdiff_t>(large_unsigned);
        consume(larger_signed);
    }

    {
        std::cout << "Counting past max int " << std::numeric_limits<int>::max() << '\n';
        int large = std::numeric_limits<int>::max() - 5;
        for (int i = 0; i < 10; ++i)
        {
            consume(static_cast<size_t>(large + i));
            consume((large + i) + size_t(0));
            consume(large + i);
        }
    }

    {
        std::cout << "Add two large " << std::numeric_limits<int>::max() << '\n';
        int large = std::numeric_limits<int>::max() - 5;
        consume(large);
        consume(large + large);
        consume(add(large, large));
    }

    to_float_and_double();

    {
        uint8_t c1 = 100;
        uint8_t c2 = 3;
        uint8_t c3 = 4;
        uint8_t result = c1 * c2 / c3;
        consume(result);
    }

    consume((uint32_t)-3);
    consume(12 - 25u);

    std::cout << "subtract_negative_to_signed\n";
    subtract_negative_to_signed();
    index_calc_with_multiply();
    index_calc_with_divide();

    std::cout << "Multiply small\n";
    multiply_small();

    std::cout << "\nMax size:\n";
    std::cout << "ptrdiff_t max: " << std::numeric_limits<ptrdiff_t>::max() << '\n';
    std::cout << "size_t max:    " << std::numeric_limits<size_t>::max() << '\n';
    std::cout << "std::vector:   " << std::vector<char>().max_size() << '\n';

    // Does not build:
    //   error: array is too large (4611686018427387903 elements)
    // std::cout << "std::array:    " << (new std::array<char, std::numeric_limits<ptrdiff_t>::max() / 2>())->max_size() << '\n';

    test_add_trunc();

    max_size_check<char, ptrdiff_t>();
    max_size_check<char, int64_t>();

    // Does not build:
    // <source>:###:19: error: static assertion failed due to requirement 'std::vector<char, std::allocator<char>>().max_size() <= max_allowed_size'
    // ### |     static_assert(std::vector<T>().max_size() <= max_allowed_size);
    //     |                   ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // max_size_check<char, int32_t>();

    return 0;
}
