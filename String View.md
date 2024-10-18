A view into a string [(1)](https://www.youtube.com/watch?v=QpFjOlzg1r4).
Does not own the memory, only points to it.
Beware of dangling views, just like you would beware of dangling pointers.
Commonly used to pass read-only text to a function.
Makes it so that the caller doesn't need to match the text storage type of the callee.
Reduces the amount of temporary string instances being created.

# References


- 1: [_C++ Weekly - Ep 190 - The Important Parts of C++17 in 10 Minutes_ by Jason Turner @ youtube.com 2019](https://www.youtube.com/watch?v=QpFjOlzg1r4)
