# Serial File Reading

Reading files sequentially through standard I/O, as a baseline before comparing
against memory-mapped or parallel approaches.

## Why it matters

- Establishes a simple, correct baseline for throughput comparisons.
- Highlights the cost of per-call syscalls and buffering choices.

## Related code

- `lecture/37/serial_reader.cpp` — straightforward sequential read.
- `lecture/37/serial_preader.cpp` — variant using positioned reads (`pread`).
- `lecture/37/file_creator.c` — generates input files for the readers.

## Notes

_(Add observations, gotchas, and benchmark results here.)_
