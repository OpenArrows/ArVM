# ArVM

**ArVM** (_Arrows Virtual Machine_) is an optimizing JIT-compiling backend for building high-performance (specific) cellular automata simulations. It includes the following components:
* **Builder**. Easy to use interface for constructing logical and arithmetic expressions.
* **Optimizer**. It is specifically designed for boolean algebra and CA.
* **Codegen**. JIT-compiles ArVM to machine code (_x86_, _x86-64_).
* **Evaluator**. As an alternative to JIT-compilation, ArVM provides a way to evaluate functions without compiling them. It is useful if the target platform is not supported by the backend.

Written in pure C btw. :cool:

ArVM is a part of the [OpenArrows project](https://github.com/OpenArrows).

## Installation

ArVM has no pre-built binaries, so the only way to use it is by cloning the repository into your project.

If you use CMake you can link ArVM to your project like this:
```cmake
set(ARVM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/arvm")
add_subdirectory(${ARVM_PATH})

target_link_libraries(yourtarget PUBLIC arvm)
```

## Usage

Running a simulation using ArVM as a backend includes three main steps:
1. ArVM IR generation
2. Optimization
3. Compilation/evaluation
   
### IR generation

First of all, the frontend need to generate a function for each arrow/cell. ArVM only provides code-related API, meaning that grid representation is frontend-defined.

API for constructing IR is decalred in `<arvm/builder.h>`. Currently, there are 6 (+ a few internal) specific expression kinds: **binary** (_modulo_), **n-ary** (logical _and_, _or_, _xor_ and arithmetic _add_), **in-interval** (checks if a value is in inclusive range [min, max]), **call** and **const**.

### Optimization

Secondly, the frontend should pass the generated IR to the optimizer.

### Compilation/evaluation

Finally, the frontend can pre-compile the generated functions and run the simulation.