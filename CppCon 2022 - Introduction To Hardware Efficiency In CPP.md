Speaker: Ivica Bogosavljevic
URL: [https://www.youtube.com/watch?v=Fs_T070H9C8](https://www.youtube.com/watch?v=Fs_T070H9C8)
Related material: [https://johnysswlab.com](https://johnysswlab.com)
 
 Improvements that make programs faster:
- Better algorithms
- Better exploiting the underlying hardware, be hardware efficient.
- Better usage of the standard library.
- Better usage of the programming language, follow best-practices.
- Better usage of the  operating system.
- Use good tools, such as compilers and profilers.
- Better software architecture
- Avoid unnecessary work.


## Hardware Efficiency

Two main performance barriers: core and memory.
We are core bound if we are limited by the CPU resources.
We are memory bound if the CPU is idle waiting for data during a large fraction of the time.
Use a profiler to determine which of the two are the case in any particular program or part of a program.
For example, a large number of cache misses is indicative of a memory bound program.
For example, a high utilization of the arithmetic functional units means the program is compute bound.
Program transformations that improve one of the two may be detrimental to the other.


# Compilers

Use a compiler that  produces efficient machine code.
Not sure which do this, but I assume `gcc`, `clang`, and `icc` qualify.
Build with optimizations enabled.


# Profilers

Some profilers read hardware performance counters.
For example Intel's VTUNE and pmu-tools, AMD's uProf, perf, LIKWID.
Can be difficult to use, must now what the numbers mean.

If the number of cache misses is high then the program is likely memory bound.
Not sure what number of cache misses is considered high.

If the numbers of SIMD instructions is high then the program is likely compute bound.
Not sure what number of SIMD instructions is considered high.


# Optimizing Computationally Intensive Codes

Code of this category should
- process primitive datatypes
- that are stored consecutively in memory
- and accessed in order.

Processing large classes, with many member, with a large `sizeof`, is usually not computationally intensive.
Because of large stride and small cache utilization when iterating over the objects.

Processing data is is scattered across the address space is usually not computationally intensive.
Because the cache system is unable to predict and fetch data from memory ahead of time.

Accessing data in random  order is usually not computationally intensive because of the above reasons.

# Programming Language Best-Practices

Pass large objects to functions by-reference instead of by-value.
To avoid needless copying.