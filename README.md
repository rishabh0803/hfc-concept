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
├── examples/        # Lecture-driven examples (egNNN.cpp) and homework_NNN.cpp
├── network/         # Socket / networking samples (client, server, file upload, etc.)
├── scaler/          # Data handling experiments (CSV read/write, random data, etc.)
├── utils/           # Small helper utilities and reusable snippets
├── pdf/             # Lecture slides and reference PDFs
├── README.md
└── .gitignore
```

---

## 🧩 Folder Meaning (for me)

- **`examples/`**  
  The *main* folder for structured practice. Files like `eg147.c`, `eg170.cpp`, `eg198.cpp` are short, focused exercises tied to individual lecture topics. Numbering is roughly chronological with the lectures.

- **`examples/homework_*.cpp`**  
  Homework problems assigned alongside the lectures. Kept next to the related examples so context stays together.

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
# C
gcc -O2 -Wall -Wextra examples/eg147.c -o eg147
./eg147

# C++
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
