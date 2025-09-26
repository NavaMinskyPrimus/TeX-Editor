# TeX-like Macro Processor

## Overview
This project is a simplified TeX-like macro processor written in C. It reads one or more input files, parses user-defined and built-in macros, and expands them into output text. The processor supports recursion, conditional evaluation, file inclusion, and escape sequences, while detecting and reporting malformed inputs.

This project was completed in **Spring 2025** for Yale’s CPSC 323 (Systems Programming). It demonstrates low-level programming skills, including manual memory management, parsing, and error detection.

---

## Key Features
- **Macro Definition and Expansion**  
  - `\def{NAME}{VALUE}` defines new macros with argument substitution (`#`).  
  - Recursive and nested macro expansion is supported.  

- **Built-in Macros**  
  - `\def`, `\undef`, `\if`, `\ifdef`, `\include`, `\expandafter`.  
  - Handles conditionals and delayed expansion.  

- **Comments & Escapes**  
  - `%` introduces comments, stripped at parse-time.  
  - Escape characters (`\{`, `\}`, `\#`, `\\`, `\%`) allow literal output.  

- **Error Detection**  
  - Malformed macro names or arguments.  
  - Undefined macros.  
  - Redefinition without `\undef`.  
  - Library/system call errors (e.g. `fopen`, `malloc`).  

- **Performance & Safety**  
  - Linear-time processing relative to input size.  
  - Debugged with `gdb` and Valgrind to avoid memory errors.  
  - Careful pointer and buffer management for safety.  

---

## Why This Project Matters
I’m drawn to systems programming because it makes you confront the realities of how software interacts with memory, processes, and data structures. This project shows how careful engineering can prevent subtle bugs, while still producing code that is performant and maintainable. That philosophy aligns closely with **Everlaw’s emphasis on code quality and correctness** over rushing features.

---

## How to Build & Run
### Requirements
- GCC with C11 support  
- `make`  
- Tested on **macOS (Clang)** and **Linux (GCC on Yale Zoo servers)**

### Build
make
./proj1 [file1 file2 ...]

