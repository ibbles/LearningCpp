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

Processing data that is scattered across the address space is usually not computationally intensive.
Because the cache system is unable to predict and fetch data from memory ahead of time.
This happens if you do look-ups in hash-maps or sets, if you traverse linked list, tree or graph data structures, or if you chase pointers in general.

Accessing data in random order is usually not computationally intensive because of the above reasons.
Even if the accesses are from a single array.
If the array is small, i.e. fit in cache, then it may be OK.

Code typically doesn't become computationally intensive by itself, most code is not.
Some domains are more commonly computationally intensive than others, such as media processing and scientific computing.
Anything that can be expressed as linear algebra.


## Vectorization

Called SIMD for Single Instruction, Multiple Data.
Process more than a single piece of data per instruction.
Load, for example, two pairs of four doubles into a pair of registers and perform four pair-wise operations between them.
Common vector sizes are 2, 4, and 8. Typically powers of two.
There are special instructions that operate on vectors.
These come in various instruction sets, such as a few versions of SSE and AVX for x64 and Neon for Arm.
Can be done with intrinsics, which are function-like identifiers that map directly to machine instructions.
In some cases the compiler can generated vectorized code from scalar loops.
This is called auto-vectorization.
There are requirements:
- Compiler optimizations must be turned on to a sufficient level.
	- Depends on which compiler you use.
- Must compile for an architecture that has vector instructions.
- Many code-specific requirements.
	- Operates on primitive data types or very small classes.
	- Pass through the data in a linear fashion, so that adjacent elements are operated on in sequence / sequentially.
	- Iterations must be independent, one iteration cannot use the result of a prior iteration. Because when done in parallel the prior iteration's result may not have been computed yet. It should be possible to run the iterations in any order and still get the same result, or run only a subset of the iterations and get a subset of the result.
	- The number of iterations must be known when the loop starts. If the end condition can change as the iterations are progressing then we cannot know if we really will be doing vector-width more iterations. This means that the iterator variable should be incremented in a predictable manner, and the stop comparison should be fixed.
	- Avoid conditional in the loop body. Vector instructions operate on all elements in the register, but if some iterations take one path and other operations take another path then things get complicated. There are mask stuff that can be done, but it becomes less efficient.
	- No pointer aliasing. Not clear to me why pointer aliasing prevents auto-vectorization. Is it a form of iteration dependency?

Transformations that make a loop vectorizable:
- Loop fission: Put the vectorizable parts of the iterations in one loop and the non-vectorizable parts in another.
- Loop unswitching: If the loop contains a condition that is fixed for each loop batch: Write two versions of the loop, each with the two paths for the condition.
- Loop sectioning: When the loop has an early out, the number of iterations isn't known ahead of time. Put a small loop that is easy to vectorize inside the big loop that is difficult to vectorize. Process the data in sections, a.k.a. batches.
- Store data in SoA instead of AoS.
- Use `__restrict__` to promise to the compiler that two pointers don't alias.

When looping over arrays in a manually vectorized loop it is common to step the iteration index by the vector width.
One must know the vector width when writing the code, or use a library that manages this behind the scenes.

The speed-up is typically 2-6 times.
Rarely the width of the vector registers because of overhead required in managing multiple values.
A scalar loop that the compile can auto-vectorize is a good indication that it will be difficult to write a faster vectorized implementation by hand.
The compiler can log information about vectorization attempts:
- clang: `-Rpass=loop-vectorize -Rpass-analysis=loop-vectorize`
- GCC: `-fopt-info-vec-allt`


# Converting Memory-Bound Code to Compute-Bound Code

We  usually get better performance if we can transform a piece of code so that it goes from being memory intensive to computationally intensive.
Compute is generally faster than data movement on a modern CPU.
CPUs are easily data-starved.
Transforming memory-bound code to compute-bound code often leads to a shorter execution time.
Strive to make efficient use of caches, small and fast memories inside the CPU.
Try to not bring more data into the cache than you will use,
try to use all the data that is being brought into cache.
The goal is to serve as many memory accesses as possible from the cache.
Cache misses happens when:
- Dereferencing a pointer for the first time.
- Chasing pointer-based data structures.
- Accessing array elements in random order.
- Look-ups in associative containers.

Smaller data sets have a higher probability to fit in cache and thus have a higher ratio of cache hits.
Keep data that is accessed together stored together.
- Sort linked lists so that each `next` pointer points to the next byte in memory.
- For vectors of pointers, allocate the pointed-to memory from a bigger chunk and keep the pointers sorted by pointed-to address.
- If they point to polymorphic types, sort the objects by type so that a loop calling a virtual function will switch function as few times as possible.
- Order class member so that members that are used at the same time are close together in memory.
	- Especially for large classes, larger than a cache line, often 64 bytes.
Don't iterate over large data sets more times than necessary, do as much work as possible each pass.

For non-trivial data structures consider implementations that produce good memory layouts.
For example, binary trees following van Emde Boas layout.
Use n-ary trees instead of binary trees, they have fewer levels and thus fewer pointer chases.
For hash maps, prefer open addressing.
Find implementations optimized for speed, such as EA STL.

Prefer to use by-value container elements instead of by-pointer.
- All elements are stored together, better cache usage and only one allocation call for all elements. If you pre-size / reserve
- All elements are stored in order, very good cache utilization and prefetching.
- Don't need an indirection to access the data.
- No virtual dispatch on function calls since there is only one function that can be called.
	- Gives the compiler the option to inline the called function.
- Drawback is we need some kind of workaround to get polymorphism, if we need it.
	- Such as `std::variant`.

Don't trust your assumptions, always measure before and after any performance/optimization motivated change.
Clever data structures often have overhead that isn't amortized if use with small data sets or used for a very short time.

Use the available memory bandwidth for data the CPU actually needs, not bytes that just happened to be nearby.
- Avoid random access pattern, prefer linear data structures when possible.
- Avoid pointer chasing, especially if the pointer targets doesn't walk predictably through memory.
- Keep things that are used together stored together.
- Keep the data set small.
- Avoid reading the same data multiple times.

## Structure Of Arrays

The traditional way to store a collection of objects is called array-of-structures.
In this format we have a `struct` that holds the data for a single entity and we store a collection of entities by having an array of those `structs`.
```cpp
struct Entity
{
	double attribute1;
	int attribute2;
	double attribute3;
};

Entity entites[N];
```

In array-of-structures we invert the format.
Instead of having any array with a bunch of structures in we have a structure that contains a bunch of arrays, one per attribute.
```cpp
struct Entities
{
	double attribute1[N];
	int attribute2[N];
	double attribute3[N];
} entities;
```

With the new formulation a loop over a subset of the attributes will only load those attributes into cache, all the other attributes will be untouched and won't pollute the cache.


## Iterate Composite Data Types In Memory Order

The classical example is a dense matrix.
When moving from element to element we want to move the shortest possible distance in memory, and we don't want to jump back to areas of the memory we've already visited.
For a row-major matrix, which is the default for a two-dimensional array, this means that we should process one row completely before moving on to the next.
The worst we can do is to iterate down the columns, because then we load a subsection of a row into cache, use one of the elements, and then move to the next row, far away in memory.
If the matrix is large enough then by the time we get back to the first row again then the data we read into cache have already been evicted.
 
 
Pass large objects to functions by-reference instead of by-value.
To avoid needless copying.