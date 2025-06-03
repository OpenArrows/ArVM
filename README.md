# ArVM

**ArVM** (_Arrows VM_) is an optimizing JIT-compiling CA backend for building high-performance simulations. It includes the following components:
* **Builder**. Easy to use interface for building logical and arithmetic expressions.
* **Optimizer**. Optimizes ArVM expressions, inlines functions. It is specifically designed for boolean algebra and cell machines. Use this feature to get the maximum performance.
* **Compiler**. Compiles ArVM to native code and runs it.
* **Evaluator**. If the target system doesn't support JIT-compilation, the built-in interpreter for ArVM IR allows you to evaluate functions without compiling them.

Written in pure C btw. :cool:

ArVM is a part of the [OpenArrows project](https://github.com/OpenArrows).