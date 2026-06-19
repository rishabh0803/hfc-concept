# Memory-Mapped I/O (`mmap`)

Mapping a file directly into the process address space so it can be accessed as
memory, avoiding explicit `read`/`write` syscalls and extra buffer copies.

## Why it matters

- Avoids copying between kernel and user buffers for each access.
- Lets the OS page data in on demand and share pages across processes.
- Useful for large files, zero-copy reads, and fast file-to-file copies.

## Related code

- `lecture/38/read_mmap.cpp`, `lecture/38/read_mmap2.cpp` — mapping a file for reading.
- `lecture/39/copy_mmap.cpp` — copying via memory maps.

## Notes

_(Add observations, gotchas, and benchmark results here.)_
