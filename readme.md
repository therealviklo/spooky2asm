spooky2asm
==========
spooky2asm is a command-line program that compiles [Spooky](https://github.com/jsannemo/spooky-vm/) to x86-64 NASM assembly. I don't think that the assembly that the compiler generates is very efficient, but it works (hopefully). Currently, it only supports Windows. (The compiler can run on other operating systems but the assembly that it generates is made for Windows.)

Usage
-----
`spooky2asm source` or `spooky2asm -o output source`

Please note that this will generate an assembly file that needs to be assembled and linked in order to be executed. Some C standard library functions (putchar, rand, srand, time) are used, as well as one Windows-specific function (ExitProcess), so make sure that they are linked against. Linking using a C compiler seems to take care of this. Here is an example of how to do this (using [NASM](https://www.nasm.us/) to assemble and [Clang](https://clang.llvm.org/) to link):  
```
spooky2asm program.spooky
nasm -f win64 program.asm
clang -o program.exe program.obj
```  
I _think_ that GCC should work as well, but I have not been able to try that out properly since MinGW doesn't seem to support 64 bit. (MinGW-w64 might work, though.)

Implementation details
----------------------
I have tried to make this compiler as similar to [spooky-vm](https://github.com/jsannemo/spooky-vm/) as possible. However, there are some small differences that I have noticed:
* spooky2asm uses 64 bit signed integers instead of the 32 bit signed integers that spooky-vm uses.
* spooky-vm doesn't seem to support empty statements (e.g. `if (true);`), which spooky2asm does.
* spooky2asm lets you indent using tabs.
* spooky2asm lets you have 0 as an implicit operand for addition and subtraction (which means that `i: Int = -1;` is valid).

spooky2asm uses its own calling convention internally, instead of the Microsoft x86-64 calling convention. However, the Microsoft x86-64 calling convention is used inside of the standard library functions when they call external functions. Here is a description of the calling convention that is used by spooky2asm, in case someone wants to know:
* All parameters are passed on the stack, placed in right-to-left order (last argument is first on the stack).
* The caller cleans up the parameter stack space.
* Values are returned in rax.
* No stack aligning is necessary.

Building from source
--------------------
To build the program from source, you can compile `all.cpp`, which #include's all of the source files.