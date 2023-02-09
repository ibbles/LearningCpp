Real-time means with an upper bound on latency in a cyclic process.
Not talking actual real-time with a real-time OS and dedicated hardware,
talking about doing the best we can on consumer machines.
Not the entire application need to be real-time, just the data processing.
Interactive elements and some I/O often have much weaker requirements.
We mark functions as being "real-time safe", meaning that have bounds on their runtime.
Such functions have restrictions, things they may not do.

Application is like a pipe with data flowing through.
Bandwidth: The amount of data that can be processed in a given amount of time, kind of an average.
Latency: Time from asking a question until answer.
This talk focus on latency, the data set is small.

Real-time means to put an upper limit on latency.
Common architecture:
- A callback given a set of data that is to be processed.
- The callback is called with a fixed frequency.
- Must complete one set before the next arrives.

For trading, fastest wins. Optimize for average.

For audio, no high value in finishing early but glitch if not finished in time. Optimize for the worst case.
Worst case should be:
- deterministic.
- known (knowable) in advance
	- Don't need an actual number for every function, just the aggregate.
- fail-tolerant, fail-less.

Properties of real-time safe code:
- Don't block, such as a mutex.
	- Don't necessarily know the task or priority of the thread holding the mutex.
- Don't allocate / deallocate memory.
- Don't do I/O.
- Don't do system calls.
- Don't call third-party code unless they are also real-time safe.
- Don't use algorithms with amortized constant complexity.
	- Because the worst-case may be way worse.
	- Watch out for O(n) container reallocations or storage reorganizations such as rehashing.
- Don't throw exceptions, because they require dynamic allocation.
	- At least for now, see _Zero-Overhead Deterministic Exceptions_ by Herb Sutter.
	- Exception implementation depend on ABI.
	- Most platforms have only a very small cost if no exception is actually thrown. MSVC 32-bit has overhead.

There is often a real-time thread, which is a thread with high scheduling priority.

The standard library specification doesn't say which functions are real-time safe or not.
It often doesn't even say if it does the things we don't want in real-time code.
Sometimes it can be inferred such as iterator invalidation, mentions of "if there is enough memory", may or may not be called from multiple threads.

Known to be real-time safe:
- `std::array`
- `std::pair`
- `std::tuple`
- `std::optional`
- `std::variant`
	- But not `boost::variant` because it keeps a backup of the current element in order to provide a strong exception guarantee.
	- `std::variant` becomes value-less instead.
- Lambdas.
	- Are allocated on the stack.
	- But beware for passing it to a function that takes it as a `std::function`.
	- Instead template the receiving function on the function type, then its not type erasure anymore.

Known to be not real-time safe:
- `std::stable_sort`
- `std::stable_partition`
- `std::inplace_merge`
- `std::execution::parallel_*`
- All containers except for `std::array`.
- `std::any` and `std::function`
	- Type-erasure, require heap allocation since the size isn't known at compile time.
	- Unless you are saved by small object optimization, dangerous to rely on.
- Coroutines.
	- Creation may perform dynamic memory allocation.
	- The compiler may optimize the memory allocation away, but don't depend on it.
	- Can create the coroutine ahead of time, calling it is real-time safe.


# Containers

On some compilers, not MSVC, we can get dynamically sized buffers without dynamic allocation through variable length arrays.
Not actually allowed by the standard, is an extension.

We can provide custom allocators for many containers, that use a real-time safe means of acquiring memory.
- Can always service an allocation request in constant time.
- Single-threaded only, because we don't want lock.
	- What about lock-free allocators? Does any exist?
- May only use memory we've allocated upfront.

Available in the standard library:
- `std::pmr::monotonic_buffer_resource`
	- Allocate memory elsewhere, give it to the buffer resource.
	- Does allocation by advancing a head pointer.
	- Cannot deallocate, must reset the allocator and start from scratch.
	- Have a fallback allocator to get more memory, we can set a null-allocator to make sure no allocation ever happens.
- `std::pmr::unsynchronized_pool_resource`
	- Does support deallocation.
	- Holds pools of memory, one pool per allocation size.
	- A pool has chunks of memory, a contiguous memory region.
	- A chunk has blocks of memory, an allocatable bit of memory.
	- Allocates chunks for pools as necessary by the user.
	- Gets chunks from an upstream allocator.
	- Can use `std::pmr::monotonic_buffer_resource` as the upstream allocator.
	- We get a real-time safe allocator that will never do a dynamic allocation from a system allocator.


`static_vector` is a vector with a compile-time fixed capacity help by-value in the object.
Not part of standard library but there is a proposal, P0842.
See talk titled _Implementing `static_vector`_ by David Stone.

Can use a similar allocation technique for associative containers,
but consider `flat_map` instead.

`


# /Containers

