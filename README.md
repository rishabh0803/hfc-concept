# ⚡ hfc-concept

This is my personal learning repo for exploring the concepts involved in **HPC / low-latency programming**.  
It is a mix of lecture notes (PDFs), example programs, homework exercises, and small experiments - all kept in one place so I can revisit and build on them later.

Most of the code here is written in **C** and **C++**, and is primarily developed and run on **Ubuntu (Linux)**.

---

## 🎯 Goals

- Understand low-level systems concepts that matter for high-performance / low-latency code.
- Practice C and C++ with a focus on performance, memory, and OS interaction.
- Work through lecture material and turn it into runnable examples.
- Build small utilities, networking samples, and benchmarks to reinforce theory.

---

## 📂 Structure & Purpose

This is how the repo is organized (subject to evolve):

```
hfc-concept/
├── lecture/         # Per-lecture code, grouped by lecture number
│   ├── 37/          #   examples + named programs for lecture 37
│   ├── 38/          #   examples + named programs for lecture 38
│   └── 39/          #   examples + named programs for lecture 39
├── homework/        # Homework exercises, grouped by lecture number
│   └── 39/          #   homework for lecture 39 (homework_NNN.cpp)
├── examples/        # Legacy flat staging area (egNNN.cpp) — being moved into lecture/<n>/
├── docs/            # Concept-wise notes (mmap, serial I/O, sockets, CSV, ...)
├── network/         # Socket / networking samples (client, server, file upload, etc.)
├── scaler/          # Data handling experiments (CSV read/write, random data, etc.)
├── utils/           # Small helper utilities and reusable snippets
├── pdf/             # Lecture slides and reference PDFs
├── README.md
└── .gitignore
```

> **Migration in progress:** the repo is moving from flat `examples/` and
> `homework/` folders toward a per-lecture layout (`lecture/<n>/`,
> `homework/<n>/`). Lectures 37–39 already follow the new structure; older
> `egNNN.*` and `homework_NNN.cpp` files are being relocated into their lecture
> folders gradually, so both layouts coexist for now.

---

## 🧩 Folder Meaning (for me)

- **`lecture/<n>/`**  
  The *main* folder for structured practice, grouped by lecture number (e.g. `lecture/37/`, `lecture/38/`, `lecture/39/`). Each folder holds the example and named programs (`egNNN.cpp`, `read_mmap.cpp`, `serial_reader.cpp`, ...) tied to that lecture's topic. This is the target layout everything is moving toward.

- **`homework/<n>/`**  
  Homework problems assigned alongside the lectures, grouped by lecture number (e.g. `homework/39/`). Files like `homework_NNN.cpp` are kept with the lecture they belong to so context stays together.

- **`examples/`**  
  Legacy flat staging area. Files like `eg147.c`, `eg170.cpp`, `eg198.cpp` are short, focused exercises tied to individual lecture topics, numbered roughly chronologically. These are being moved into the matching `lecture/<n>/` folder over time.

- **`docs/`**  
  Concept-wise notes that cut across lectures — memory-mapped I/O, serial file reading, sockets, CSV/data handling, etc. Start at [`docs/README.md`](docs/README.md) for the index.

- **`network/`**  
  Networking-related code — basic TCP/UDP clients and servers, plus small projects like `fileUpload/`. Good place for socket programming and low-latency networking experiments.

- **`scaler/`**  
  Data-handling experiments — reading/writing CSVs, generating random datasets, file I/O patterns.

- **`utils/`**  
  Reusable helpers — timing, logging, small wrappers used across examples.

- **`pdf/`**  
  Lecture slides and reference material. Used as the source for the examples and homework.

---

## 🛠️ Build & Run (Ubuntu)

Most examples are single-file programs. Typical commands:

```bash
# C (per-lecture layout)
gcc -O2 -Wall -Wextra lecture/37/file_creator.c -o file_creator
./file_creator

# C++ (per-lecture layout)
g++ -std=c++20 -O2 -Wall -Wextra lecture/38/read_mmap.cpp -o read_mmap
./read_mmap

# Legacy flat examples still build the same way
g++ -std=c++20 -O2 -Wall -Wextra examples/eg198.cpp -o eg198
./eg198
```

Flags that come up often for performance / low-latency work:

```bash
-O2 / -O3        # optimization
-march=native    # tune for current CPU
-pthread         # threading
-lrt             # POSIX real-time (timers, shm)
```

---

## 📝 Notes

- This is a *learning* repo — code is meant to be readable and experimental, not production-ready.
- Things will be reorganized as topics progress.
- PDFs in `pdf/` are personal lecture references and not redistributed.
