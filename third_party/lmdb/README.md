# [strdup.patch](./strdup.patch)

Eliminates usage of strdup from mdb.c.

`strdup` internally uses glibc malloc to allocate the returned `char *`.
This means that if application is using some other allocator,
say jemalloc, `free`-ing result of `strdup` is an undefined behavior.

## References
- https://stackoverflow.com/questions/32944390/what-is-the-rationale-for-not-including-strdup-in-the-c-standard
- jemalloc/jemalloc#365
- rust-lang/rust#9925
